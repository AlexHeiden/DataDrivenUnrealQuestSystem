#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestData.generated.h"

UENUM(BlueprintType)
enum class EQuestState: uint8
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

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InstigatorTag;	

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer ModifierTags;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Amount;
};
