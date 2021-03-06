// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "filesystem.h"

namespace skyline::vfs {
    /**
     * @brief The PartitionFileSystem class abstracts a partition filesystem using the vfs::FileSystem api
     */
    class PartitionFileSystem : public FileSystem {
      private:
        struct FsHeader {
            u32 magic; //!< The filesystem magic: 'PFS0' or 'HFS0'
            u32 numFiles; //!< The number of files in the filesystem
            u32 stringTableSize; //!< The size of the filesystem's string table
            u32 _pad_;
        } header;
        static_assert(sizeof(FsHeader) == 0x10);

        struct PartitionFileEntry {
            u64 offset; //!< The offset of the file in the backing
            u64 size; //!< The size of the file
            u32 stringTableOffset; //!< The offset of the file in the string table
            u32 _pad_;
        };
        static_assert(sizeof(PartitionFileEntry) == 0x18);

        struct HashedFileEntry {
            PartitionFileEntry entry; //!< The base file entry
            u32 _pad_;
            std::array<u8, 0x20> hash; //!< The hash of the file
        };
        static_assert(sizeof(HashedFileEntry) == 0x40);

        bool hashed; //!< Whether the filesystem contains hash data
        size_t fileDataOffset; //!< The offset from the backing to the base of the file data
        std::shared_ptr<Backing> backing; //!< The backing file of the filesystem
        std::unordered_map<std::string, PartitionFileEntry> fileMap; //!< A map that maps file names to their corresponding entry

      protected:
        std::shared_ptr<Backing> OpenFileImpl(const std::string &path, Backing::Mode mode) override;

        std::optional<Directory::EntryType> GetEntryTypeImpl(const std::string &path) override;

        std::shared_ptr<Directory> OpenDirectoryImpl(const std::string &path, Directory::ListMode listMode) override;

      public:
        PartitionFileSystem(const std::shared_ptr<Backing> &backing);
    };

    /**
     * @brief The PartitionFileSystemDirectory provides access to the root directory of a partition filesystem
     */
    class PartitionFileSystemDirectory : public Directory {
      private:
        std::vector<Entry> fileList; //!< A list of every file in the PFS root directory

      public:
        PartitionFileSystemDirectory(std::vector<Entry> fileList, ListMode listMode);

        std::vector<Entry> Read();
    };
}
