// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CodeAI : ModuleRules
{
	public CodeAI(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "AIModule", "UMG", "Slate", "SlateCore" , "GameplayTasks"});
	}
}
