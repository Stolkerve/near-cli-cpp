project "near-cli-cpp"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	vectorextensions "Default"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
	}

	includedirs
	{
		"src",
		"dependencies/cpp-httplib",
		"dependencies/nlohmann",
		"dependencies/libsodium/src/libsodium/include"
	}

	filter "configurations:Debug"
		libdirs {
			"dependencies/libsodium/bin/%{cfg.buildcfg}-%{cfg.system}/libsodium/",
		}

	filter "configurations:Release"
		libdirs {
			"dependencies/libsodium/bin/%{cfg.buildcfg}-%{cfg.system}/libsodium/",
		}

	filter "system:linux"
		pic "On"
		systemversion "latest"
		links
		{
			"dl",
			"pthread",
			"libsodium",
			"stdc++fs"
		}
	filter "system:windows"
		systemversion "latest"

		links
		{
			"ws2_32.lib",
			"libsodium"
		}

	filter "configurations:Debug"
		defines "NEAR_CPP_CLIENT_DEBUG"
		runtime "Debug"
		warnings "extra"
		symbols "on"
		buildoptions "-g3"

	filter "configurations:Release"
		defines "NEAR_CPP_CLIENT_RELEASE"
		runtime "Release"
		optimize "Full"
        warnings "extra"