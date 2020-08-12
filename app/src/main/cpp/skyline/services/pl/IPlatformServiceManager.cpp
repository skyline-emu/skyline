// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <kernel/types/KProcess.h>
#include "resources/FontChineseSimplified.ttf.h"
#include "resources/FontChineseTraditional.ttf.h"
#include "resources/FontExtendedChineseSimplified.ttf.h"
#include "resources/FontKorean.ttf.h"
#include "resources/FontNintendoExtended.ttf.h"
#include "resources/FontStandard.ttf.h"
#include "IPlatformServiceManager.h"

namespace skyline::service::pl {
    /**
     * @brief This holds an entry in the the font table
     */
    struct FontEntry {
        u8 *data; //!< The font TTF data
        size_t length; //!< The length of the font TTF data
        size_t offset; //!< The offset of the font in shared memory
    };

    std::array<FontEntry, 6> fontTable = {{
                                              {FontChineseSimplified, FontExtendedChineseSimplifiedLength},
                                              {FontChineseTraditional, FontChineseTraditionalLength},
                                              {FontExtendedChineseSimplified, FontExtendedChineseSimplifiedLength},
                                              {FontKorean, FontKoreanLength},
                                              {FontNintendoExtended, FontNintendoExtendedLength},
                                              {FontStandard, FontStandardLength}
                                          }};

    IPlatformServiceManager::IPlatformServiceManager(const DeviceState &state, ServiceManager &manager) : fontSharedMem(std::make_shared<kernel::type::KSharedMemory>(state, NULL, constant::FontSharedMemSize, memory::Permission{true, false, false})), BaseService(state, manager, Service::pl_IPlatformServiceManager, "pl:IPlatformServiceManager", {
        {0x1, SFUNC(IPlatformServiceManager::GetLoadState)},
        {0x2, SFUNC(IPlatformServiceManager::GetSize)},
        {0x3, SFUNC(IPlatformServiceManager::GetSharedMemoryAddressOffset)},
        {0x4, SFUNC(IPlatformServiceManager::GetSharedMemoryNativeHandle)}
    }) {
        constexpr u32 SharedFontResult = 0x7F9A0218; //!< This is the decrypted magic for a single font in the shared font data
        constexpr u32 SharedFontMagic = 0x36F81A1E; //!< This is the encrypted magic for a single font in the shared font data
        constexpr u32 SharedFontKey = SharedFontMagic ^SharedFontResult; //!< This is the XOR key for encrypting the font size

        auto pointer = reinterpret_cast<u32 *>(fontSharedMem->kernel.address);
        for (auto &font : fontTable) {
            *pointer++ = SharedFontResult;
            *pointer++ = font.length ^ SharedFontKey;
            font.offset = reinterpret_cast<u64>(pointer) - fontSharedMem->kernel.address;

            memcpy(pointer, font.data, font.length);
            pointer = reinterpret_cast<u32 *>(reinterpret_cast<u64>(pointer) + font.length);
        }
    }

    void IPlatformServiceManager::GetLoadState(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        constexpr u32 FontLoaded = 1; //!< This is returned to show that all fonts have been loaded into memory

        response.Push(FontLoaded);
    }

    void IPlatformServiceManager::GetSize(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto fontId = request.Pop<u32>();

        response.Push<u32>(fontTable.at(fontId).length);
    }

    void IPlatformServiceManager::GetSharedMemoryAddressOffset(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto fontId = request.Pop<u32>();

        response.Push<u32>(fontTable.at(fontId).offset);
    }

    void IPlatformServiceManager::GetSharedMemoryNativeHandle(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto handle = state.process->InsertItem<type::KSharedMemory>(fontSharedMem);
        response.copyHandles.push_back(handle);
    }
}
