using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SpineEditorPlugin : ModuleRules
	{
		public SpineEditorPlugin(ReadOnlyTargetRules Target) : base(Target)
		{
            //enable IWYU
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../SpinePlugin/Public/spine-cpp/include"));

			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../SpinePlugin/Public/spine-cpp/include"));



            PublicDependencyModuleNames.AddRange(
                new string[] {
                "SpinePlugin",
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd" ,
                "AssetRegistry",
                "AssetTools",
                });

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                        "BlueprintGraph",
                        "KismetCompiler",
                        "GraphEditor",
                        "Slate",
                        "SlateCore",
                        "EditorStyle",
                        "PropertyEditor",
                        "EditorWidgets",
                         "ClassViewer",
                         "InternationalizationSettings",
                         "ConfigEditor",
                         "InputCore",
                }
            );
        }
	}
}
