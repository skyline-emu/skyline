// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <kernel/types/KProcess.h>
#include "ISettingsServer.h"

namespace skyline::service::settings {
    ISettingsServer::ISettingsServer(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, {
        {0x1, SFUNC(ISettingsServer::GetAvailableLanguageCodes)},
        {0x2, SFUNC(ISettingsServer::MakeLanguageCode)},
        {0x5, SFUNC(ISettingsServer::GetAvailableLanguageCodes2)}
    }) {}

    constexpr std::array<u64, constant::NewLanguageCodeListSize> LanguageCodeList = {
        util::MakeMagic<u64>("ja"),
        util::MakeMagic<u64>("en-US"),
        util::MakeMagic<u64>("fr"),
        util::MakeMagic<u64>("de"),
        util::MakeMagic<u64>("it"),
        util::MakeMagic<u64>("es"),
        util::MakeMagic<u64>("zh-CN"),
        util::MakeMagic<u64>("ko"),
        util::MakeMagic<u64>("nl"),
        util::MakeMagic<u64>("pt"),
        util::MakeMagic<u64>("ru"),
        util::MakeMagic<u64>("zh-TW"),
        util::MakeMagic<u64>("en-GB"),
        util::MakeMagic<u64>("fr-CA"),
        util::MakeMagic<u64>("es-419"),
        util::MakeMagic<u64>("zh-Hans"),
        util::MakeMagic<u64>("zh-Hant"),
    };

    void ISettingsServer::GetAvailableLanguageCodes(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.process->WriteMemory(LanguageCodeList.data(), request.outputBuf.at(0).address, constant::OldLanguageCodeListSize * sizeof(u64));

        response.Push<i32>(constant::OldLanguageCodeListSize);
    }

    void ISettingsServer::MakeLanguageCode(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u64>(LanguageCodeList.at(request.Pop<i32>()));
    }

    void ISettingsServer::GetAvailableLanguageCodes2(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.process->WriteMemory(LanguageCodeList.data(), request.outputBuf.at(0).address, constant::NewLanguageCodeListSize * sizeof(u64));

        response.Push<i32>(constant::NewLanguageCodeListSize);
    }
}
