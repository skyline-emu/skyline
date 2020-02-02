#pragma once

#include <kernel/types/KSharedMemory.h>
#include <common.h>
#include "input/common.h"

namespace skyline::input {
    /**
     * @brief The Input class manages input devices
     */
    class Input {
      private:
        const DeviceState &state; //!< The state of the device

      public:
        Input(const DeviceState &state);

        std::shared_ptr<npad::CommonNpad> commonNpad; //!< The common npad device
        std::array<std::shared_ptr<npad::NpadDevice>, npad::constant::NpadCount> npad; //!< Array of npad devices

        std::shared_ptr<kernel::type::KSharedMemory> hidKMem; //!< The shared memory reserved for HID input
        HidSharedMemory *hidMem; //!< A pointer to the root of HID shared memory
    };
}
