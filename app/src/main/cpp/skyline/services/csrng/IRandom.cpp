// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <common/settings.h>
#include "IRandom.h"

namespace skyline::service::csrng {
    IRandom::IRandom(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager){}

    Result IRandom::GetRandomBytes(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        u32 length=request.outputBuf.size();
        buffer.resize(length);
        for(u32 i=0;i<length;i++) {
            buffer[i]=engine();
        }
        response.Push(buffer);
        return {};
    }
}
