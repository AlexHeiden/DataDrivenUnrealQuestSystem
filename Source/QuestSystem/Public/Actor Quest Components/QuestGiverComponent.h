#pragma once

#include "Data Classes/QuestDefinition.h"
#include "QuestGiverComponent.generated.h"

UCLASS(ClassGroup = Quest, meta=(BlueprintSpawnableComponent))
class UQuestGiverComponent: public UActorComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool TryAcceptQuest(UQuestDefinition* QuestDefinition);
};
