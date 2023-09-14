export module Launcher;

import <Windows.h>;
import <string>;
import <expected>;

export struct GamePath_t {
    std::string RootPath;
    std::string ExecutablePath;
};

export namespace Launcher {
    enum class PathError : uint8_t {
        BAD_LIBRARY_FOLDERS = 0,
        BAD_STEAM_PATH,
        BAD_EXECUTABLE_PATH
    };

    enum class InjectError : uint8_t {
        BAD_DLL_PATH = 0,
        BAD_DISABLE_ASLR,
        BAD_CREATE_PROCESS,
        BAD_VIRTUAL_ALLOC,
        BAD_MEMORY_WRITE,
        BAD_LOADLIBRARYA,
        BAD_CREATE_REMOTE_THREAD
    };

    inline std::expected<GamePath_t, PathError> GetPath(std::string_view FolderName, std::string_view ExecutableName);
    inline std::expected<PROCESS_INFORMATION, InjectError> SpawnAndInject(GamePath_t& Path, const std::string& DllPath);

    template<typename ErrorEnum>
    inline std::string ErrorFromU8(ErrorEnum Error) {
        if constexpr (std::is_same_v<ErrorEnum, Launcher::PathError>) {
            constexpr std::string_view ErrorMessages[] = {
                "BAD_LIBRARY_FOLDERS",
                "BAD_STEAM_PATH",
                "BAD_EXECUTABLE_PATH"
            };
    
            constexpr size_t NumErrorMessages = std::size(ErrorMessages);
            if (static_cast<size_t>(Error) < NumErrorMessages) {
                return std::string(ErrorMessages[static_cast<size_t>(Error)]);
            }
        }
        else if constexpr (std::is_same_v<ErrorEnum, Launcher::InjectError>) {
            constexpr std::string_view ErrorMessages[] = {
                "BAD_DLL_PATH",
                "BAD_DISABLE_ASLR",
                "BAD_CREATE_PROCESS",
                "BAD_VIRTUAL_ALLOC",
                "BAD_MEMORY_WRITE",
                "BAD_LOADLIBRARYW",
                "BAD_CREATE_REMOTE_THREAD"
            };
    
            constexpr size_t NumErrorMessages = std::size(ErrorMessages);
            if (static_cast<size_t>(Error) < NumErrorMessages) {
                return std::string(ErrorMessages[static_cast<size_t>(Error)]);
            }
        }
    
        return "Unknown Error";
    }
}