#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayMessageRuntime/Public/GameFramework/GameplayMessageSubsystem.h"
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

UCLASS(Blueprintable)
class UQuestManagerSubsystem: public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AcceptQuest(UQuestDefinition* QuestDefinition);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestActive(FGameplayTag QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestCompleted(FGameplayTag QuestID) const;

private:
	UPROPERTY()
	TMap<FGameplayTag, FActiveQuestState> ActiveQuests;
	
	UPROPERTY()
	FGameplayTagContainer CompletedQuestIDs;

	TMap<FGameplayTag, FGameplayMessageListenerHandle> ListenerHandles;
	TMap<FGameplayTag, int32> ListenerRefCounts;

	bool CheckPrerequisites(UQuestDefinition* QuestDefinition) const;
	void HandleGameplayEvent(FGameplayTag EventTag, const FQuestEventPayload& EventPayload);
	void RegisterQuestListeners(FGameplayTag EventID);
	void UnregisterQuestListeners(FGameplayTag EventID);
};
