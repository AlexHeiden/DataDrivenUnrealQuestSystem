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
