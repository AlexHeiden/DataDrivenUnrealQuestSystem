#pragma once

#include "GameplayTagContainer.h"
#include "QuestBroadcasterComponent.generated.h"

// Blueprint component to use in all actors which broadcast quest events
// Filled and used by game designers in the Engine
UCLASS(ClassGroup = Quest, meta=(BlueprintSpawnableComponent))
class UQuestBroadcasterComponent: public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag InstigatorTag;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer ModifierTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 Amount = 1;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void BroadcastQuestEvent();
};
