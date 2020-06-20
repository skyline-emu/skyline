// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "nacp.h"

namespace skyline::vfs {
    NACP::NACP(const std::shared_ptr<vfs::Backing> &backing) {
        backing->Read(&nacpContents);

        // TODO: Select based on language settings, complete struct, yada yada
        ApplicationTitle &languageEntry = nacpContents.titleEntries[0];
        applicationName = std::string(languageEntry.applicationName.data(), languageEntry.applicationName.size());
        applicationPublisher = std::string(languageEntry.applicationPublisher.data(), languageEntry.applicationPublisher.size());
    }
}