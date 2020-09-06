// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <kernel/types/KEvent.h>
#include <services/base_service.h>
#include <services/serviceman.h>

namespace skyline::service::nifm {
    /**
     * @brief IRequest is used by applications to bring up a network (https://switchbrew.org/wiki/Network_Interface_services#IRequest)
     */
    class IRequest : public BaseService {
      private:
        std::shared_ptr<type::KEvent> event0; //!< The KEvent that is signalled on request state changes
        std::shared_ptr<type::KEvent> event1; //!< The KEvent that is signalled on request changes

      public:
        IRequest(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This returns the current state of the request (https://switchbrew.org/wiki/Network_Interface_services#GetRequestState)
         */
        Result GetRequestState(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This returns the error code if a network bring up request fails (https://switchbrew.org/wiki/Network_Interface_services#GetResult)
         */
        Result GetResult(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This returns two KEvent handles that signal request on request updates (https://switchbrew.org/wiki/Network_Interface_services#GetSystemEventReadableHandles)
         */
        Result GetSystemEventReadableHandles(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This submits a request to bring up a network (https://switchbrew.org/wiki/Network_Interface_services#Submit)
         */
        Result Submit(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
