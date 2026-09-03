#include "Public/ActorQuestComponents/QuestBroadcasterComponent.h"

#include "Public/DataClasses/QuestData.h"
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
