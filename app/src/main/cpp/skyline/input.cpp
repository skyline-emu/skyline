// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "input.h"

namespace skyline::input {
    Input::Input(const DeviceState &state) : state(state), kHid(std::make_shared<kernel::type::KSharedMemory>(state, NULL, sizeof(HidSharedMemory), memory::Permission(true, false, false))), hid(reinterpret_cast<HidSharedMemory *>(kHid->kernel.address)), npad(state, hid) {}
}
