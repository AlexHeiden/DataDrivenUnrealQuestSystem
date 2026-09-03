#include "Public/QuestManagers/QuestManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Public/DataClasses//QuestSaveGame.h"

void UQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	OnQuestRewardGranted.AddDynamic(this, &UQuestManagerSubsystem::DebugLogQuestRewardGranted);
}

void UQuestManagerSubsystem::Deinitialize()
{
	OnQuestRewardGranted.Clear();
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
	// Reject: null/invalid quest, no objectives, active/completed quest, prerequisutes unmet
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

void UQuestManagerSubsystem::SaveQuestState() const
{
	UQuestSaveGame* SaveObject = Cast<UQuestSaveGame>(UGameplayStatics::CreateSaveGameObject(UQuestSaveGame::StaticClass()));
	SaveObject->CompletedQuestIDs = CompletedQuestIDs;

	for (auto& Pair : ActiveQuests)
	{
		FQuestSaveData QuestSaveData;
		QuestSaveData.CurrentObjectiveIndex = Pair.Value.CurrentObjectiveIndex;
		QuestSaveData.ObjectiveProgresses = Pair.Value.ObjectiveProgresses;

		SaveObject->QuestProgresses.Add(Pair.Key, QuestSaveData);
	}

	UGameplayStatics::SaveGameToSlot(SaveObject, TEXT("SaveSlot1"), 0);
}

void UQuestManagerSubsystem::LoadQuestState()
{
	UQuestSaveGame* SaveObject = Cast<UQuestSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SaveSlot1"), 0));
	if (!SaveObject)
	{
		return;
	}
	
	CompletedQuestIDs = SaveObject->CompletedQuestIDs;
	ActiveQuests.Empty();
	for (const auto& Pair : SaveObject->QuestProgresses)
	{
		UQuestDefinition* QuestDefinition = ResolveQuestDefinition(Pair.Key);
		if (!QuestDefinition)
		{
			continue;
		}

		FActiveQuestState ActiveQuestState(QuestDefinition);
		ActiveQuestState.CurrentObjectiveIndex = Pair.Value.CurrentObjectiveIndex;
		ActiveQuestState.ObjectiveProgresses = Pair.Value.ObjectiveProgresses;

		ActiveQuests.Emplace(Pair.Key, MoveTemp(ActiveQuestState));
		RegisterQuestListeners(QuestDefinition->Objectives[ActiveQuestState.CurrentObjectiveIndex].TriggerTag);
	}

	OnQuestsLoaded.Broadcast();
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

	// Must unregister using the current objective's TriggerTag before moving on to the next objective/quest end
	UnregisterQuestListeners(ActiveQuestState->QuestDefinition->Objectives[ActiveQuestState->CurrentObjectiveIndex].TriggerTag);
	CurrentObjectiveProgress->bCompleted = true;
	ActiveQuestState->CurrentObjectiveIndex++;
}

void UQuestManagerSubsystem::HandleGameplayEvent(FGameplayTag EventTag, const FQuestEventPayload& EventPayload)
{
	// If Amount == 0, then there would be no objective progress at all, skip entirely
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
		// InstigatorTag should be in the RequiredModifierTags too
		// Adding InstigatorTag to other tags for a single HasAll() check
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
		
		if (!IsObjectiveCompleted(&CurrentObjectiveProgress, &CurrentObjectiveData))
		{
			OnObjectiveProgressChanged.Broadcast(ActiveQuestState.QuestDefinition->QuestID, ActiveQuestState.CurrentObjectiveIndex);
			continue;
		}
		CompleteCurrentObjective(&CurrentObjectiveProgress, &ActiveQuestState);
		
		if (!HasCompletedAllObjectives(&ActiveQuestState))
		{
			OnObjectiveProgressChanged.Broadcast(ActiveQuestState.QuestDefinition->QuestID, ActiveQuestState.CurrentObjectiveIndex);
			RegisterQuestListeners(ActiveQuestState.QuestDefinition->Objectives[ActiveQuestState.CurrentObjectiveIndex].TriggerTag);
			continue;
		}

		// We can't remove the quests in a foreach loop, so we save their ids to delete them afterwards
		QuestsToComplete.Add(ActiveQuestState.QuestDefinition->QuestID);
	}
	
	for (FGameplayTag& QuestID : QuestsToComplete)
	{
		CompletedQuestIDs.AddTag(QuestID);
		const int32 RewardXP = ActiveQuests.FindAndRemoveChecked(QuestID).QuestDefinition->RewardXP;

		OnQuestStateChanged.Broadcast(QuestID);
		OnQuestRewardGranted.Broadcast(QuestID, RewardXP);
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

void UQuestManagerSubsystem::DebugLogQuestRewardGranted(FGameplayTag QuestID, int32 RewardXP)
{
	UE_LOG(LogTemp, Log, TEXT("Quest reward granted. %s: %d XP"), *QuestID.ToString(), RewardXP);
}

// Always resolves via path + TryLoad()
// A larger project might cache already loaded quests instead
UQuestDefinition* UQuestManagerSubsystem::ResolveQuestDefinition(FGameplayTag QuestID)
{
	if (!QuestID.IsValid())
	{
		return nullptr;
	}
	
	FPrimaryAssetId AssetId(UQuestDefinition::QuestAssetType, QuestID.GetTagName());
	UAssetManager& AssetManager = UAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
	if (AssetPath.IsValid())
	{
		return Cast<UQuestDefinition>(AssetPath.TryLoad());
	}

	return nullptr;
}

