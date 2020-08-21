// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "common.h"
#include "kernel/types/KSharedMemory.h"
#include "input/shared_mem.h"
#include "input/npad.h"

namespace skyline::input {
    /**
     * @brief The Input class manages translating host input to guest input
     */
    class Input {
      private:
        const DeviceState &state;

      public:
        std::shared_ptr<kernel::type::KSharedMemory> kHid; //!< The kernel shared memory object for HID Shared Memory
        HidSharedMemory *hid; //!< A pointer to HID Shared Memory on the host

        NpadManager npad; //!< This manages all the NPad controllers

        Input(const DeviceState &state);
    };
}
