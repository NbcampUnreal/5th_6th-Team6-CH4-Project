

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class Team6_MultiGame : ModuleRules
{
	public Team6_MultiGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "EnhancedInput",
            "NavigationSystem",
            "AIModule",
            "GameplayTasks",
            "UMG",

            "OnlineSubsystem",
            "OnlineSubsystemEOS",
            "OnlineSubsystemUtils",

            "VoiceChat"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        PublicIncludePaths.AddRange(new string[] { "Team6_MultiGame" });
    }
}
