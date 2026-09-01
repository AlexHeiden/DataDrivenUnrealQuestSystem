#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data Classes/QuestDefinition.h"
#include "Data Classes/QuestObjectiveData.h"
#include "Data Classes/QuestData.h"
#include "QuestManagerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FActiveQuestState
{
	GENERATED_BODY()

	FActiveQuestState() = default;

	FActiveQuestState(UQuestDefinition* InQuestDefinition):
		QuestDefinition(InQuestDefinition)
	{
		QuestState = EQuestState::Active;
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
	// Delegates for world reactivity
	// These are created just for the demonstration purpose
	// See "Possible Next Steps" in "README" to see how it could be improved 
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
	
	UPROPERTY()
	FGameplayTagContainer CompletedQuestIDs;

	TMap<FGameplayTag, FGameplayMessageListenerHandle> ListenerHandles;
	TMap<FGameplayTag, int32> ListenerRefCounts;
	
	bool CheckPrerequisites(const UQuestDefinition* QuestDefinition) const;

	static bool IsObjectiveCompleted(const FQuestObjectiveProgress* ObjectiveProgress, const FQuestObjectiveData* ObjectiveData);
	static bool HasCompletedAllObjectives(const FActiveQuestState* ActiveQuestState);
	
	UFUNCTION()
	void DebugLogQuestRewardGranted(FGameplayTag QuestID, int32 RewardXP);
	
	void CompleteCurrentObjective(FQuestObjectiveProgress* CurrentObjectiveProgress, FActiveQuestState* ActiveQuestState);
	void HandleGameplayEvent(FGameplayTag EventTag, const FQuestEventPayload& EventPayload);
	void RegisterQuestListeners(FGameplayTag EventID);
	void UnregisterQuestListeners(FGameplayTag EventID);

	UQuestDefinition* ResolveQuestDefinition(FGameplayTag QuestID);
};
