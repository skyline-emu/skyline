// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <kernel/types/KProcess.h>
#include "results.h"
#include "IDirectory.h"

namespace skyline::service::fssrv {
    struct __attribute__((packed)) DirectoryEntry {
        std::array<char, 0x301> name;

        struct {
            bool directory : 1;
            bool archive : 1;
            u8 _pad_ : 6;
        } attributes;

        u16 _pad0_;
        vfs::Directory::EntryType type;
        u8 _pad1_[3];
        u64 size;
    };

    IDirectory::IDirectory(std::shared_ptr<vfs::Directory> backing, std::shared_ptr<vfs::FileSystem> backingFs, const DeviceState &state, ServiceManager &manager) : backing(std::move(backing)), backingFs(std::move(backingFs)), BaseService(state, manager) {}

    Result IDirectory::Read(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto entries{backing->Read()};
        auto outputEntries{request.inputBuf.at(0).cast<DirectoryEntry>()};
        size_t i{};

        for (; i < std::min(entries.size(), outputEntries.size()); i++) {
            auto &entry{entries.at(i)};

            outputEntries[i] = {
                .type = entry.type,
                .attributes.directory = (entry.type == vfs::Directory::EntryType::Directory),
                .size = entry.size,
            };

            span(outputEntries[i].name).copy_from(entry.name);
        }

        response.Push<u64>(i);
        return {};
    }
}
