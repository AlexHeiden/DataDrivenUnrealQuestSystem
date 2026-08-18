#include "Public/Quest Managers/QuestManagerSubsystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"

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
		|| CompletedQuestIDs.HasTag(QuestDefinition->QuestID)
		|| !CheckPrerequisites(QuestDefinition))
	{
		return false;
	}
	
	ActiveQuests.Emplace(QuestDefinition->QuestID, FActiveQuestState(QuestDefinition));
	RegisterQuestListeners(QuestDefinition->Objectives[0].TriggerTag);

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

	return CompletedQuestIDs.HasTag(QuestID);
}

bool UQuestManagerSubsystem::CheckPrerequisites(UQuestDefinition* QuestDefinition) const
{
	if (QuestDefinition == nullptr)
	{
		return false;
	}
	
	return CompletedQuestIDs.HasAll(QuestDefinition->RequiredCompletedQuests);
}

void UQuestManagerSubsystem::HandleGameplayEvent(FGameplayTag EventTag, const FQuestEventPayload& EventPayload)
{
	UE_LOG(LogTemp, Log, TEXT("Quest Progressed"));
}

void UQuestManagerSubsystem::RegisterQuestListeners(FGameplayTag EventID)
{
	if (!EventID.IsValid())
	{
		return;
	}

	int32& RefCount = ListenerRefCounts.FindOrAdd(EventID, 0);
	if (RefCount > 0)
	{
		++RefCount;
		return;
	}

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	FGameplayMessageListenerHandle ListenerHandle = GameplayMessageSubsystem.RegisterListener<FQuestEventPayload>(
		EventID, this, &UQuestManagerSubsystem::HandleGameplayEvent);

	ListenerHandles.Emplace(EventID, ListenerHandle);
	++RefCount;
}

void UQuestManagerSubsystem::UnregisterQuestListeners(FGameplayTag EventID)
{
	if (!EventID.IsValid())
	{
		return;
	}
	
	int32* RefCount = ListenerRefCounts.Find(EventID);
	if (!RefCount)
	{
		return;
	}
	
	--(*RefCount);
	if (*RefCount <= 0)
	{
		if (FGameplayMessageListenerHandle* ListenerHandle = ListenerHandles.Find(EventID))
		{
			UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			GameplayMessageSubsystem.UnregisterListener(*ListenerHandle);
			ListenerHandles.Remove(EventID);
		}

		ListenerRefCounts.Remove(EventID);
	}
}

