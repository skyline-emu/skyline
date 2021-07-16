// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <common/settings.h>
#include "IRandom.h"

namespace skyline::service::csrng {
    IRandom::IRandom(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {
        gen=std::mt19937_64(rd());
    }

    Result IRandom::GetRandomBytes(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        u32 length=request.outputBuf.at(0).size();
        for(auto i:request.outputBuf.at(0)) {
            i=dist(gen);
        }
        return {};
    }
}
