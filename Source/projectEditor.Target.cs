using UnrealBuildTool;
using System.Collections.Generic;

public class projectEditorTarget : TargetRules
{
	public projectEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("project");

		// 当前本机 UE_5.4 安装中的 MegascansPlugin 是 UE4 旧插件结构，
		// 描述文件声明了模块但缺少 Source 模块目录，会阻止 Editor 目标编译。
		DisablePlugins.Add("MegascansPlugin");
	}
}
