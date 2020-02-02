#pragma once

#include <services/base_service.h>
#include <services/serviceman.h>
#include <kernel/types/KProcess.h>
#include <input.h>

namespace skyline::constant {
    constexpr size_t hidSharedMemSize = 0x40000; //!< The size of HID Shared Memory (https://switchbrew.org/wiki/HID_Shared_Memory)
}

namespace skyline::service::hid {
    /**
     * @brief IAppletResource is used to get the handle to the HID shared memory (https://switchbrew.org/wiki/HID_services#IAppletResource)
     */
    class IAppletResource : public BaseService {
      public:
        IAppletResource(const DeviceState &state, ServiceManager &manager);

        std::shared_ptr<type::KSharedMemory> hidSharedMemory;

        /**
         * @brief This opens a handle to HID shared memory (https://switchbrew.org/wiki/HID_services#GetSharedMemoryHandle)
         */
        void GetSharedMemoryHandle(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };

    /**
     * @brief hid or Human Interface Device service is used to access input devices (https://switchbrew.org/wiki/HID_services#hid)
     */
    class hid : public BaseService {
      private:
        std::shared_ptr<IAppletResource> resource{}; //!< A shared pointer to the applet resource

      public:
        hid(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This returns an IAppletResource (https://switchbrew.org/wiki/HID_services#CreateAppletResource)
         */
        void CreateAppletResource(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This sets the style of controllers supported (https://switchbrew.org/wiki/HID_services#SetSupportedNpadStyleSet)
         */
        void SetSupportedNpadStyleSet(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This sets the NpadIds which are supported (https://switchbrew.org/wiki/HID_services#SetSupportedNpadIdType)
         */
        void SetSupportedNpadIdType(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This requests the activation of a controller. This is stubbed as we don't have to activate anything.
         */
        void ActivateNpad(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Sets the Joy-Con hold mode (https://switchbrew.org/wiki/HID_services#SetNpadJoyHoldType)
         */
        void SetNpadJoyHoldType(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Sets the Joy-Con assignment mode to Single by default (https://switchbrew.org/wiki/HID_services#SetNpadJoyAssignmentModeSingleByDefault)
         */
        void SetNpadJoyAssignmentModeSingleByDefault(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Sets the Joy-Con assignment mode to Single (https://switchbrew.org/wiki/HID_services#SetNpadJoyAssignmentModeSingle)
         */
        void SetNpadJoyAssignmentModeSingle(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief Sets the Joy-Con assignment mode to Dual (https://switchbrew.org/wiki/HID_services#SetNpadJoyAssignmentModeDual)
         */
        void SetNpadJoyAssignmentModeDual(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
