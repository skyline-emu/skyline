// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <common/settings.h>
#include "IRandom.h"

namespace skyline::service::csrng {
    IRandom::IRandom(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager){}

    Result IRandom::GetRandomBytes(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        u32 length=request.outputBuf.size();
        for(u32 i=1;i<=length;i++) {
            response.Push<u8>((u8)engine());
        }
        return {};
    }
}
