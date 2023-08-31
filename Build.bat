@echo off
setlocal

premake5 vs2022
msbuild /p:Configuration="Debug" /p:Platform="Win32" /verbosity:minimal /p:BuildStlModules=true /p:EnableModules=true /p:BuildStlModules=true "ALIM-Launcher.sln" /m