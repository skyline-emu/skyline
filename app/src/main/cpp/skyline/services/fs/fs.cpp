#include "fs.h"
#include <kernel/types/KProcess.h>

namespace skyline::service::fs {
    fsp::fsp(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::fs_fsp, {
        {0x1, SFUNC(fsp::SetCurrentProcess)},
        {0x12, SFUNC(fsp::OpenSdCardFileSystem)},
        {0xC8, SFUNC(fsp::OpenDataStorageByCurrentProcess)}
    }) {}

    void fsp::SetCurrentProcess(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        process = *reinterpret_cast<pid_t *>(request.cmdArg);
    }

    void fsp::OpenSdCardFileSystem(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        manager.RegisterService(std::make_shared<IFileSystem>(FsType::SdCard, state, manager), session, response);
    }

    void fsp::OpenDataStorageByCurrentProcess(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        manager.RegisterService(std::make_shared<IStorage>(state.fileSystem->romFs, state, manager), session, response);
    }

    IFileSystem::IFileSystem(FsType type, const DeviceState &state, ServiceManager &manager) : type(type), BaseService(state, manager, false, Service::fs_IFileSystem, {}) {}

    IStorage::IStorage(const std::vector<u8> dataVec, const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::fs_IStorage, {
        {0x0, SFUNC(IStorage::Read)}
    }) {
        this->data = dataVec;
    }

    void IStorage::Read(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto offset = request.Pop<u64>();
        auto size = request.Pop<u64>();


        state.process->WriteMemory(data.data() + offset, request.outputBuf.at(0).address, std::min(static_cast<u64>(data.size() - offset), size));
    }
}
