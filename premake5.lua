-- premake5.lua project solution script

cfg_systemversion = "latest" -- "10.0.17763.0"   -- To use the latest version of the SDK available

-- solution
workspace "AnotherSoftRenderer"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }

    location "Build"
    defines { "_CRT_SECURE_NO_WARNINGS" }
    systemversion(cfg_systemversion)
	
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        targetsuffix("_d")

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"    

    filter "platforms:Win32"
        architecture "x32"

    filter "platforms:Win64"
        architecture "x64"

    filter {}

    targetdir("Build/Bin")  
    objdir("Build/Obj/%{prj.name}/%{cfg.buildcfg}")
	debugdir ("Build/..");
	
-- project renderer
project "Renderer"
    language "C++"
    kind "StaticLib"

    includedirs {
        "./ThirdParty/glm",
		"./ThirdParty/stb",
		"./ThirdParty/tinyobjloader",
		"./Renderer/Include"
    }
	
	vpaths {
		["stb"] = {
			"./ThirdParty/stb/*.h",
			"./ThirdParty/stb/*.cc"
		},
		["tinyobjloader"] = {
			"./ThirdParty/tinyobjloader/*.h",
			"./ThirdParty/tinyobjloader/*.cc"
		}
	}
	
    files {
		"./ThirdParty/stb/*.h",
		"./ThirdParty/stb/*.cc",
		"./ThirdParty/tinyobjloader/*.h",
		"./ThirdParty/tinyobjloader/*.cc",
		
        "./Renderer/Include/*.h",
		"./Renderer/Source/*.cc"
    }

-- project viewer
project "Viewer"
    language "C++"
    kind "ConsoleApp"

	dependson { "Renderer" }
	links { "Renderer" }
	
    includedirs {
        "./ThirdParty/glm",
		"./ThirdParty/stb",
		"./ThirdParty/tinyobjloader",
		"./Renderer/Include"
    }
	
    files {
		"./Viewer/Include/*.h",
		"./Viewer/Source/*.cc"
    }

-- project realtime viewer
project "RealTimeViewer"
    language "C++"
    kind "ConsoleApp"

	dependson { "Renderer" }
	links { "Renderer" }
	
-- third library cflags and libs
	includedirs { "./ThirdParty/SDL/include" }
	libdirs { 
		"./ThirdParty/SDL/lib/%{cfg.architecture:gsub('x86_64', 'x64')}"
	}
	links {
		"SDL2",
		"SDL2main"
	}
	
	postbuildcommands {
		-- Copy the SDL2 dll to the Bin folder.
		'{COPY} "%{path.getabsolute("./ThirdParty/SDL/lib/" .. cfg.architecture:gsub("x86_64", "x64") .. "/SDL2.dll")}" "%{cfg.targetdir}"',
	}
	
    includedirs {
        "./ThirdParty/glm",
		"./ThirdParty/stb",
		"./ThirdParty/tinyobjloader",
		"./Renderer/Include",
		"./RealTimeViewer/Include"
    }
	
    files {
		"./RealTimeViewer/Include/*.h",
		"./RealTimeViewer/Source/*.cc"
    }