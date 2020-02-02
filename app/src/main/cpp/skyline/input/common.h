#pragma once

#include <common.h>
#include "sharedmem.h"
#include "npad.h"

namespace skyline::input {
    /**
     * @brief Encapsulates hid shared memory
     */
    struct HidSharedMemory {
        DebugPadSection debugPad; //!< The debug pad section
        TouchScreenSection touchScreen; //!< The touch screen section
        MouseSection mouse; //!< The mouse section
        KeyboardSection keyboard; //!< The keyboard section
        std::array<BasicXpadSection, 4> xpad; //!< The xpads section
        HomeButtonSection homeButton; //!< The home button section
        SleepButtonSection sleepButton; //!< The sleep button section
        CaptureButtonSection captureButton; //!< The capture button section
        std::array<InputDetectorSection, 16> inputDetector; //!< The input detectors section
        u64 _pad0_[0x80 * 0x10]; //!< Unique pad (<5.0.0)
        std::array<npad::NpadSection, npad::constant::NpadCount> npad; //!< The npads section
        GestureSection gesture; //!< The gesture section
        ConsoleSixAxisSensorSection consoleSixAxisSensor; //!< The gyro/accel section
        u64 _pad1_[0x7BC];
    };
    static_assert(sizeof(HidSharedMemory) == 0x40000);
}
