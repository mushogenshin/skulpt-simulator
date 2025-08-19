using UnrealBuildTool;

public class SistineSimulatorEditor : ModuleRules
{
	public SistineSimulatorEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"Engine",
			"CoreUObject",
			"UnrealEd", // Main editor module
			"SistineSimulatorGame", // Our game module
			"Blutility", "EditorScriptingUtilities",
			"UMGEditor", "ModelViewViewModelBlueprint",
			// "LevelSequence", "ControlRig", "ControlRigDeveloper", "IKRig",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"EditorStyle",
			// ... other editor modules you might need
		});
	}
}