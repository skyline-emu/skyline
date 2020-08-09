// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <common.h>
#include <span>
#include <queue>
#include "engines/engine.h"
#include "engines/gpfifo.h"
#include "memory_manager.h"

namespace skyline::gpu {
    namespace gpfifo {
        /**
         * @brief This contains a single GPFIFO entry that is submitted through 'SubmitGpfifo'
         * @url https://nvidia.github.io/open-gpu-doc/manuals/volta/gv100/dev_pbdma.ref.txt
         * @url https://github.com/NVIDIA/open-gpu-doc/blob/ab27fc22db5de0d02a4cabe08e555663b62db4d4/classes/host/clb06f.h#L155
         */
        struct GpEntry {
            enum class Fetch {
                Unconditional = 0,
                Conditional = 1,
            };

            union {
                struct {
                    Fetch fetch : 1;
                    u8 _pad_ : 1;
                    u32 get : 30;
                };
                u32 entry0;
            };

            enum class Opcode : u8 {
                Nop = 0,
                Illegal = 1,
                Crc = 2,
                PbCrc = 3,
            };

            enum class Priv {
                User = 0,
                Kernel = 1,
            };

            enum class Level {
                Main = 0,
                Subroutine = 1,
            };

            enum class Sync {
                Proceed = 0,
                Wait = 1,
            };

            union {
                struct {
                    union {
                        u8 getHi;
                        Opcode opcode;
                    };
                    Priv priv : 1;
                    Level level : 1;
                    u32 size : 21;
                    Sync sync : 1;
                };
                u32 entry1;
            };
        };
        static_assert(sizeof(GpEntry) == 0x8);

        /**
         * @brief This holds a single pushbuffer method header that describes a compressed method sequence
         * @url https://github.com/NVIDIA/open-gpu-doc/blob/ab27fc22db5de0d02a4cabe08e555663b62db4d4/manuals/volta/gv100/dev_ram.ref.txt#L850
         * @url https://github.com/NVIDIA/open-gpu-doc/blob/ab27fc22db5de0d02a4cabe08e555663b62db4d4/classes/host/clb06f.h#L179
         */
        union PushBufferMethodHeader {
            enum class TertOp : u8 {
                Grp0IncMethod = 0,
                Grp0SetSubDevMask = 1,
                Grp0StoreSubDevMask = 2,
                Grp0UseSubDevMask = 3,
                Grp2NonIncMethod = 0
            };

            enum class SecOp : u8 {
                Grp0UseTert = 0,
                IncMethod = 1,
                Grp2UseTert = 2,
                NonIncMethod = 3,
                ImmdDataMethod = 4,
                OneInc = 5,
                Reserved6 = 6,
                EndPbSegment = 7
            };

            struct {
                union {
                    u16 methodAddress : 12;
                    struct {
                        u8 _pad0_ : 4;
                        u16 subDeviceMask : 12;
                    };

                    struct {
                        u16 _pad1_ : 13;
                        u8 methodSubChannel : 3;
                        union {
                            TertOp tertOp : 3;
                            u16 methodCount : 13;
                            u16 immdData : 13;
                        };
                    };

                    struct {
                        u32 _pad2_ : 29;
                        SecOp secOp : 3;
                    };
                };
            };
            u32 entry;
        };
        static_assert(sizeof(PushBufferMethodHeader) == 0x4);

        /**
         * @brief The GPFIFO class handles creating pushbuffers from GP entries and then processing them
         * @url https://github.com/NVIDIA/open-gpu-doc/blob/ab27fc22db5de0d02a4cabe08e555663b62db4d4/manuals/volta/gv100/dev_pbdma.ref.txt#L62
         */
        class GPFIFO {
          private:
            /**
             * @brief This is used to hold a pushbuffer's GPFIFO entry and contents, pushbuffers are made up of several 32-bit words
             */
            struct PushBuffer {
                GpEntry gpEntry;
                std::vector<u32> segment;

                PushBuffer(const GpEntry &gpEntry, const vmm::MemoryManager &memoryManager, bool fetch) : gpEntry(gpEntry) {
                    if (fetch)
                        Fetch(memoryManager);
                }

                inline void Fetch(const vmm::MemoryManager &memoryManager) {
                    segment.resize(gpEntry.size);
                    memoryManager.Read<u32>(segment, (static_cast<u64>(gpEntry.getHi) << 32) | (static_cast<u64>(gpEntry.get) << 2));
                }
            };

            const DeviceState &state;
            engine::GPFIFO gpfifoEngine; //!< The engine for processing GPFIFO method calls
            std::array<std::shared_ptr<engine::Engine>, 8> subchannels;
            std::queue<PushBuffer> pushBufferQueue;
            skyline::Mutex pushBufferQueueLock; //!< This is used to lock pushbuffer queue insertions as the GPU runs on a seperate thread

            /**
             * @brief Processes a pushbuffer segment, calling methods as needed
             */
            void Process(const std::vector<u32> &segment);

            /**
             * @brief This sends a method call to the GPU hardware
             */
            void Send(MethodParams params);

          public:
            GPFIFO(const DeviceState &state) : state(state), gpfifoEngine(state) {}

            /**
             * @brief Executes all pending entries in the FIFO
             */
            void Run();

            /**
             * @brief Pushes a list of entries to the FIFO, these commands will be executed on calls to 'Step'
             */
            void Push(std::span<GpEntry> entries);
        };
    }
}