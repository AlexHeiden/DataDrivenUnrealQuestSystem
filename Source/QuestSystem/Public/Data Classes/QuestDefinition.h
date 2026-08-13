#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "QuestObjectiveData.h"
#include "QuestDefinition.generated.h"

// Quest data asset
UCLASS(BlueprintType)
class UQuestDefinition: public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	static const FPrimaryAssetType QuestAssetType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText QuestName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText QuestDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag QuestID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer RequiredCompletedQuests;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FQuestObjectiveData> Objectives;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
