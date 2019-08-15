using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SpinePlugin : ModuleRules
	{
		public SpinePlugin(ReadOnlyTargetRules Target) : base(Target)
		{
            //development build 也不需要优化代码.
            OptimizeCode = CodeOptimization.InShippingBuildsOnly;

            // PrivatePCHHeaderFile = "Private/SpinePluginPrivatePCH.h";

            //enable IWYU
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/spine-cpp/include"));

			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/spine-cpp/include"));

            PublicDependencyModuleNames.AddRange(new string[] { "Core","GameplayTags", "GameplayAbilities", "CoreUObject", "Engine", "RHI", "ProceduralMeshComponent", "UMG", "Slate", "SlateCore" });

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
         //       PrivateDependencyModuleNames.Add("Slate");
            }

        }
    }
}
