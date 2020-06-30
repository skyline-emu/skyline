// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <array>
#include <common.h>
#include "backing.h"

namespace skyline::vfs {
    /**
     * @brief The NACP class provides easy access to the data found in an NACP file (https://switchbrew.org/wiki/NACP_Format)
     */
    class NACP {
      private:
        /**
         * @brief This contains the name and publisher of an application for one language
         */
        struct ApplicationTitle {
            std::array<char, 0x200> applicationName; //!< The name of the application
            std::array<char, 0x100> applicationPublisher; //!< The publisher of the application
        };
        static_assert(sizeof(ApplicationTitle) == 0x300);

        /**
         * @brief This struct encapsulates all the data within an NACP file
         */
        struct NacpData {
            std::array<ApplicationTitle, 0x10> titleEntries; //!< Title entries for each language
            u8 _pad_[0x4000 - (0x10 * 0x300)];
        } nacpContents{};
        static_assert(sizeof(NacpData) == 0x4000);

      public:
        /**
         * @param backing The backing for the NACP
         */
        NACP(const std::shared_ptr<vfs::Backing> &backing);

        std::string applicationName; //!< The name of the application in the currently selected language
        std::string applicationPublisher; //!< The publisher of the application in the currently selected language
    };
}