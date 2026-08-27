#include "Actor Quest Components/QuestBroadcasterComponent.h"

#include "Data Classes/QuestData.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void UQuestBroadcasterComponent::BroadcastQuestEvent()
{
	if (!EventTag.IsValid())
	{
		return;
	}

	FQuestEventPayload Payload;
	Payload.InstigatorTag = InstigatorTag;
	Payload.ModifierTags = ModifierTags;
	Payload.Amount = Amount;

	UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(EventTag, Payload);
}
