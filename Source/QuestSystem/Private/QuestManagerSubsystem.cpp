#include "Public/Quest Managers/QuestManagerSubsystem.h"

void UQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UQuestManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

int UQuestManagerSubsystem::Test()
{
	return CompletedQuestIDs.Num();
}
