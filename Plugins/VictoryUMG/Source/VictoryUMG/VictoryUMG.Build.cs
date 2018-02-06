/*
	By Rama for You
	
	You are welcome to use this code anywhere as long as you include this notice.
	
	copyright 2015
*/
using UnrealBuildTool;

public class VictoryUMG : ModuleRules
{
	public VictoryUMG(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				
				"RHI",
				"RenderCore",
				 
				"UMG", "Slate", "SlateCore", 
                
                "APPFRAMEWORK" //for color picker! -Rama
		
			}
		);
		//Private Paths
        PrivateIncludePaths.AddRange(new string[] { 
			"VictoryUMG/Public",
			"VictoryUMG/Private"
		});
	}
}
