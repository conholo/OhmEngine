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

IncludeDirectories = {}
IncludeDirectories["GLFW"] = "Ohm/vendor/GLFW/include"
IncludeDirectories["glad"] = "Ohm/vendor/glad/include"
IncludeDirectories["glm"] = "Ohm/vendor/glm"
IncludeDirectories["entt"] = "Ohm/vendor/entt/include"


group "Dependencies"
	include "Ohm/vendor/GLFW"
	include "Ohm/vendor/glad"
group ""

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
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/entt/include/entt.hpp",
	}

	includedirs 
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDirectories.GLFW}",
		"%{IncludeDirectories.glad}",
		"%{IncludeDirectories.glm}",
		"%{IncludeDirectories.entt}",
	}

	links
	{
		"GLFW",
		"glad"
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
		"Ohm/src",
		"Ohm/vendor",
		"Ohm/vendor/spdlog/include",
		"%{IncludeDirectories.glm}",
		"%{IncludeDirectories.entt}",
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


