// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <kernel/types/KProcess.h>
#include "INotificationService.h"

namespace skyline::service::friends {
    INotificationService::INotificationService(const DeviceState &state, ServiceManager &manager) : notificationEvent(std::make_shared<type::KEvent>(state)), BaseService(state, manager, Service::friends_INotificationService, "friends:INotificationService", {
        {0x0, SFUNC(INotificationService::GetEvent)},
    }) {}

    void INotificationService::GetEvent(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        KHandle handle = state.process->InsertItem(notificationEvent);
        state.logger->Debug("Friend Notification Event Handle: 0x{:X}", handle);

        response.copyHandles.push_back(handle);
    }
}
