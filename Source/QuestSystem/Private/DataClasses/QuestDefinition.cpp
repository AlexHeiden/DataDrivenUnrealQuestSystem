#include "Public/DataClasses/QuestDefinition.h"

const FPrimaryAssetType UQuestDefinition::QuestAssetType(TEXT("Quest"));

FPrimaryAssetId UQuestDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(QuestAssetType, QuestID.GetTagName());
}
