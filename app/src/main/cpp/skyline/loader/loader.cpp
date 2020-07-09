// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <nce.h>
#include <os.h>
#include <kernel/memory.h>
#include "loader.h"

namespace skyline::loader {
    Loader::ExecutableLoadInfo Loader::LoadExecutable(const std::shared_ptr<kernel::type::KProcess> process, const DeviceState &state, Executable &executable, size_t offset) {
        u64 base = constant::BaseAddress + offset;

        u64 textSize = executable.text.contents.size();
        u64 roSize = executable.ro.contents.size();
        u64 dataSize = executable.data.contents.size() + executable.bssSize;

        if (!util::IsAligned(textSize, PAGE_SIZE) || !util::IsAligned(roSize, PAGE_SIZE) || !util::IsAligned(dataSize, PAGE_SIZE))
            throw exception("LoadProcessData: Sections are not aligned with page size: 0x{:X}, 0x{:X}, 0x{:X}", textSize, roSize, dataSize);

        if (!util::IsAligned(executable.text.offset, PAGE_SIZE) || !util::IsAligned(executable.ro.offset, PAGE_SIZE) || !util::IsAligned(executable.data.offset, PAGE_SIZE))
            throw exception("LoadProcessData: Section offsets are not aligned with page size: 0x{:X}, 0x{:X}, 0x{:X}", executable.text.offset, executable.ro.offset, executable.data.offset);

        // The data section will always be the last section in memory, so put the patch section after it
        u64 patchOffset = executable.data.offset + dataSize;
        std::vector<u32> patch = state.nce->PatchCode(executable.text.contents, base, patchOffset);

        u64 patchSize = patch.size() * sizeof(u32);
        u64 padding = util::AlignUp(patchSize, PAGE_SIZE) - patchSize;

        process->NewHandle<kernel::type::KPrivateMemory>(base + executable.text.offset, textSize, memory::Permission{true, false, true}, memory::states::CodeStatic); // R-X
        state.logger->Debug("Successfully mapped section .text @ 0x{0:X}, Size = 0x{1:X}", base + executable.text.offset, textSize);

        process->NewHandle<kernel::type::KPrivateMemory>(base + executable.ro.offset, roSize, memory::Permission{true, false, false}, memory::states::CodeReadOnly); // R--
        state.logger->Debug("Successfully mapped section .rodata @ 0x{0:X}, Size = 0x{1:X}", base + executable.ro.offset, roSize);

        process->NewHandle<kernel::type::KPrivateMemory>(base + executable.data.offset, dataSize, memory::Permission{true, true, false}, memory::states::CodeMutable); // RW-
        state.logger->Debug("Successfully mapped section .data @ 0x{0:X}, Size = 0x{1:X}", base + executable.data.offset, dataSize);

        process->NewHandle<kernel::type::KPrivateMemory>(base + patchOffset, patchSize + padding, memory::Permission{true, true, true}, memory::states::CodeMutable); // RWX
        state.logger->Debug("Successfully mapped section .patch @ 0x{0:X}, Size = 0x{1:X}", base + patchOffset, patchSize + padding);

        process->WriteMemory(executable.text.contents.data(), base + executable.text.offset, textSize);
        process->WriteMemory(executable.ro.contents.data(), base + executable.ro.offset, roSize);
        process->WriteMemory(executable.data.contents.data(), base + executable.data.offset, dataSize - executable.bssSize);
        process->WriteMemory(patch.data(), base + patchOffset, patchSize);

        return {base, patchOffset + patchSize + padding};
    }
}
