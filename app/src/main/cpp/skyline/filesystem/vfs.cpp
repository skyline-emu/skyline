#include "constants.h"
#include <sstream>
#include <sys/stat.h>
#include "vfs.h"

namespace skyline::filesystem {
    std::string Vfs::MakeFullPath(std::string path, bool isDir) {
        if (path == "@SdCard" || path == constants::SDLIT)
            path = constants::SDCARD;
        else if (path == constants::USERLIT)
            path = constants::USERNAND;
        else if (path == constants::SYSTEMLIT)
            path = constants::SYSTEMNAND;
        else if (path == constants::SDCONTLIT)
            path = constants::SDCARD + "/Nintendo/Contents";
        else if (path == constants::USERCONTLIT)
            path = constants::USERNAND + "/Contents";
        else if (path == constants::SYSTEMCONTLIT)
            path = constants::SYSTEMNAND + "/Contents";

        path = GetFullPath(path, "");

        if (isDir) {
            struct stat info;
            if (stat(path.c_str(), &info) == 0 && !(info.st_mode & S_IFDIR))
                mkdir(path.c_str(), 0775);
        }

        return path;
    }

    std::string Vfs::GetFullPath(const std::string& basePath, const std::string& filename) {
        // TODO: Use JNI to get dataDir() instead
        return "/data/data/skyline.emu/" + constants::BASEPATH + "/" + basePath + "/" + filename;
    }

    std::string Vfs::GetSdCardPath() {
        return MakeFullPath(constants::SDCARD);
    }

    std::string Vfs::GetNandPath() {
        return MakeFullPath(constants::NAND);
    }

    std::string Vfs::GetSystemPath() {
        return MakeFullPath(constants::SYSTEM);
    }

    std::string Vfs::GetSavePath() {
        // TODO
        return "";
    }

    std::string Vfs::SwitchPathToSystemPath(const std::string switchPath) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(switchPath);
        while (std::getline(tokenStream, token, ':'))
        {
            tokens.push_back(token);
        }

        if (tokens.size() == 2)
            return GetFullPath(MakeFullPath(tokens[0]), tokens[1]);
        else
            return nullptr;
    }
}
