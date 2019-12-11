#pragma once

#include <common.h>
#include <kernel/ipc.h>

namespace skyline::gpu {
    /**
     * @brief This class encapsulates a Parcel object (https://switchbrew.org/wiki/Display_services#Parcel)
     */
    class Parcel {
      private:
        /**
         * @brief This holds the header of a parcel
         */
        struct ParcelHeader {
            u32 dataSize;
            u32 dataOffset;
            u32 objectsSize;
            u32 objectsOffset;
        } header{};
        static_assert(sizeof(ParcelHeader) == 0x10);

        const DeviceState &state; //!< The state of the device

      public:
        std::vector<u8> data; //!< A vector filled with data in the parcel
        std::vector<u8> objects; //!< A vector filled with objects in the parcel

        /**
         * @brief This constructor fills in the Parcel object with data from a IPC buffer
         * @param buffer The buffer that contains the parcel
         * @param state The state of the device
         */
        Parcel(kernel::ipc::InputBuffer &buffer, const DeviceState &state);

        /**
         * @brief This constructor fills in the Parcel object with data from a Parcel on a remote process
         * @param address The remote address of the parcel
         * @param size The size of the parcel
         * @param state The state of the device
         */
        Parcel(u64 address, u64 size, const DeviceState &state);

        /**
         * @brief This constructor is used to create an empty parcel then write to a process
         * @param state The state of the device
         */
        Parcel(const DeviceState &state);

        /**
         * @brief Writes some data to the Parcel
         * @tparam ValueType The type of the object to write
         * @param value The object to be written
         */
        template<typename ValueType>
        void WriteData(const ValueType &value) {
            data.reserve(data.size() + sizeof(ValueType));
            auto item = reinterpret_cast<const u8 *>(&value);
            for (uint index = 0; sizeof(ValueType) > index; index++) {
                data.push_back(*item);
                item++;
            }
        }

        /**
         * @brief Writes an object to the Parcel
         * @tparam ValueType The type of the object to write
         * @param value The object to be written
         */
        template<typename ValueType>
        void WriteObject(const ValueType &value) {
            objects.reserve(objects.size() + sizeof(ValueType));
            auto item = reinterpret_cast<const u8 *>(&value);
            for (uint index = 0; sizeof(ValueType) > index; index++) {
                objects.push_back(*item);
                item++;
            }
        }

        /**
         * @brief Writes the Parcel object into a particular output buffer on a process
         * @param buffer The buffer to write into
         * @param process The process to write the Parcel to
         * @return The total size of the message
         */
        u64 WriteParcel(kernel::ipc::OutputBuffer& buffer, pid_t process = 0);

        /**
         * @brief Writes the Parcel object into the process's memory
         * @param address The address to write the Parcel object to
         * @param maxSize The maximum size of the Parcel
         * @param process The process to write the Parcel to
         * @return The total size of the message
         */
        u64 WriteParcel(u64 address, u64 maxSize, pid_t process = 0);
    };
}
