// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <os.h>
#include "results.h"
#include "svc.h"

namespace skyline::kernel::svc {
    void SetHeapSize(DeviceState &state) {
        auto size = state.ctx->registers.w1;

        if (!util::IsAligned(size, 0x200000)) {
            state.ctx->registers.w0 = result::InvalidSize;
            state.ctx->registers.x1 = 0;

            state.logger->Warn("svcSetHeapSize: 'size' not divisible by 2MB: {}", size);
            return;
        }

        auto &heap = state.process->heap;
        heap->Resize(size);

        state.ctx->registers.w0 = Result{};
        state.ctx->registers.x1 = heap->address;

        state.logger->Debug("svcSetHeapSize: Allocated at 0x{:X} for 0x{:X} bytes", heap->address, heap->size);
    }

    void SetMemoryAttribute(DeviceState &state) {
        auto address = state.ctx->registers.x0;
        if (!util::PageAligned(address)) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcSetMemoryAttribute: 'address' not page aligned: 0x{:X}", address);
            return;
        }

        auto size = state.ctx->registers.x1;
        if (!util::PageAligned(size)) {
            state.ctx->registers.w0 = result::InvalidSize;
            state.logger->Warn("svcSetMemoryAttribute: 'size' {}: 0x{:X}", size ? "not page aligned" : "is zero", size);
            return;
        }

        memory::MemoryAttribute mask{.value = state.ctx->registers.w2};
        memory::MemoryAttribute value{.value = state.ctx->registers.w3};

        auto maskedValue = mask.value | value.value;
        if (maskedValue != mask.value || !mask.isUncached || mask.isDeviceShared || mask.isBorrowed || mask.isIpcLocked) {
            state.ctx->registers.w0 = result::InvalidCombination;
            state.logger->Warn("svcSetMemoryAttribute: 'mask' invalid: 0x{:X}, 0x{:X}", mask.value, value.value);
            return;
        }

