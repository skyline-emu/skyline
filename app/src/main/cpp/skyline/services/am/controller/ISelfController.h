// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/base_service.h>
#include <services/serviceman.h>

namespace skyline::service::am {
    /**
     * @brief This has functions relating to an application's own current status (https://switchbrew.org/wiki/Applet_Manager_services#ISelfController)
     */
    class ISelfController : public BaseService {
      private:
        std::shared_ptr<kernel::type::KEvent> libraryAppletLaunchableEvent; //!< This KEvent is triggered when the library applet is launchable
        std::shared_ptr<kernel::type::KEvent> accumulatedSuspendedTickChangedEvent; //!< This KEvent is triggered when the time the system has spent in suspend is updated

      public:
        ISelfController(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This function prevents the running application from being quit via the home button (https://switchbrew.org/wiki/Applet_Manager_services#LockExit)
         */
        Result LockExit(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function allows the running application to be quit via the home button (https://switchbrew.org/wiki/Applet_Manager_services#UnlockExit)
         */
        Result UnlockExit(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function obtains a handle to the library applet launchable event (https://switchbrew.org/wiki/Applet_Manager_services#GetLibraryAppletLaunchableEvent)
         */
        Result GetLibraryAppletLaunchableEvent(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function takes a u8 bool flag and no output (Stubbed) (https://switchbrew.org/wiki/Applet_Manager_services#SetOperationModeChangedNotification)
         */
        Result SetOperationModeChangedNotification(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function takes a u8 bool flag and no output (Stubbed) (https://switchbrew.org/wiki/Applet_Manager_services#SetPerformanceModeChangedNotification)
         */
        Result SetPerformanceModeChangedNotification(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function takes 3 unknown u8 values and has no output (Stubbed) (https://switchbrew.org/wiki/Applet_Manager_services#GetCurrentFocusState)
         */
        Result SetFocusHandlingMode(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function toggles whether a restart message should be sent (https://switchbrew.org/wiki/Applet_Manager_services#SetRestartMessageEnabled)
         */
        Result SetRestartMessageEnabled(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function takes a u8 bool flag and has no output (Stubbed) (https://switchbrew.org/wiki/Applet_Manager_services#SetOutOfFocusSuspendingEnabled)
         */
        Result SetOutOfFocusSuspendingEnabled(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This function returns an output u64 LayerId (https://switchbrew.org/wiki/Applet_Manager_services#CreateManagedDisplayLayer)
         */
        Result CreateManagedDisplayLayer(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This obtains a handle to the system sleep time change KEvent  (https://switchbrew.org/wiki/Applet_Manager_services#GetAccumulatedSuspendedTickChangedEvent)
         */
        Result GetAccumulatedSuspendedTickChangedEvent(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
