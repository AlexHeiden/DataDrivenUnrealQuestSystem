#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"
#include "Public/DataClasses/QuestDefinition.h"
#include "Public/DataClasses/QuestObjectiveData.h"
#include "Public/DataClasses/QuestData.h"
#include "QuestManagerSubsystem.generated.h"

// Main active quest container for the runtime
USTRUCT(BlueprintType)
struct FActiveQuestState
{
	GENERATED_BODY()

	FActiveQuestState() = default;

	FActiveQuestState(UQuestDefinition* InQuestDefinition):
		QuestDefinition(InQuestDefinition)
	{
		QuestState = EQuestState::Active;
		// ObjectiveProgresses are index-aligned with QuestDefinition->Objectives
		ObjectiveProgresses.Init(FQuestObjectiveProgress(), QuestDefinition->Objectives.Num());
	}
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UQuestDefinition> QuestDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly)
	EQuestState QuestState = EQuestState::Inactive;

	UPROPERTY(BlueprintReadOnly)
	TArray<FQuestObjectiveProgress> ObjectiveProgresses;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentObjectiveIndex = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, FGameplayTag, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgressChanged, FGameplayTag, QuestID, int32, ObjectiveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestRewardGranted, FGameplayTag, QuestID, int32, RewardXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestsLoaded);

// GameInstanceSubsystem controlling the quests' acceptance, progression and completion
UCLASS(Blueprintable)
class UQuestManagerSubsystem: public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Delegates for world reactivity (UI, VFX, etc.)
	// For a demo, these BlueprintAssignable delegates are the simplest
	// A larger project would more likely send the messages through GameplayMessageSubsystem to another subsystem
	UPROPERTY(BlueprintAssignable, Category="Quest")
	FOnQuestStateChanged OnQuestStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Quest")
	FOnObjectiveProgressChanged OnObjectiveProgressChanged;
	UPROPERTY(BlueprintAssignable, Category="Quest")
	FOnQuestRewardGranted OnQuestRewardGranted;
	UPROPERTY(BlueprintAssignable, Category="Quest")
	FOnQuestsLoaded OnQuestsLoaded;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AcceptQuest(UQuestDefinition* QuestDefinition);
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestActive(FGameplayTag QuestID) const;
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestCompleted(FGameplayTag QuestID) const;
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FGameplayTag> GetActiveQuestIDs() const;
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetQuestName(FGameplayTag QuestID) const;
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FText GetCurrentObjectiveText(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SaveQuestState() const;
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void LoadQuestState();

private:
	UPROPERTY()
	TMap<FGameplayTag, FActiveQuestState> ActiveQuests;
	
	// I made it an FGameplayTagContainer instead of TSet to use all convenient FGameplayTagContainer functions
	// TSet would have cheaper operations, but, unless we have A LOT of completed quests, we wouldn't feel the difference
	UPROPERTY()
	FGameplayTagContainer CompletedQuestIDs;

	// They listen for a specific quest event broadcasted to launch HandleGameplayEvent()
	TMap<FGameplayTag, FGameplayMessageListenerHandle> ListenerHandles;
	// If we have 2 quests waiting for the same event, we would increase a RefCount and still have just 1 listener
	TMap<FGameplayTag, int32> ListenerRefCounts;

	// Temporary listener for OnQuestRewardGranted
	// Sends log messages to verify rewards. Safe to remove after a real reward manager replaces it
	UFUNCTION()
	void DebugLogQuestRewardGranted(FGameplayTag QuestID, int32 RewardXP);

	// Checks all quests in QuestDefinition->RequiredCompletedQuests are present in CompletedQuestIDs 
	bool CheckPrerequisites(const UQuestDefinition* QuestDefinition) const;

	static bool IsObjectiveCompleted(const FQuestObjectiveProgress* ObjectiveProgress, const FQuestObjectiveData* ObjectiveData);
	static bool HasCompletedAllObjectives(const FActiveQuestState* ActiveQuestState);
	
	void CompleteCurrentObjective(FQuestObjectiveProgress* CurrentObjectiveProgress, FActiveQuestState* ActiveQuestState);
	// Central event handler bound to all listened tags
	// Advances objectives' progress, checks for objective/quest completion,
	// triggers world reactivity delegates
	void HandleGameplayEvent(FGameplayTag EventTag, const FQuestEventPayload& EventPayload);
	// Registers a GameplayMessageSubsystem listener for EventID if one doesn't already exist;
	// increments RefCount either way
	void RegisterQuestListeners(FGameplayTag EventID);
	// Decrements RefCount for EventID; unregisters the listener only when RefCount == 0
	// (i.e. no active quests need this event anymore)
	void UnregisterQuestListeners(FGameplayTag EventID);
	
	UQuestDefinition* ResolveQuestDefinition(FGameplayTag QuestID);
};
