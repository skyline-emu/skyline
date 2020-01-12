#pragma once

#include "constants.h"
#include <vector>

namespace skyline::filesystem {
    class Vfs {
      private:
        /**
         * @brief Make a directory for emulator
         * @param path path from appdir/vfs
         * @param isDir gien path is dir
         * @return Full directory path from android root
         */
        std::string MakeFullPath(std::string path, bool isDir = true);

      public:
        std::vector<__uint8_t> romFs; //!< romFs buffer to be written

        /**
         * @brief Get full path of vfs directories
         * @param basePath basePath of a file
         * @param filename name of the file
         * @return full string path from root of device
         */
        std::string GetFullPath(const std::string& basePath, const std::string& filename);

        /**
         * @brief Get vfs sdcard path
         * @return vfs sdcard path
         */
        std::string GetSdCardPath();

        /**
         * @brief Get vfs nand path
         * @return vfs nand path
         */
        std::string GetNandPath();

        /**
         * @brief Get vfs System path
         * @return vfs System path
         */
        std::string GetSystemPath();

        /**
         * @brief Get vfs Save path (TODO)
         * @return vfs Save path
         */
        std::string GetSavePath();

        /**
         * @brief Convert switch path to recognizable android path
         * @param switchPath path valid in switch
         * @return path valid in android
         */
        std::string SwitchPathToSystemPath(std::string switchPath);
    };
}
