#pragma once

#include "Public/DataClasses/QuestDefinition.h"
#include "QuestGiverComponent.generated.h"

// Blueprint component to use in quest-giving actors
// Filled and used by game designers in the Engine
UCLASS(ClassGroup = Quest, meta=(BlueprintSpawnableComponent))
class UQuestGiverComponent: public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests")
	TObjectPtr<UQuestDefinition> QuestToGive;
	
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool TryAcceptQuest();
};
