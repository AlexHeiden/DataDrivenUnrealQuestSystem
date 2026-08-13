#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.generated.h"

UENUM(BlueprintType)
enum EQuestTypes
{
	Inactive,
	Active,
	Completed,
	Failed
};

// Runtime progress for a single objective within an active quest.
USTRUCT(BlueprintType)
struct FQuestObjectiveProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bCompleted = false;
};

// Container sent by gameplay events to quest system through GMS
USTRUCT(BlueprintType)
struct FQuestEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InstigatorTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer ModifierTags;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Amount;
};
