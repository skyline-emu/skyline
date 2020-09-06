// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/base_service.h>
#include <services/serviceman.h>

namespace skyline::service::pctl {
    /**
     * @brief IParentalControlServiceFactory is used to open a parental controls instance (https://switchbrew.org/wiki/Parental_Control_services#pctl:s.2C_pctl:r.2C_pctl:a.2C_pctl)
     */
    class IParentalControlServiceFactory : public BaseService {
      public:
        IParentalControlServiceFactory(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This creates and initializes an IParentalControlService instance that can be used to read parental control configuration
         */
        Result CreateService(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
