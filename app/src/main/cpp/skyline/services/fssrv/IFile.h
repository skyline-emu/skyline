// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/serviceman.h>
#include <vfs/backing.h>

namespace skyline::service::fssrv {
    /**
     * @brief IFile is an interface for accessing files
     * @url https://switchbrew.org/wiki/Filesystem_services#IFile
     */
    class IFile : public BaseService {
      private:
        std::shared_ptr<vfs::Backing> backing;

      public:
        IFile(std::shared_ptr<vfs::Backing> backing, const DeviceState &state, ServiceManager &manager);

        /**
         * @brief Reads a buffer from a region of an IFile
         */
        Result Read(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Writes a buffer to a region of an IFile
         */
        Result Write(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Flushes any written data to the IFile on the Switch, however the emulator processes any FS event immediately so this does nothing
         */
        Result Flush(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Sets the size of an IFile
         */
        Result SetSize(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Obtains the size of an IFile
         */
        Result GetSize(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        SERVICE_DECL(
            SFUNC(0x0, IFile, Read),
            SFUNC(0x1, IFile, Write),
            SFUNC(0x2, IFile, Flush),
            SFUNC(0x3, IFile, SetSize),
            SFUNC(0x4, IFile, GetSize)
        )
    };
}
