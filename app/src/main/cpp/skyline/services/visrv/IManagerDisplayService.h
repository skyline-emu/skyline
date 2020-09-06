// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "IDisplayService.h"

namespace skyline::service::visrv {
    /**
     * @brief This service retrieves information about a display in context of the entire system (https://switchbrew.org/wiki/Display_services#IManagerDisplayService)
     */
    class IManagerDisplayService : public IDisplayService {
      public:
        IManagerDisplayService(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief Creates a managed layer on a specific display
         */
        Result CreateManagedLayer(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Destroys a managed layer created on a specific display
         */
        Result DestroyManagedLayer(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This takes a layer's ID and adds it to the layer stack
         */
        Result AddToLayerStack(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
