using UnrealBuildTool;

public class MediaPipeDemo : ModuleRules
{
    public MediaPipeDemo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "MediaPipe"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}
