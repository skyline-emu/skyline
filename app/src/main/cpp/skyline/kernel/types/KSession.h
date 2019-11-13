#pragma once

#include <common.h>
#include <services/base_service.h>
#include "KSyncObject.h"

namespace skyline::kernel::type {
    /**
     * @brief KService holds a reference to a service, this is equivalent to KClientSession
     */
    class KSession : public KSyncObject {
      public:
        const std::shared_ptr<service::BaseService> serviceObject; //!< A shared pointer to the service class
        std::unordered_map<handle_t, std::shared_ptr<service::BaseService>> domainTable; //!< This maps from a virtual handle to it's service
        handle_t handleIndex = constant::BaseVirtualHandleIndex;
        const service::Service serviceType; //!< The type of the service
        enum class ServiceStatus { Open, Closed } serviceStatus{ServiceStatus::Open}; //!< If the session is open or closed
        bool isDomain{}; //!< Holds if this is a domain session or not

        /**
         * @param state The state of the device
         * @param serviceObject A shared pointer to the service class
         */
        KSession(const DeviceState &state, std::shared_ptr<service::BaseService> &serviceObject) : serviceObject(serviceObject), serviceType(serviceObject->serviceType), KSyncObject(state, KType::KSession) {}

        /**
         * This converts this session into a domain session (https://switchbrew.org/wiki/IPC_Marshalling#Domains)
         * @return The virtual handle of this service in the domain
         */
        handle_t ConvertDomain() {
            isDomain = true;
            domainTable[handleIndex] = serviceObject;
            return handleIndex++;
        }
    };
}
