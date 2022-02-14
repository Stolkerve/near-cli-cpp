newoption {
    trigger = "arch",
    description = "Set the current target architecture",
    default = "x86",
    allowed = {
        { "x86", "32 bit target architecture" },
        { "x86_64", "64 bit target architecture" },
    }
}

workspace "near-cli-cpp"  
	configurations { "Debug", "Release" }
	architecture (_OPTIONS["arch"])
	startproject "near-cli-cpp"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

group "Dependencies"
	include "./near-cli-cpp/dependencies/libsodium"

group ""
	include "./near-cli-cpp/"
