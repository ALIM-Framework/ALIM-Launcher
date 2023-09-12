import Launcher;

import <Windows.h>;
import <filesystem>;
import <fstream>;
import <string>;
import <expected>;

namespace fs = std::filesystem;

std::string GetCommandLineWithoutProgramName() {
    LPSTR CommandLine = GetCommandLineA();
    std::string commandLineStr = CommandLine;

    size_t SpacePos = commandLineStr.find(' ');
    if (SpacePos != std::string::npos)
        return commandLineStr.substr(SpacePos + 1);

    return "";
}

std::optional<std::string> GetSteamPath() {
    char cSteamPath[MAX_PATH];
    HKEY hSteamKey;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(SOFTWARE\WOW6432Node\Valve\Steam)", 0, KEY_QUERY_VALUE, &hSteamKey) != ERROR_SUCCESS)
        return std::nullopt;

    DWORD dwLen = MAX_PATH;
    if (RegQueryValueExA(hSteamKey, "InstallPath", nullptr, nullptr, reinterpret_cast<LPBYTE>(&cSteamPath), &dwLen) == ERROR_SUCCESS) {
        cSteamPath[dwLen - 1] = '\0';
        RegCloseKey(hSteamKey);
        return cSteamPath;
    }

    return std::nullopt;
}

std::expected<GamePath_t, Launcher::PathError> Launcher::GetPath(std::string_view FolderName, std::string_view ExecutableName) {
    auto SteamPathOpt = GetSteamPath();
    if (!SteamPathOpt.has_value())
        return std::unexpected(Launcher::PathError::BAD_STEAM_PATH);

    std::string SteamPath = *SteamPathOpt;

    std::ifstream ConfigFile(SteamPath + "\\steamapps\\libraryfolders.vdf");
    if (!ConfigFile.is_open())
        return std::unexpected(Launcher::PathError::BAD_LIBRARY_FOLDERS);

    std::string Line;
    while (std::getline(ConfigFile, Line)) {
        if (Line.find("\"path\"") != std::string::npos) {
            size_t FirstQuote = Line.find('"', 7);
            if (FirstQuote == std::string::npos)
                continue;

            size_t SecondQuote = Line.find('"', FirstQuote + 1);
            if (SecondQuote == std::string::npos)
                continue;

            std::string Path = Line.substr(FirstQuote + 1);
            Path.erase(0, 3);
            Path.erase(Path.size() - 1);

            std::filesystem::path GameDirectoryPath = std::filesystem::path(Path) / "steamapps" / "common" / FolderName;
            std::filesystem::path GamePath = std::filesystem::path(Path) / "steamapps" / "common" / FolderName / ExecutableName;
            
            if (std::filesystem::exists(GamePath))
                return GamePath_t{GameDirectoryPath.string(), GamePath.string()};
        }
    }

    return std::unexpected(Launcher::PathError::BAD_EXECUTABLE_PATH);
}

// https://xionghuilin.com/c-convert-between-string-and-cstring-lpwstr/
LPWSTR StringToLPWSTR(const std::string& String) {
    int StringSize = static_cast<int>(String.size());
    int BufferLen = ::MultiByteToWideChar(CP_ACP, 0, String.c_str(), StringSize, NULL, 0);

    if (BufferLen == 0)
        return 0;

    LPWSTR WideString = new WCHAR[BufferLen + 1];

    ::MultiByteToWideChar(CP_ACP, 0, String.c_str(), StringSize, WideString, BufferLen);

    WideString[BufferLen] = 0;
    return WideString;
}

std::expected<PROCESS_INFORMATION, Launcher::InjectError> Launcher::SpawnAndInject(GamePath_t& Path, const std::string& DllPath) {
    PROCESS_INFORMATION ProcessInfo = { 0 };
    STARTUPINFOW StartupInfo = { 0 };
    StartupInfo.cb = sizeof(StartupInfo);

    if (!fs::exists(DllPath))
        return std::unexpected(Launcher::InjectError::BAD_DLL_PATH);

    std::string AbsoluteDllPath = fs::absolute(DllPath).string();

    Path.ExecutablePath += " -launch-dir=\"" + fs::current_path().string() + "\" " + GetCommandLineWithoutProgramName();

    std::wstring RootDirectory = std::wstring(Path.RootPath.begin(), Path.RootPath.end());
    if (!CreateProcessW(nullptr, StringToLPWSTR(Path.ExecutablePath), nullptr, nullptr, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED,
        nullptr, RootDirectory.c_str(), &StartupInfo, &ProcessInfo))
    {
        return std::unexpected(Launcher::InjectError::BAD_CREATE_PROCESS);
    }

    LPVOID RemoteDLL = VirtualAllocEx(ProcessInfo.hProcess, nullptr, AbsoluteDllPath.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!RemoteDLL)
        return std::unexpected(Launcher::InjectError::BAD_VIRTUAL_ALLOC);

    if (!WriteProcessMemory(ProcessInfo.hProcess, RemoteDLL, AbsoluteDllPath.c_str(), AbsoluteDllPath.length(), nullptr))
        return std::unexpected(Launcher::InjectError::BAD_MEMORY_WRITE);

    LPVOID LoadLibraryAddress = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryA");
    if (!LoadLibraryAddress)
        return std::unexpected(Launcher::InjectError::BAD_LOADLIBRARYA);

    HANDLE Thread = CreateRemoteThread(ProcessInfo.hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryAddress), RemoteDLL, 0, nullptr);
    if (!Thread) {
        CloseHandle(ProcessInfo.hThread);
        return std::unexpected(Launcher::InjectError::BAD_CREATE_REMOTE_THREAD);
    }

    VirtualFreeEx(ProcessInfo.hProcess, Thread, AbsoluteDllPath.length(), MEM_RELEASE);
    return ProcessInfo;
}