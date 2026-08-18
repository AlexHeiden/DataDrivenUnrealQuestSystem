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
	if (!EventPayload.InstigatorTag.IsValid() || EventPayload.Amount == 0)
	{
		return;
	}
	
	for (auto& Pair : ActiveQuests)
	{
		FActiveQuestState& ActiveQuestState = Pair.Value;
		if (ActiveQuestState.QuestState != EQuestState::Active)
		{
			continue;
		}
		
		const FQuestObjectiveData& CurrentObjectiveData =
			ActiveQuestState.QuestDefinition->Objectives[ActiveQuestState.CurrentObjectiveIndex];
		FGameplayTagContainer EventTags = EventPayload.ModifierTags;
		EventTags.AddTag(EventPayload.InstigatorTag);
		if (CurrentObjectiveData.TriggerTag != EventTag
			|| !EventTags.HasAll(CurrentObjectiveData.RequiredModifierTags))
		{
			continue;
		}

		FQuestObjectiveProgress& CurrentObjectiveProgress
			= ActiveQuestState.ObjectiveProgresses[ActiveQuestState.CurrentObjectiveIndex];
		CurrentObjectiveProgress.CurrentCount
			= FMath::Max(CurrentObjectiveProgress.CurrentCount + EventPayload.Amount, 0);
		
		if (CurrentObjectiveProgress.CurrentCount >= CurrentObjectiveData.RequiredCount)
		{
			CurrentObjectiveProgress.bCompleted = true;
			ActiveQuestState.CurrentObjectiveIndex++;
			//TODO: unregister old objective listener and subscribe new objective listener or complete quest
		}
	}
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

