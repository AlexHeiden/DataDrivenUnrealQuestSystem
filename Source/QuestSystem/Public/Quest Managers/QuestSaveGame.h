#pragma once

#include "GameplayTagContainer.h"
#include "Data Classes/QuestData.h"
#include "GameFramework/SaveGame.h"
#include "QuestSaveGame.generated.h"

USTRUCT()
struct FQuestSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 CurrentObjectiveIndex = 0;

	UPROPERTY()
	TArray<FQuestObjectiveProgress> ObjectiveProgresses;
};

UCLASS()
class UQuestSaveGame: public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TMap<FGameplayTag, FQuestSaveData> QuestProgresses;

	UPROPERTY()
	FGameplayTagContainer CompletedQuestIDs;
};
