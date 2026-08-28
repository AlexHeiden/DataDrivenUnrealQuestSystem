#include "Public/Quest Managers/QuestManagerSubsystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"

void UQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	OnQuestStateChanged.AddDynamic(this, &UQuestManagerSubsystem::UQuestManagerSubsystem::DebugLogQuestStateChanged);
	OnObjectiveProgressChanged.AddDynamic(this, &UQuestManagerSubsystem::DebugLogObjectiveProgressChanged);
}

void UQuestManagerSubsystem::Deinitialize()
{
	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	for (auto& Pair: ListenerHandles)
	{
		GameplayMessageSubsystem.UnregisterListener(Pair.Value);
	}

	ListenerHandles.Empty();
	ListenerRefCounts.Empty();
	
	Super::Deinitialize();
}

bool UQuestManagerSubsystem::AcceptQuest(UQuestDefinition* QuestDefinition)
{
	if (QuestDefinition == nullptr
		|| !QuestDefinition->QuestID.IsValid()
		|| QuestDefinition->Objectives.Num() == 0
		|| ActiveQuests.Contains(QuestDefinition->QuestID)
		|| CompletedQuestIDs.HasTag(QuestDefinition->QuestID)
		|| !CheckPrerequisites(QuestDefinition))
	{
		return false;
	}

	FActiveQuestState ActiveQuestState(QuestDefinition);
	RegisterQuestListeners(QuestDefinition->Objectives[ActiveQuestState.CurrentObjectiveIndex].TriggerTag);
	ActiveQuests.Emplace(QuestDefinition->QuestID, MoveTemp(ActiveQuestState));

	OnQuestStateChanged.Broadcast(QuestDefinition->QuestID);
	
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

TArray<FGameplayTag> UQuestManagerSubsystem::GetActiveQuestIDs() const
{
	TArray<FGameplayTag> ActiveQuestIDs;
	ActiveQuests.GetKeys(ActiveQuestIDs);
	return ActiveQuestIDs;
}

FText UQuestManagerSubsystem::GetQuestName(FGameplayTag QuestID) const
{
	if (!ActiveQuests.Contains(QuestID))
	{
		return FText::GetEmpty();
	}

	return ActiveQuests.Find(QuestID)->QuestDefinition->QuestName;
}

FText UQuestManagerSubsystem::GetCurrentObjectiveText(FGameplayTag QuestID) const
{
	const FActiveQuestState* ActiveQuestState = ActiveQuests.Find(QuestID);
	if (!ActiveQuestState || ActiveQuestState->CurrentObjectiveIndex >= ActiveQuestState->ObjectiveProgresses.Num())
	{
		return FText::GetEmpty();
	}

	const FQuestObjectiveData& CurrentObjectiveData = ActiveQuestState->QuestDefinition->Objectives[ActiveQuestState->CurrentObjectiveIndex];
	const FQuestObjectiveProgress& CurrentObjectiveProgress = ActiveQuestState->ObjectiveProgresses[ActiveQuestState->CurrentObjectiveIndex];
	return FText::Format(
		NSLOCTEXT("Quest", "ObjectiveFormat", "{0}: {1}/{2}"),
		CurrentObjectiveData.Description,
		CurrentObjectiveProgress.CurrentCount,
		CurrentObjectiveData.RequiredCount
	);
}

bool UQuestManagerSubsystem::CheckPrerequisites(const UQuestDefinition* QuestDefinition) const
{
	if (QuestDefinition == nullptr)
	{
		return false;
	}
	
	return CompletedQuestIDs.HasAll(QuestDefinition->RequiredCompletedQuests);
}

bool UQuestManagerSubsystem::IsObjectiveCompleted(const FQuestObjectiveProgress* ObjectiveProgress,
	const FQuestObjectiveData* ObjectiveData)
{
	if (ObjectiveProgress == nullptr || ObjectiveData == nullptr)
	{
		return false;
	}

	return ObjectiveProgress->CurrentCount >= ObjectiveData->RequiredCount;
}

bool UQuestManagerSubsystem::HasCompletedAllObjectives(const FActiveQuestState* ActiveQuestState)
{
	if (ActiveQuestState == nullptr)
	{
		return false;
	}

	return ActiveQuestState->CurrentObjectiveIndex >= ActiveQuestState->QuestDefinition->Objectives.Num();
}

void UQuestManagerSubsystem::CompleteCurrentObjective(FQuestObjectiveProgress* CurrentObjectiveProgress,
                                                      FActiveQuestState* ActiveQuestState)
{
	if (CurrentObjectiveProgress == nullptr || ActiveQuestState == nullptr)
	{
		return;
	}
	
	UnregisterQuestListeners(ActiveQuestState->QuestDefinition->Objectives[ActiveQuestState->CurrentObjectiveIndex].TriggerTag);
	CurrentObjectiveProgress->bCompleted = true;
	ActiveQuestState->CurrentObjectiveIndex++;
}

void UQuestManagerSubsystem::HandleGameplayEvent(FGameplayTag EventTag, const FQuestEventPayload& EventPayload)
{
	if (!EventPayload.InstigatorTag.IsValid() || EventPayload.Amount == 0)
	{
		return;
	}

	TArray<FGameplayTag> QuestsToComplete;
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

		// Check that the objective was listening for this event
		// If it was, check that event has all required objective tags
		if (CurrentObjectiveData.TriggerTag != EventTag
			|| !EventTags.HasAll(CurrentObjectiveData.RequiredModifierTags))
		{
			continue;
		}

		// Progress the objective
		FQuestObjectiveProgress& CurrentObjectiveProgress
			= ActiveQuestState.ObjectiveProgresses[ActiveQuestState.CurrentObjectiveIndex];
		CurrentObjectiveProgress.CurrentCount
			= FMath::Max(CurrentObjectiveProgress.CurrentCount + EventPayload.Amount, 0);

		OnObjectiveProgressChanged.Broadcast(ActiveQuestState.QuestDefinition->QuestID, ActiveQuestState.CurrentObjectiveIndex);
		
		if (!IsObjectiveCompleted(&CurrentObjectiveProgress, &CurrentObjectiveData))
		{
			continue;
		}
		CompleteCurrentObjective(&CurrentObjectiveProgress, &ActiveQuestState);
		
		if (!HasCompletedAllObjectives(&ActiveQuestState))
		{
			RegisterQuestListeners(ActiveQuestState.QuestDefinition->Objectives[ActiveQuestState.CurrentObjectiveIndex].TriggerTag);
			continue;
		}

		QuestsToComplete.Add(ActiveQuestState.QuestDefinition->QuestID);
	}

	for (FGameplayTag& QuestID : QuestsToComplete)
	{
		CompletedQuestIDs.AddTag(QuestID);
		ActiveQuests.Remove(QuestID);

		OnQuestStateChanged.Broadcast(QuestID);
	}
}

void UQuestManagerSubsystem::RegisterQuestListeners(FGameplayTag EventID)
{
	if (!EventID.IsValid())
	{
		return;
	}

	int32& RefCount = ListenerRefCounts.FindOrAdd(EventID, 0);
	if (RefCount == 0)
	{
		UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		FGameplayMessageListenerHandle ListenerHandle = GameplayMessageSubsystem.RegisterListener<FQuestEventPayload>(
			EventID, this, &UQuestManagerSubsystem::HandleGameplayEvent);
		ListenerHandles.Emplace(EventID, ListenerHandle);
	}
	
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

void UQuestManagerSubsystem::DebugLogQuestStateChanged(FGameplayTag QuestID)
{
	UE_LOG(LogTemp, Log, TEXT("Quest state changed: %s"), *QuestID.ToString())
}

void UQuestManagerSubsystem::DebugLogObjectiveProgressChanged(FGameplayTag QuestID, int32 ObjectiveIndex)
{
	UE_LOG(LogTemp, Log, TEXT("Quest objective changed: %s, objective: %d"), *QuestID.ToString(), ObjectiveIndex);
}
