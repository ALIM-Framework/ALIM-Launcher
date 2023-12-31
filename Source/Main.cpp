import Launcher;

import <Windows.h>;
#include <TlHelp32.h>

import <format>;
import <string>;
import <expected>;
import <string_view>;

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR pCmdLine,
                   int nCmdShow)
{
    // Get Path
    //------------------------------------------------------------------------
    std::expected<GamePath_t, Launcher::PathError> PathResult = Launcher::GetPath("Alien Isolation", "AI.exe");
    if (!PathResult.has_value()) {
        Launcher::PathError Error = PathResult.error();
        std::string ErrorString = std::format("Error: {}", Launcher::ErrorFromU8<Launcher::PathError>(Error));
        std::wstring WErrorString = std::wstring(ErrorString.begin(), ErrorString.end());

        MessageBoxW(nullptr, WErrorString.c_str(), L"ALIM-Framework Launcher", MB_OK | MB_ICONERROR);

        return 1;
    }

    GamePath_t AIPath = *PathResult;

    // Launch & Inject
    //------------------------------------------------------------------------
    SetEnvironmentVariableA("SteamAppId", "214490"); // Alien Isolation will think it was launched via Steam and will not re-launch itself.

    std::string DllPath = "ALIM-Core.dll";

    std::expected<PROCESS_INFORMATION, Launcher::InjectError> InjectResult = Launcher::SpawnAndInject(AIPath, DllPath);
    if (!InjectResult.has_value()) {
        Launcher::InjectError Error = InjectResult.error();
        std::string ErrorString = std::format("Failed Injecting: {}", Launcher::ErrorFromU8<Launcher::InjectError>(Error));
        DWORD LastError = GetLastError();
        if (LastError) {
            LPSTR MessageBuf;
            DWORD BufferLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, LastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&MessageBuf, 0, nullptr);

            std::string LastErrorString(MessageBuf, MessageBuf + BufferLen);

            ErrorString += std::format(" ({})", LastErrorString);
            LocalFree(MessageBuf);
        }

        std::wstring WErrorString = std::wstring(ErrorString.begin(), ErrorString.end());
        MessageBoxW(nullptr, WErrorString.c_str(), L"ALIM-Core", MB_OK | MB_ICONERROR);

        return 1;
    }

    PROCESS_INFORMATION ProcessInfo = *InjectResult;

    ResumeThread(ProcessInfo.hThread);
    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

    CloseHandle(ProcessInfo.hProcess);
    CloseHandle(ProcessInfo.hThread);

    return 0;
}