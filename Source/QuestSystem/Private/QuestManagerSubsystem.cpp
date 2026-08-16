#include "Public/Quest Managers/QuestManagerSubsystem.h"

void UQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UQuestManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UQuestManagerSubsystem::AcceptQuest(UQuestDefinition* QuestDefinition)
{
	if (QuestDefinition == nullptr
		|| !QuestDefinition->QuestID.IsValid()
		|| ActiveQuests.Contains(QuestDefinition->QuestID)
		|| CompletedQuestIDs.Contains(QuestDefinition->QuestID))
	{
		return false;
	}
	
	ActiveQuests.Add(QuestDefinition->QuestID, FActiveQuestState(QuestDefinition));

	return true;
}

bool UQuestManagerSubsystem::IsQuestActive(FGameplayTag QuestID) const
{
	if (!QuestID.IsValid())
	{
		return false;
	}
	
	return ActiveQuests.Contains(QuestID);
}

bool UQuestManagerSubsystem::IsQuestCompleted(FGameplayTag QuestID) const
{
	if (!QuestID.IsValid())
	{
		return false;
	}

	return CompletedQuestIDs.Contains(QuestID);
}
