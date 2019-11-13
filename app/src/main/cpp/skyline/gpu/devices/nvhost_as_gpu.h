#pragma once

#include "nvdevice.h"

namespace skyline::gpu::device {
    /**
     * @brief NvHostAsGpu (/dev/nvhost-as-gpu) is used to access GPU virtual address spaces (https://switchbrew.org/wiki/NV_services#.2Fdev.2Fnvhost-as-gpu)
     */
    class NvHostAsGpu : public NvDevice {
      public:
        NvHostAsGpu(const DeviceState &state);
    };
}
