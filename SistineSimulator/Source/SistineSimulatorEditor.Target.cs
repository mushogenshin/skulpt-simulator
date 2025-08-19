using UnrealBuildTool;
using System.Collections.Generic;

public class SistineSimulatorEditorTarget : TargetRules
{
	public SistineSimulatorEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "SistineSimulatorGame" } );
	}
}