        auto chunk = state.os->memory.GetChunk(address);
        auto block = state.os->memory.GetBlock(address);
        if (!chunk || !block) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcSetMemoryAttribute: Cannot find memory region: 0x{:X}", address);
            return;
        }

        if (!chunk->state.attributeChangeAllowed) {
            state.ctx->registers.w0 = result::InvalidState;
            state.logger->Warn("svcSetMemoryAttribute: Attribute change not allowed for chunk: 0x{:X}", address);
            return;
        }

        block->attributes.isUncached = value.isUncached;
        MemoryManager::InsertBlock(chunk, *block);

        state.logger->Debug("svcSetMemoryAttribute: Set caching to {} at 0x{:X} for 0x{:X} bytes", !block->attributes.isUncached, address, size);
        state.ctx->registers.w0 = Result{};
    }

    void MapMemory(DeviceState &state) {
        auto destination = state.ctx->registers.x0;
        auto source = state.ctx->registers.x1;
        auto size = state.ctx->registers.x2;

        if (!util::PageAligned(destination) || !util::PageAligned(source)) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcMapMemory: Addresses not page aligned: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes)", source, destination, size);
            return;
        }

        if (!util::PageAligned(size)) {
            state.ctx->registers.w0 = result::InvalidSize;
            state.logger->Warn("svcMapMemory: 'size' {}: 0x{:X}", size ? "not page aligned" : "is zero", size);
            return;
        }

        auto stack = state.os->memory.stack;
        if (!stack.IsInside(destination)) {
            state.ctx->registers.w0 = result::InvalidMemoryRegion;
            state.logger->Warn("svcMapMemory: Destination not within stack region: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes)", source, destination, size);
            return;
        }

        auto descriptor = state.os->memory.Get(source);
        if (!descriptor) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcMapMemory: Source has no descriptor: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes)", source, destination, size);
            return;
        }
        if (!descriptor->chunk.state.mapAllowed) {
            state.ctx->registers.w0 = result::InvalidState;
            state.logger->Warn("svcMapMemory: Source doesn't allow usage of svcMapMemory: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes) 0x{:X}", source, destination, size, descriptor->chunk.state.value);
            return;
        }

        state.process->NewHandle<type::KPrivateMemory>(destination, size, descriptor->block.permission, memory::states::Stack);
        state.process->CopyMemory(source, destination, size);

        auto object = state.process->GetMemoryObject(source);
        if (!object)
            throw exception("svcMapMemory: Cannot find memory object in handle table for address 0x{:X}", source);

        object->item->UpdatePermission(source, size, {false, false, false});

        state.logger->Debug("svcMapMemory: Mapped range 0x{:X} - 0x{:X} to 0x{:X} - 0x{:X} (Size: 0x{:X} bytes)", source, source + size, destination, destination + size, size);
        state.ctx->registers.w0 = Result{};
    }

    void UnmapMemory(DeviceState &state) {
        auto source = state.ctx->registers.x0;
        auto destination = state.ctx->registers.x1;
        auto size = state.ctx->registers.x2;

        if (!util::PageAligned(destination) || !util::PageAligned(source)) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcUnmapMemory: Addresses not page aligned: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes)", source, destination, size);
            return;
        }

        if (!util::PageAligned(size)) {
            state.ctx->registers.w0 = result::InvalidSize;
            state.logger->Warn("svcUnmapMemory: 'size' {}: 0x{:X}", size ? "not page aligned" : "is zero", size);
            return;
        }

        auto stack = state.os->memory.stack;
        if (!stack.IsInside(source)) {
            state.ctx->registers.w0 = result::InvalidMemoryRegion;
            state.logger->Warn("svcUnmapMemory: Source not within stack region: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes)", source, destination, size);
            return;
        }

        auto sourceDesc = state.os->memory.Get(source);
        auto destDesc = state.os->memory.Get(destination);
        if (!sourceDesc || !destDesc) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcUnmapMemory: Addresses have no descriptor: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes)", source, destination, size);
            return;
        }

        if (!destDesc->chunk.state.mapAllowed) {
            state.ctx->registers.w0 = result::InvalidState;
            state.logger->Warn("svcUnmapMemory: Destination doesn't allow usage of svcMapMemory: Source: 0x{:X}, Destination: 0x{:X} (Size: 0x{:X} bytes) 0x{:X}", source, destination, size, destDesc->chunk.state.value);
            return;
        }

        auto destObject = state.process->GetMemoryObject(destination);
        if (!destObject)
            throw exception("svcUnmapMemory: Cannot find destination memory object in handle table for address 0x{:X}", destination);

        destObject->item->UpdatePermission(destination, size, sourceDesc->block.permission);

        state.process->CopyMemory(destination, source, size);

        auto sourceObject = state.process->GetMemoryObject(destination);
        if (!sourceObject)
            throw exception("svcUnmapMemory: Cannot find source memory object in handle table for address 0x{:X}", source);

        state.process->DeleteHandle(sourceObject->handle);

        state.logger->Debug("svcUnmapMemory: Unmapped range 0x{:X} - 0x{:X} to 0x{:X} - 0x{:X} (Size: 0x{:X} bytes)", source, source + size, destination, destination + size, size);
        state.ctx->registers.w0 = Result{};
    }

    void QueryMemory(DeviceState &state) {
        memory::MemoryInfo memInfo{};

        auto address = state.ctx->registers.x2;
        auto descriptor = state.os->memory.Get(address, false);

        if (descriptor) {
            memInfo = {
                .address = descriptor->block.address,
                .size = descriptor->block.size,
                .type = static_cast<u32>(descriptor->chunk.state.type),
                .attributes = descriptor->block.attributes.value,
                .permissions = static_cast<u32>(descriptor->block.permission.Get()),
                .deviceRefCount = 0,
                .ipcRefCount = 0,
            };

            state.logger->Debug("svcQueryMemory: Address: 0x{:X}, Size: 0x{:X}, Type: 0x{:X}, Is Uncached: {}, Permissions: {}{}{}", memInfo.address, memInfo.size, memInfo.type, static_cast<bool>(descriptor->block.attributes.isUncached), descriptor->block.permission.r ? "R" : "-", descriptor->block.permission.w ? "W" : "-", descriptor->block.permission.x ? "X" : "-");
        } else {
            auto addressSpaceEnd = state.os->memory.addressSpace.address + state.os->memory.addressSpace.size;

            memInfo = {
                .address = addressSpaceEnd,
                .size = ~addressSpaceEnd + 1,
                .type = static_cast<u32>(memory::MemoryType::Reserved),
            };

            state.logger->Debug("svcQueryMemory: Trying to query memory outside of the application's address space: 0x{:X}", address);
        }

        state.process->WriteMemory(memInfo, state.ctx->registers.x0);

        state.ctx->registers.w0 = Result{};
    }

    void ExitProcess(DeviceState &state) {
        state.logger->Debug("svcExitProcess: Exiting current process: {}", state.process->pid);
        state.os->KillThread(state.process->pid);
    }

    void CreateThread(DeviceState &state) {
        auto entryAddress = state.ctx->registers.x1;
        auto entryArgument = state.ctx->registers.x2;
        auto stackTop = state.ctx->registers.x3;
        auto priority = static_cast<i8>(state.ctx->registers.w4);

        if (!state.thread->switchPriority.Valid(priority)) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcCreateThread: 'priority' invalid: {}", priority);
            return;
        }

        auto thread = state.process->CreateThread(entryAddress, entryArgument, stackTop, priority);
        state.logger->Debug("svcCreateThread: Created thread with handle 0x{:X} (Entry Point: 0x{:X}, Argument: 0x{:X}, Stack Pointer: 0x{:X}, Priority: {}, TID: {})", thread->handle, entryAddress, entryArgument, stackTop, priority, thread->tid);

        state.ctx->registers.w1 = thread->handle;
        state.ctx->registers.w0 = Result{};
    }

    void StartThread(DeviceState &state) {
        auto handle = state.ctx->registers.w0;
        try {
            auto thread = state.process->GetHandle<type::KThread>(handle);
            state.logger->Debug("svcStartThread: Starting thread: 0x{:X}, PID: {}", handle, thread->tid);
            thread->Start();
            state.ctx->registers.w0 = Result{};
        } catch (const std::exception &) {
            state.logger->Warn("svcStartThread: 'handle' invalid: 0x{:X}", handle);
            state.ctx->registers.w0 = result::InvalidHandle;
        }
    }

    void ExitThread(DeviceState &state) {
        state.logger->Debug("svcExitThread: Exiting current thread: {}", state.thread->tid);
        state.os->KillThread(state.thread->tid);
    }

    void SleepThread(DeviceState &state) {
        auto in = state.ctx->registers.x0;

        switch (in) {
            case 0:
            case 1:
            case 2:
                state.logger->Debug("svcSleepThread: Yielding thread: {}", in);
                break;
            default:
                state.logger->Debug("svcSleepThread: Thread sleeping for {} ns", in);
                struct timespec spec = {
                    .tv_sec = static_cast<time_t>(state.ctx->registers.x0 / 1000000000),
                    .tv_nsec = static_cast<long>(state.ctx->registers.x0 % 1000000000)
                };
                nanosleep(&spec, nullptr);
        }
    }

    void GetThreadPriority(DeviceState &state) {
        auto handle = state.ctx->registers.w1;
        try {
            auto priority = state.process->GetHandle<type::KThread>(handle)->priority;
            state.logger->Debug("svcGetThreadPriority: Writing thread priority {}", priority);

            state.ctx->registers.w1 = priority;
            state.ctx->registers.w0 = Result{};
        } catch (const std::exception &) {
            state.logger->Warn("svcGetThreadPriority: 'handle' invalid: 0x{:X}", handle);
            state.ctx->registers.w0 = result::InvalidHandle;
        }
    }

    void SetThreadPriority(DeviceState &state) {
        auto handle = state.ctx->registers.w0;
        auto priority = state.ctx->registers.w1;

        try {
            state.logger->Debug("svcSetThreadPriority: Setting thread priority to {}", priority);
            state.process->GetHandle<type::KThread>(handle)->UpdatePriority(static_cast<u8>(priority));
            state.ctx->registers.w0 = Result{};
        } catch (const std::exception &) {
            state.logger->Warn("svcSetThreadPriority: 'handle' invalid: 0x{:X}", handle);
            state.ctx->registers.w0 = result::InvalidHandle;
        }
    }

    void ClearEvent(DeviceState &state) {
        auto object = state.process->GetHandle<type::KEvent>(state.ctx->registers.w0);
        object->signalled = false;
        state.ctx->registers.w0 = Result{};
    }

    void MapSharedMemory(DeviceState &state) {
        try {
            auto object = state.process->GetHandle<type::KSharedMemory>(state.ctx->registers.w0);
            auto address = state.ctx->registers.x1;

            if (!util::PageAligned(address)) {
                state.ctx->registers.w0 = result::InvalidAddress;
                state.logger->Warn("svcMapSharedMemory: 'address' not page aligned: 0x{:X}", address);
                return;
            }

            auto size = state.ctx->registers.x2;
            if (!util::PageAligned(size)) {
                state.ctx->registers.w0 = result::InvalidSize;
                state.logger->Warn("svcMapSharedMemory: 'size' {}: 0x{:X}", size ? "not page aligned" : "is zero", size);
                return;
            }

            memory::Permission permission = *reinterpret_cast<memory::Permission *>(&state.ctx->registers.w3);
            if ((permission.w && !permission.r) || (permission.x && !permission.r)) {
                state.logger->Warn("svcMapSharedMemory: 'permission' invalid: {}{}{}", permission.r ? "R" : "-", permission.w ? "W" : "-", permission.x ? "X" : "-");
                state.ctx->registers.w0 = result::InvalidNewMemoryPermission;
                return;
            }

            state.logger->Debug("svcMapSharedMemory: Mapping shared memory at 0x{:X} for {} bytes ({}{}{})", address, size, permission.r ? "R" : "-", permission.w ? "W" : "-", permission.x ? "X" : "-");

            object->Map(address, size, permission);

            state.ctx->registers.w0 = Result{};
        } catch (const std::exception &) {
            state.logger->Warn("svcMapSharedMemory: 'handle' invalid: 0x{:X}", state.ctx->registers.w0);
            state.ctx->registers.w0 = result::InvalidHandle;
        }
    }

    void CreateTransferMemory(DeviceState &state) {
        auto address = state.ctx->registers.x1;
        if (!util::PageAligned(address)) {
            state.ctx->registers.w0 = result::InvalidAddress;
            state.logger->Warn("svcCreateTransferMemory: 'address' not page aligned: 0x{:X}", address);
            return;
        }

        auto size = state.ctx->registers.x2;
        if (!util::PageAligned(size)) {
            state.ctx->registers.w0 = result::InvalidSize;
            state.logger->Warn("svcCreateTransferMemory: 'size' {}: 0x{:X}", size ? "not page aligned" : "is zero", size);
            return;
        }

        memory::Permission permission = *reinterpret_cast<memory::Permission *>(&state.ctx->registers.w3);
        if ((permission.w && !permission.r) || (permission.x && !permission.r)) {
            state.logger->Warn("svcCreateTransferMemory: 'permission' invalid: {}{}{}", permission.r ? "R" : "-", permission.w ? "W" : "-", permission.x ? "X" : "-");
            state.ctx->registers.w0 = result::InvalidNewMemoryPermission;
            return;
        }

        state.logger->Debug("svcCreateTransferMemory: Creating transfer memory at 0x{:X} for {} bytes ({}{}{})", address, size, permission.r ? "R" : "-", permission.w ? "W" : "-", permission.x ? "X" : "-");

        auto shmem = state.process->NewHandle<type::KTransferMemory>(state.process->pid, address, size, permission);

        state.ctx->registers.w0 = Result{};
        state.ctx->registers.w1 = shmem.handle;
    }

    void CloseHandle(DeviceState &state) {
        auto handle = static_cast<KHandle>(state.ctx->registers.w0);
        try {
            state.process->handles.erase(handle);
            state.logger->Debug("svcCloseHandle: Closing handle: 0x{:X}", handle);
            state.ctx->registers.w0 = Result{};
        } catch (const std::exception &) {
            state.logger->Warn("svcCloseHandle: 'handle' invalid: 0x{:X}", handle);
            state.ctx->registers.w0 = result::InvalidHandle;
        }
    }

    void ResetSignal(DeviceState &state) {
        auto handle = state.ctx->registers.w0;
        try {
            auto &object = state.process->handles.at(handle);
            switch (object->objectType) {
                case type::KType::KEvent:
                    std::static_pointer_cast<type::KEvent>(object)->ResetSignal();
                    break;

                case type::KType::KProcess:
                    std::static_pointer_cast<type::KProcess>(object)->ResetSignal();
                    break;

                default: {
                    state.logger->Warn("svcResetSignal: 'handle' type invalid: 0x{:X} ({})", handle, object->objectType);
                    state.ctx->registers.w0 = result::InvalidHandle;
                    return;
                }
            }

            state.logger->Debug("svcResetSignal: Resetting signal: 0x{:X}", handle);
            state.ctx->registers.w0 = Result{};
        } catch (const std::out_of_range &) {
            state.logger->Warn("svcResetSignal: 'handle' invalid: 0x{:X}", handle);
            state.ctx->registers.w0 = result::InvalidHandle;
            return;
        }
    }

    void WaitSynchronization(DeviceState &state) {
        constexpr auto maxSyncHandles = 0x40; // The total amount of handles that can be passed to WaitSynchronization

        auto numHandles = state.ctx->registers.w2;
        if (numHandles > maxSyncHandles) {
            state.ctx->registers.w0 = result::OutOfHandles;
            return;
        }

        std::string handleStr;
        std::vector<std::shared_ptr<type::KSyncObject>> objectTable;
        std::vector<KHandle> waitHandles(numHandles);

        state.process->ReadMemory(waitHandles.data(), state.ctx->registers.x1, numHandles * sizeof(KHandle));

        for (const auto &handle : waitHandles) {
            handleStr += fmt::format("* 0x{:X}\n", handle);

            auto object = state.process->handles.at(handle);
            switch (object->objectType) {
                case type::KType::KProcess:
                case type::KType::KThread:
                case type::KType::KEvent:
                case type::KType::KSession:
                    break;

                default: {
                    state.ctx->registers.w0 = result::InvalidHandle;
                    return;
                }
            }

            objectTable.push_back(std::static_pointer_cast<type::KSyncObject>(object));
        }

        auto timeout = state.ctx->registers.x3;
        state.logger->Debug("svcWaitSynchronization: Waiting on handles:\n{}Timeout: 0x{:X} ns", handleStr, timeout);

        auto start = util::GetTimeNs();
        while (true) {
            if (state.thread->cancelSync) {
                state.thread->cancelSync = false;
                state.ctx->registers.w0 = result::Cancelled;
                break;
            }

            uint index{};
            for (const auto &object : objectTable) {
                if (object->signalled) {
                    state.logger->Debug("svcWaitSynchronization: Signalled handle: 0x{:X}", waitHandles.at(index));
                    state.ctx->registers.w0 = Result{};
                    state.ctx->registers.w1 = index;
                    return;
                }
                index++;
            }

            if ((util::GetTimeNs() - start) >= timeout) {
                state.logger->Debug("svcWaitSynchronization: Wait has timed out");
                state.ctx->registers.w0 = result::TimedOut;
                return;
            }
        }
    }

    void CancelSynchronization(DeviceState &state) {
        try {
            state.process->GetHandle<type::KThread>(state.ctx->registers.w0)->cancelSync = true;
        } catch (const std::exception &) {
            state.logger->Warn("svcCancelSynchronization: 'handle' invalid: 0x{:X}", state.ctx->registers.w0);
            state.ctx->registers.w0 = result::InvalidHandle;
        }
    }

    void ArbitrateLock(DeviceState &state) {
        auto address = state.ctx->registers.x1;
        if (!util::WordAligned(address)) {
            state.logger->Warn("svcArbitrateLock: 'address' not word aligned: 0x{:X}", address);
            state.ctx->registers.w0 = result::InvalidAddress;
            return;
        }

        auto ownerHandle = state.ctx->registers.w0;
        auto requesterHandle = state.ctx->registers.w2;
        if (requesterHandle != state.thread->handle)
            throw exception("svcWaitProcessWideKeyAtomic: Handle doesn't match current thread: 0x{:X} for thread 0x{:X}", requesterHandle, state.thread->handle);

        state.logger->Debug("svcArbitrateLock: Locking mutex at 0x{:X}", address);

        if (state.process->MutexLock(address, ownerHandle))
            state.logger->Debug("svcArbitrateLock: Locked mutex at 0x{:X}", address);
        else
            state.logger->Debug("svcArbitrateLock: Owner handle did not match current owner for mutex or didn't have waiter flag at 0x{:X}", address);

        state.ctx->registers.w0 = Result{};
    }

    void ArbitrateUnlock(DeviceState &state) {
        auto address = state.ctx->registers.x0;
        if (!util::WordAligned(address)) {
            state.logger->Warn("svcArbitrateUnlock: 'address' not word aligned: 0x{:X}", address);
            state.ctx->registers.w0 = result::InvalidAddress;
            return;
        }

        state.logger->Debug("svcArbitrateUnlock: Unlocking mutex at 0x{:X}", address);

        if (state.process->MutexUnlock(address)) {
            state.logger->Debug("svcArbitrateUnlock: Unlocked mutex at 0x{:X}", address);
            state.ctx->registers.w0 = Result{};
        } else {
            state.logger->Debug("svcArbitrateUnlock: A non-owner thread tried to release a mutex at 0x{:X}", address);
            state.ctx->registers.w0 = result::InvalidAddress;
        }
    }

    void WaitProcessWideKeyAtomic(DeviceState &state) {
        auto mtxAddress = state.ctx->registers.x0;
        if (!util::WordAligned(mtxAddress)) {
            state.logger->Warn("svcWaitProcessWideKeyAtomic: mutex address not word aligned: 0x{:X}", mtxAddress);
            state.ctx->registers.w0 = result::InvalidAddress;
            return;
        }

        auto condAddress = state.ctx->registers.x1;
        auto handle = state.ctx->registers.w2;
        if (handle != state.thread->handle)
            throw exception("svcWaitProcessWideKeyAtomic: Handle doesn't match current thread: 0x{:X} for thread 0x{:X}", handle, state.thread->handle);

        if (!state.process->MutexUnlock(mtxAddress)) {
            state.logger->Debug("WaitProcessWideKeyAtomic: A non-owner thread tried to release a mutex at 0x{:X}", mtxAddress);
            state.ctx->registers.w0 = result::InvalidAddress;
            return;
        }

        auto timeout = state.ctx->registers.x3;
        state.logger->Debug("svcWaitProcessWideKeyAtomic: Mutex: 0x{:X}, Conditional-Variable: 0x{:X}, Timeout: {} ns", mtxAddress, condAddress, timeout);

        if (state.process->ConditionalVariableWait(condAddress, mtxAddress, timeout)) {
            state.logger->Debug("svcWaitProcessWideKeyAtomic: Waited for conditional variable and relocked mutex");
            state.ctx->registers.w0 = Result{};
        } else {
            state.logger->Debug("svcWaitProcessWideKeyAtomic: Wait has timed out");
            state.ctx->registers.w0 = result::TimedOut;
        }
    }

    void SignalProcessWideKey(DeviceState &state) {
        auto address = state.ctx->registers.x0;
        auto count = state.ctx->registers.w1;

        state.logger->Debug("svcSignalProcessWideKey: Signalling Conditional-Variable at 0x{:X} for {}", address, count);
        state.process->ConditionalVariableSignal(address, count);
        state.ctx->registers.w0 = Result{};
    }

    void GetSystemTick(DeviceState &state) {
        u64 tick;
        asm("STR X1, [SP, #-16]!\n\t"
            "MRS %0, CNTVCT_EL0\n\t"
            "MOV X1, #0xF800\n\t"
            "MOVK X1, #0x124, lsl #16\n\t"
            "MUL %0, %0, X1\n\t"
            "MRS X1, CNTFRQ_EL0\n\t"
            "UDIV %0, %0, X1\n\t"
            "LDR X1, [SP], #16" : "=r"(tick));
        state.ctx->registers.x0 = tick;
    }

    void ConnectToNamedPort(DeviceState &state) {
        constexpr auto portSize = 0x8; //!< The size of a port name string
        std::string_view port(state.process->GetPointer<char>(state.ctx->registers.x1), portSize);

        KHandle handle{};
        if (port.compare("sm:") >= 0) {
            handle = state.process->NewHandle<type::KSession>(std::static_pointer_cast<service::BaseService>(state.os->serviceManager.smUserInterface)).handle;
        } else {
            state.logger->Warn("svcConnectToNamedPort: Connecting to invalid port: '{}'", port);
            state.ctx->registers.w0 = result::NotFound;
            return;
        }

        state.logger->Debug("svcConnectToNamedPort: Connecting to port '{}' at 0x{:X}", port, handle);

        state.ctx->registers.w1 = handle;
        state.ctx->registers.w0 = Result{};
    }

    void SendSyncRequest(DeviceState &state) {
        state.os->serviceManager.SyncRequestHandler(static_cast<KHandle>(state.ctx->registers.x0));
        state.ctx->registers.w0 = Result{};
    }

    void GetThreadId(DeviceState &state) {
        constexpr KHandle threadSelf = 0xFFFF8000; // This is the handle used by threads to refer to themselves
        auto handle = state.ctx->registers.w1;
        pid_t pid{};

        if (handle != threadSelf)
            pid = state.process->GetHandle<type::KThread>(handle)->tid;
        else
            pid = state.thread->tid;

        state.logger->Debug("svcGetThreadId: Handle: 0x{:X}, PID: {}", handle, pid);

        state.ctx->registers.x1 = static_cast<u64>(pid);
        state.ctx->registers.w0 = Result{};
    }

    void OutputDebugString(DeviceState &state) {
        auto debug = state.process->GetString(state.ctx->registers.x0, state.ctx->registers.x1);

        if (debug.back() == '\n')
            debug.pop_back();

        state.logger->Info("Debug Output: {}", debug);
        state.ctx->registers.w0 = Result{};
    }

    void GetInfo(DeviceState &state) {
        auto id0 = state.ctx->registers.w1;
        auto handle = state.ctx->registers.w2;
        auto id1 = state.ctx->registers.x3;

        u64 out{};

        constexpr auto totalPhysicalMemory = 0xF8000000; // ~4 GB of RAM

        switch (id0) {
            case constant::infoState::AllowedCpuIdBitmask:
            case constant::infoState::AllowedThreadPriorityMask:
            case constant::infoState::IsCurrentProcessBeingDebugged:
            case constant::infoState::TitleId:
            case constant::infoState::PrivilegedProcessId:
                break;

            case constant::infoState::AliasRegionBaseAddr:
                out = state.os->memory.alias.address;
                break;

            case constant::infoState::AliasRegionSize:
                out = state.os->memory.alias.size;
                break;

            case constant::infoState::HeapRegionBaseAddr:
                out = state.os->memory.heap.address;
                break;

            case constant::infoState::HeapRegionSize:
                out = state.os->memory.heap.size;
                break;

            case constant::infoState::TotalMemoryAvailable:
                out = totalPhysicalMemory;
                break;

            case constant::infoState::TotalMemoryUsage:
                out = state.process->heap->size + constant::DefStackSize + state.os->memory.GetProgramSize();
                break;

            case constant::infoState::AddressSpaceBaseAddr:
                out = state.os->memory.base.address;
                break;

            case constant::infoState::AddressSpaceSize:
                out = state.os->memory.base.size;
                break;

            case constant::infoState::StackRegionBaseAddr:
                out = state.os->memory.stack.address;
                break;

            case constant::infoState::StackRegionSize:
                out = state.os->memory.stack.size;
                break;

            case constant::infoState::PersonalMmHeapSize:
                out = totalPhysicalMemory;
                break;

            case constant::infoState::PersonalMmHeapUsage:
                out = state.process->heap->address + constant::DefStackSize;
                break;

            case constant::infoState::TotalMemoryAvailableWithoutMmHeap:
                out = totalPhysicalMemory; // TODO: NPDM specifies SystemResourceSize, subtract that from this
                break;

            case constant::infoState::TotalMemoryUsedWithoutMmHeap:
                out = state.process->heap->size + constant::DefStackSize; // TODO: Same as above
                break;

            case constant::infoState::UserExceptionContextAddr:
                out = state.process->tlsPages[0]->Get(0);
                break;

            default:
                state.logger->Warn("svcGetInfo: Unimplemented case ID0: {}, ID1: {}", id0, id1);
                state.ctx->registers.w0 = result::InvalidEnumValue;
                return;
        }

        state.logger->Debug("svcGetInfo: ID0: {}, ID1: {}, Out: 0x{:X}", id0, id1, out);

        state.ctx->registers.x1 = out;
        state.ctx->registers.w0 = Result{};
    }
}
