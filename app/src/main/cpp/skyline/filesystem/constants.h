#pragma once

#include <string>

namespace skyline::filesystem::constants {
    const std::string BASEPATH = "vfs";
    const std::string NAND = "sky-nand";
    const std::string SDCARD = "sky-sdcard";
    const std::string SYSTEM = "sky-system";
    const std::string SAFENAND = "sky-nand/safe";
    const std::string SYSTEMNAND = "sky-nand/system";
    const std::string USERNAND = "sky-nand/user";

    const std::string SDLIT = "@Sdcard";
    const std::string USERLIT = "@User";
    const std::string SYSTEMLIT = "@System";
    const std::string SDCONTLIT = "@SdCardContent";
    const std::string USERCONTLIT = "@UserContent";
    const std::string SYSTEMCONTLIT = "@SystemContent";
}
