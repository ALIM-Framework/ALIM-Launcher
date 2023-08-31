workspace "ALIM-Launcher"
    configurations { "Debug", "Release" }

    filter "configurations:*"
        architecture "x86"

    cppdialect "C++latest"
    staticruntime "on"
    language "C++"

project "ALIM-Launcher"
    kind "ConsoleApp"
    targetdir "Build/%{cfg.buildcfg}"
    objdir "Build/Intermediate/%{cfg.buildcfg}"

    files { "Source/**.cpp", "Modules/**.ixx", "Modules/**.cxx" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"








require('vstudio')

local p = premake
local m = p.vstudio.vc2010

m.elements.clCompile = function(cfg)
	local calls = {
        function(cfg) m.element("ScanSourceForModuleDependencies", nil, "true") end,
        function(cfg) m.element("EnableModules", nil, "true") end,
        function(cfg) m.element("BuildStlModules", nil, "true") end,

		m.precompiledHeader,
		m.warningLevel,
		m.treatWarningAsError,
		m.disableSpecificWarnings,
		m.treatSpecificWarningsAsErrors,
		m.basicRuntimeChecks,
		m.clCompilePreprocessorDefinitions,
		m.clCompileUndefinePreprocessorDefinitions,
		m.clCompileAdditionalIncludeDirectories,
		m.clCompileAdditionalUsingDirectories,
		m.forceIncludes,
		m.debugInformationFormat,
		m.optimization,
		m.functionLevelLinking,
		m.intrinsicFunctions,
		m.justMyCodeDebugging,
		m.supportOpenMP,
		m.minimalRebuild,
		m.omitFramePointers,
		m.stringPooling,
		m.runtimeLibrary,
		m.omitDefaultLib,
		m.exceptionHandling,
		m.runtimeTypeInfo,
		m.bufferSecurityCheck,
		m.treatWChar_tAsBuiltInType,
		m.floatingPointModel,
		m.floatingPointExceptions,
		m.inlineFunctionExpansion,
		m.enableEnhancedInstructionSet,
		m.multiProcessorCompilation,
		m.additionalCompileOptions,
		m.compileAs,
		m.callingConvention,
		m.languageStandard,
		m.languageStandardC,
		m.conformanceMode,
		m.structMemberAlignment,
		m.useFullPaths,
		m.removeUnreferencedCodeData,
		m.compileAsWinRT,
		m.externalWarningLevel,
		m.externalAngleBrackets,
		m.scanSourceForModuleDependencies,
		m.useStandardPreprocessor,
	}

	if cfg.kind == p.STATICLIB then
		table.insert(calls, m.programDatabaseFileName)
	end

	return calls
end