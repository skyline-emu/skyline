// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/base_service.h>
#include <services/serviceman.h>

namespace skyline {
    namespace constant {
        constexpr u32 FontSharedMemSize = 0x1100000; //!< This is the total size of the font shared memory
    }

    namespace service::pl {
        /**
         * @brief IPlatformServiceManager is used to access shared fonts (https://switchbrew.org/wiki/Shared_Database_services#pl:u.2C_pl:s)
         */
        class IPlatformServiceManager : public BaseService {
          private:
            std::shared_ptr<kernel::type::KSharedMemory> fontSharedMem; //!< This shared memory stores the TTF data of all shared fonts

          public:
            IPlatformServiceManager(const DeviceState &state, ServiceManager &manager);

            /**
             * @brief This returns the loading state of the requested font (https://switchbrew.org/wiki/Shared_Database_services#GetLoadState)
             */
            Result GetLoadState(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief This returns the size of the requested font (https://switchbrew.org/wiki/Shared_Database_services#GetSize)
             */
            Result GetSize(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief This returns the offset in shared memory of the requested font (https://switchbrew.org/wiki/Shared_Database_services#GetSharedMemoryAddressOffset)
             */
            Result GetSharedMemoryAddressOffset(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief This returns a handle to the whole font shared memory (https://switchbrew.org/wiki/Shared_Database_services#GetSharedMemoryNativeHandle)
             */
            Result GetSharedMemoryNativeHandle(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
        };
    }
}
