// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <kernel/types/KEvent.h>
#include <random>
#include <services/serviceman.h>

namespace skyline::service::csrng {
    /**
     * @brief IRandom or "Random Number Generator API" is a randomize module
     * @url https://github.com/Ryujinx/Ryujinx/blob/00ce9eea620652b97b4d3e8cd9218c6fccff8b1c/Ryujinx.HLE/HOS/Services/Spl/IRandomInterface.cs
     */
    class IRandom : public BaseService {
      public:
        IRandom(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief Initializes an random number generator?
         */
        Result GetRandomBytes(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

      SERVICE_DECL(
          SFUNC(0x0, IRandom, GetRandomBytes)
      )

      private:
        std::random_device engine;
        u32 length;
        std::string buffer;
    };
}
