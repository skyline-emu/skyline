// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "vfs/os_backing.h"
#include "loader/nro.h"
#include "loader/nso.h"
#include "loader/nca.h"
#include "loader/nsp.h"
#include "nce/guest.h"
#include "os.h"

namespace skyline::kernel {
    OS::OS(std::shared_ptr<JvmManager> &jvmManager, std::shared_ptr<Logger> &logger, std::shared_ptr<Settings> &settings) : state(this, process, jvmManager, settings, logger), memory(state), serviceManager(state) {}

    void OS::Execute(int romFd, loader::RomFormat romType) {
        auto romFile = std::make_shared<vfs::OsBacking>(romFd);

        if (romType == loader::RomFormat::NRO) {
            state.loader = std::make_shared<loader::NroLoader>(romFile);
        } else if (romType == loader::RomFormat::NSO) {
            state.loader = std::make_shared<loader::NsoLoader>(romFile);
        } else if (romType == loader::RomFormat::NCA) {
            state.loader = std::make_shared<loader::NcaLoader>(romFile);
        } else if (romType == loader::RomFormat::NSP) {
            state.loader = std::make_shared<loader::NspLoader>(romFile);
        } else {
            throw exception("Unsupported ROM extension.");
        }

        auto process = CreateProcess(constant::BaseAddress, 0, constant::DefStackSize);
        state.loader->LoadProcessData(process, state);
        process->InitializeMemory();
        process->threads.at(process->pid)->Start(); // The kernel itself is responsible for starting the main thread

        state.nce->Execute();
    }

    std::shared_ptr<type::KProcess> OS::CreateProcess(u64 entry, u64 argument, size_t stackSize) {
        auto stack = std::make_shared<type::KSharedMemory>(state, 0, stackSize, memory::Permission{true, true, false}, memory::states::Reserved, MAP_NORESERVE | MAP_STACK);
        stack->guest = stack->kernel;
        if (mprotect(reinterpret_cast<void *>(stack->guest.address), PAGE_SIZE, PROT_NONE))
            throw exception("Failed to create guard pages");

        auto tlsMem = std::make_shared<type::KSharedMemory>(state, 0, (sizeof(ThreadContext) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1), memory::Permission{true, true, false}, memory::states::Reserved);
        tlsMem->guest = tlsMem->kernel;

        auto pid = clone(reinterpret_cast<int (*)(void *)>(&guest::GuestEntry), reinterpret_cast<void *>(stack->guest.address + stackSize), CLONE_FILES | CLONE_FS | CLONE_SETTLS | SIGCHLD, reinterpret_cast<void *>(entry), nullptr, reinterpret_cast<void *>(tlsMem->guest.address));
        if (pid == -1)
            throw exception("Call to clone() has failed: {}", strerror(errno));

        state.logger->Debug("Successfully created process with PID: {}", pid);
        process = std::make_shared<kernel::type::KProcess>(state, pid, argument, stack, tlsMem);

        return process;
    }

    void OS::KillThread(pid_t pid) {
        if (process->pid == pid) {
            state.logger->Debug("Killing process with PID: {}", pid);
            for (auto &thread: process->threads)
                thread.second->Kill();
        } else {
            state.logger->Debug("Killing thread with TID: {}", pid);
            process->threads.at(pid)->Kill();
        }
    }
}
