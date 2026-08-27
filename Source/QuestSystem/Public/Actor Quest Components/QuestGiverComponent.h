#pragma once

#include "Data Classes/QuestDefinition.h"
#include "QuestGiverComponent.generated.h"

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
