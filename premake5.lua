workspace "Ohm"
	architecture "x64"
	startproject "OhmEditor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Ohm"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	location "Ohm"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs 
	{
		"%{prj.name}/src"
	}

	pchheader "ohmpch.h"
	pchsource "%{prj.name}/src/ohmpch.cpp"

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "OHM_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "OHM_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "OHM_DIST"
		runtime "Release"
		optimize "on"

project "OhmEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	location "OhmEditor"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs 
	{
		"Ohm/src"
	}

	links 
	{
		"Ohm"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "OHM_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "OHM_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "OHM_DIST"
		runtime "Release"
		optimize "on"


