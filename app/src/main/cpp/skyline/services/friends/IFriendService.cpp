// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "IFriendService.h"

namespace skyline::service::friends {
    IFriendService::IFriendService(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, Service::friends_IFriendService, "friends:IFriendService", {
    }) {}
}
