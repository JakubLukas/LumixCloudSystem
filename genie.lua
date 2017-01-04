project "LumixCloudSystem"
	libType()
	files { 
		"src/**.cpp",
		"src/**.h",
		"genie.lua"
	}
	includedirs { "../../CloudSystem/src", }
	defines { "BUILDING_CLOUDS" }
	links { "engine" }
	useLua()
	includedirs { "../LumixEngine/external/lua/include", "../LumixEngine/external/bgfx/include" }
	defaultConfigurations()