#pragma once

#include <cstdint>
#include "loader.h"

namespace skyline::loader {
    class NsoLoader : public Loader {
      private:
        /**
         * @brief This holds a single data segment's offset and size
         */
        struct NsoSegmentHeader {
            u32 fileOffset; //!< The offset of the region
            u32 memoryOffset; //!< The memory offset where the region should be loaded
            u32 decompressedSize; //!< Size of the region after decompression
        };

        /**
         * @brief This holds the header of an NRO file
         */
        struct NsoHeader {
            u32 magic; //!< The NSO magic "NSO0"
            u32 version; //!< The version of the application
            u32 : 32;
            u32 flags; //!< The flags used with the NSO

            NsoSegmentHeader text; //!< The .text segment header
            u32 modOffset; //!< The offset of the MOD metadata
            NsoSegmentHeader ro; //!< The .ro segment header
            u32 modSize; //!< The size of the MOD metadata
            NsoSegmentHeader data; //!< The .data segment header
            u32 bssSize; //!< The size of the bss segment

            u64 buildId[4]; //!< The build ID of the NSO

            u32 textCompressedSize; //!< The size of the compressed .text segment
            u32 roCompressedSize; //!< The size of the compressed .ro segment
            u32 dataCompressedSize; //!< The size of the compressed .data segment

            u32 reserved[7];

            u64 apiInfo; //!< The .rodata-relative offset of .apiInfo
            u64 dynstr; //!< The .rodata-relative offset of .dynstr
            u64 dynsym; //!< The .rodata-relative offset of .dynsym

            u64 segmentHashes[3][4]; //!< The SHA256 checksums of the .text, .ro and .data segments
        } header{};

      public:
        /**
         * @param romFd The file descriptor to load the data from
         */
        NsoLoader(const int romFd);

        /**
         * This loads in the data of the main process
         * @param process The process to load in the data
         * @param state The state of the device
         */
        void LoadProcessData(const std::shared_ptr<kernel::type::KProcess> process, const DeviceState &state);
    };
}