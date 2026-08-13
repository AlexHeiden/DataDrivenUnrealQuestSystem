// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class QuestSystem : ModuleRules
{
	public QuestSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"QuestSystem",
			"QuestSystem/Variant_Platforming",
			"QuestSystem/Variant_Platforming/Animation",
			"QuestSystem/Variant_Combat",
			"QuestSystem/Variant_Combat/AI",
			"QuestSystem/Variant_Combat/Animation",
			"QuestSystem/Variant_Combat/Gameplay",
			"QuestSystem/Variant_Combat/Interfaces",
			"QuestSystem/Variant_Combat/UI",
			"QuestSystem/Variant_SideScrolling",
			"QuestSystem/Variant_SideScrolling/AI",
			"QuestSystem/Variant_SideScrolling/Gameplay",
			"QuestSystem/Variant_SideScrolling/Interfaces",
			"QuestSystem/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
