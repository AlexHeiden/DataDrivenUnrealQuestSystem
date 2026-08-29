#pragma once

#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "QuestSaveGame.generated.h"

struct FQuestSaveData
{
	int32 CurrentObjectiveIndex = 0;
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
	TArray<FGameplayTag> CompletedQuestIDs;
};
