// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <kernel/types/KSession.h>
#include <nce.h>
#include "base_service.h"

namespace skyline::service {
    /**
     * @brief The ServiceManager class manages passing IPC requests to the right Service and running event loops of Services
     * @todo This implementation varies significantly from HOS, this should be rectified much later on
     */
    class ServiceManager {
      private:
        const DeviceState &state; //!< The state of the device
        std::unordered_map<ServiceName, std::shared_ptr<BaseService>> serviceMap; //!< A mapping from a Service to the underlying object
        Mutex mutex; //!< This mutex is used to ensure concurrent access to services doesn't cause crashes

        /**
         * @brief Creates an instance of the service if it doesn't already exist, otherwise returns an existing instance
         * @param name The name of the service to create
         * @return A shared pointer to an instance of the service
         */
        std::shared_ptr<BaseService> CreateService(ServiceName name);

      public:
        std::shared_ptr<BaseService> smUserInterface; //!< This is used by applications to open connections to services

        /**
         * @param state The state of the device
         */
        ServiceManager(const DeviceState &state);

        /**
         * @brief Creates a new service using it's type enum and writes it's handle or virtual handle (If it's a domain request) to IpcResponse
         * @param name The service's name
         * @param session The session object of the command
         * @param response The response object to write the handle or virtual handle to
         */
        std::shared_ptr<BaseService> NewService(ServiceName name, type::KSession &session, ipc::IpcResponse &response);

        /**
         * @brief Registers a service object in the manager and writes it's handle or virtual handle (If it's a domain request) to IpcResponse
         * @param serviceObject An instance of the service
         * @param session The session object of the command
         * @param response The response object to write the handle or virtual handle to
         * @param submodule If the registered service is a submodule or not
         * @param name The name of the service to register if it's not a submodule - it will be added to the service map
         */
        void RegisterService(std::shared_ptr<BaseService> serviceObject, type::KSession &session, ipc::IpcResponse &response, bool submodule = true, ServiceName name = {});

        /**
         * @param serviceType The type of the service
         * @tparam The class of the service
         * @return A shared pointer to an instance of the service
         * @note This only works for services created with `NewService` as sub-interfaces used with `RegisterService` can have multiple instances
         */
        template<typename Type>
        std::shared_ptr<Type> GetService(ServiceName name) {
            return std::static_pointer_cast<Type>(serviceMap.at(name));
        }

        template<typename Type>
        constexpr std::shared_ptr<Type> GetService(std::string_view name) {
            return GetService<Type>(util::MakeMagic<ServiceName>(name));
        }

        /**
         * @brief Closes an existing session to a service
         * @param service The handle of the KService object
         */
        void CloseSession(KHandle handle);

        /**
         * @brief Handles a Synchronous IPC Request
         * @param handle The handle of the object
         */
        void SyncRequestHandler(KHandle handle);
    };
}
