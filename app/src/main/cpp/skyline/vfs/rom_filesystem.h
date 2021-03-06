// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "filesystem.h"

namespace skyline {
    namespace constant {
        constexpr u32 RomFsEmptyEntry{0xFFFFFFFF}; //!< The value a RomFS entry has its offset set to, if it's empty
    }

    namespace vfs {
        /**
         * @brief The RomFileSystem class abstracts access to a RomFS image using the vfs::FileSystem api
         */
        class RomFileSystem : public FileSystem {
          private:
            std::shared_ptr<Backing> backing;

            /**
             * @brief Traverses the sibling files of the given file and adds them to the file map
             * @param offset The offset of the file entry to traverses the siblings of
             * @param path The path to the parent directory of the supplied file entry
             */
            void TraverseFiles(u32 offset, const std::string &path);

            /**
             * @brief Traverses the directories within the given directory, adds them to the directory map and calls TraverseFiles on them
             * @param offset The offset of the directory entry to traverses the directories in
             * @param path The path to the supplied directory entry
             */
            void TraverseDirectory(u32 offset, const std::string &path);

          protected:
            std::shared_ptr<Backing> OpenFileImpl(const std::string &path, Backing::Mode mode) override;

            std::optional<Directory::EntryType> GetEntryTypeImpl(const std::string &path) override;

            std::shared_ptr<Directory> OpenDirectoryImpl(const std::string &path, Directory::ListMode listMode) override;

          public:
            struct RomFsHeader {
                u64 headerSize; //!< The size of the header
                u64 dirHashTableOffset; //!< The offset of the directory hash table
                u64 dirHashTableSize; //!< The size of the directory hash table
                u64 dirMetaTableOffset; //!< The offset of the directory metadata table
                u64 dirMetaTableSize; //!< The size of the directory metadata table
                u64 fileHashTableOffset; //!< The offset of the file hash table
                u64 fileHashTableSize; //!< The size of the file hash table
                u64 fileMetaTableOffset; //!< The offset of the file metadata table
                u64 fileMetaTableSize; //!< The size of the file metadata table
                u64 dataOffset; //!< The offset of the file data
            } header{};
            static_assert(sizeof(RomFsHeader) == 0x50);

            struct RomFsDirectoryEntry {
                u32 parentOffset; //!< The offset from the directory metadata base of the parent directory
                u32 siblingOffset; //!< The offset from the directory metadata base of a sibling directory
                u32 childOffset; //!< The offset from the directory metadata base of a child directory
                u32 fileOffset; //!< The offset from the file metadata base of a child file
                u32 hash; //!< The hash of the directory
                u32 nameSize; //!< The size of the directory's name in bytes
            };

            struct RomFsFileEntry {
                u32 parentOffset; //!< The offset from the directory metadata base of the parent directory
                u32 siblingOffset; //!< The offset from the file metadata base of a sibling file
                u64 offset; //!< The offset from the file data base of the file contents
                u64 size; //!< The size of the file in bytes
                u32 hash; //!< The hash of the file
                u32 nameSize; //!< The size of the file's name in bytes
            };

            std::unordered_map<std::string, RomFsFileEntry> fileMap; //!< A map that maps file names to their corresponding entry
            std::unordered_map<std::string, RomFsDirectoryEntry> directoryMap; //!< A map that maps directory names to their corresponding entry

            RomFileSystem(std::shared_ptr<Backing> backing);
        };

        /**
         * @brief The RomFileSystemDirectory provides access to directories within a RomFS
         */
        class RomFileSystemDirectory : public Directory {
          private:
            RomFileSystem::RomFsDirectoryEntry ownEntry; //!< This directory's entry in the RomFS header
            RomFileSystem::RomFsHeader header; //!< A header of this files parent RomFS image
            std::shared_ptr<Backing> backing;

          public:
            RomFileSystemDirectory(std::shared_ptr<Backing> backing, const RomFileSystem::RomFsHeader &header, const RomFileSystem::RomFsDirectoryEntry &ownEntry, ListMode listMode);

            std::vector<Entry> Read();
        };
    }
}
