#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestObjectiveData.generated.h"

// Container with data on how to complete a quest's objective
USTRUCT(BlueprintType)
struct FQuestObjectiveData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag TriggerTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer RequiredModifierTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 RequiredCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;
};
