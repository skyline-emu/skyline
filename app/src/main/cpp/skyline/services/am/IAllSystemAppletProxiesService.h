// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/base_service.h>
#include <services/serviceman.h>

namespace skyline::service::am {
    /**
     * @brief IAllSystemAppletProxiesService is used to open proxies (https://switchbrew.org/wiki/Applet_Manager_services#appletAE)
     */
    class IAllSystemAppletProxiesService : public BaseService {
      public:
        IAllSystemAppletProxiesService(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This returns #ILibraryAppletProxy (https://switchbrew.org/wiki/Applet_Manager_services#OpenLibraryAppletProxy)
         */
        Result OpenLibraryAppletProxy(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This returns #IApplicationProxy (https://switchbrew.org/wiki/Applet_Manager_services#OpenApplicationProxy)
         */
        Result OpenApplicationProxy(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This returns #IOverlayAppletProxy (https://switchbrew.org/wiki/Applet_Manager_services#OpenOverlayAppletProxy)
         */
        Result OpenOverlayAppletProxy(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This returns #ISystemAppletProxy (https://switchbrew.org/wiki/Applet_Manager_services#OpenSystemAppletProxy)
         */
        Result OpenSystemAppletProxy(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
