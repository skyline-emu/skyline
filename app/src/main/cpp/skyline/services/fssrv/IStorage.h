// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/base_service.h>
#include <services/serviceman.h>
#include <vfs/backing.h>

namespace skyline::service::fssrv {
    /**
     * @brief IStorage is an interface to a raw backing device (https://switchbrew.org/wiki/Filesystem_services#IStorage)
     */
    class IStorage : public BaseService {
      private:
        std::shared_ptr<vfs::Backing> backing; //!< The backing of the IStorage

      public:
        IStorage(std::shared_ptr<vfs::Backing> &backing, const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This reads a buffer from a region of an IStorage (https://switchbrew.org/wiki/Filesystem_services#Read)
         */
        Result Read(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This obtains the size of an IStorage (https://switchbrew.org/wiki/Filesystem_services#GetSize)
         */
        Result GetSize(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
