#pragma once

#include <services/base_service.h>
#include <services/serviceman.h>

namespace skyline::service::fs {
    /**
     * @brief These are the possible types of the filesystem
     */
    enum class FsType {
        Nand,
        SdCard,
        GameCard
    };

    /**
     * @brief fsp-srv or IFileSystemProxy is responsible for providing handles to file systems (https://switchbrew.org/wiki/Filesystem_services#fsp-srv)
     */
    class fsp : public BaseService {
      public:
        pid_t process{}; //!< This holds the PID set by SetCurrentProcess

        fsp(const DeviceState &state, ServiceManager &manager);

        /**
         * @brief This sets the PID of the process using FS currently (https://switchbrew.org/wiki/Filesystem_services#SetCurrentProcess)
         */
        void SetCurrentProcess(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief This returns a handle to an instance of #IFileSystem (https://switchbrew.org/wiki/Filesystem_services#IFileSystem) with type SDCard
         */
        void OpenSdCardFileSystem(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        /**
         * @brief OpenDataStorageByCurrentProcess() -> object<nn::fssrv::sf::IStorage> data_storage
         */
        void OpenDataStorageByCurrentProcess(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };

    /**
     * @brief IFileSystem is used to interact with a filesystem (https://switchbrew.org/wiki/Filesystem_services#IFileSystem)
     */
    class IFileSystem : public BaseService {
      public:
        FsType type;

        IFileSystem(FsType type, const DeviceState &state, ServiceManager &manager);
    };

    /**
     * @brief This is the interface for a raw device, usually a block device. (https://switchbrew.org/wiki/Filesystem_services#IFileSystem#IStorage)
     */
    class IStorage : public BaseService {
      public:
        std::vector<u8> data;

        /**
         * @brief constructor for IStorage service
         * @param data buffer of a blcok device
         */
        IStorage(const std::vector<u8> dataVec, const DeviceState &state, ServiceManager &manager);

        /**
         * @brief Takes a type-0x46 buffer, an offset and length.(https://switchbrew.org/wiki/Filesystem_services#IStorage#Read)
         */
        void Read(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);
    };
}
