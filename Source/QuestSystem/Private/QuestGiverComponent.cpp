#include "Actor Quest Components/QuestGiverComponent.h"

#include "Quest Managers/QuestManagerSubsystem.h"

bool UQuestGiverComponent::TryAcceptQuest()
{
	if (!QuestToGive)
	{
		return false;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	UQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UQuestManagerSubsystem>();
	if (!QuestManager)
	{
		return false;
	}

	return QuestManager->AcceptQuest(QuestToGive);
}
