#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data Classes/QuestDefinition.h"
#include "Data Classes/QuestObjectiveData.h"
#include "Data Classes/QuestData.h"
#include "QuestManagerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FActiveQuestState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UQuestDefinition> QuestDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly)
	EQuestState QuestState = EQuestState::Inactive;

	UPROPERTY(BlueprintReadOnly)
	TArray<FQuestObjectiveProgress> ObjectiveProgresses;
};

UCLASS(Blueprintable)
class UQuestManagerSubsystem: public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	int Test();

private:

	UPROPERTY()
	
	
	UPROPERTY()
	TArray<FGameplayTag> CompletedQuestIDs;
};

