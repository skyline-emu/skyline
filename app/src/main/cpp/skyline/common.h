// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <map>
#include <unordered_map>
#include <span>
#include <vector>
#include <fstream>
#include <mutex>
#include <functional>
#include <thread>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sstream>
#include <memory>
#include <sys/mman.h>
#include <fmt/format.h>
#include <frozen/unordered_map.h>
#include <frozen/string.h>
#include <jni.h>
#include "nce/guest_common.h"

namespace skyline {
    namespace frz = frozen;
    using KHandle = u32; //!< The type of a kernel handle

    /**
     * @brief The result of an operation in HOS
     * @url https://switchbrew.org/wiki/Error_codes
     */
    union Result {
        u32 raw{};
        struct __attribute__((packed)) {
            u16 module : 9;
            u16 id : 12;
        };

        /**
         * @note Success is 0, 0 - it is the only error that's not specific to a module
         */
        Result() = default;

        constexpr Result(u16 module, u16 id) {
            this->module = module;
            this->id = id;
        }

        constexpr operator u32() const {
            return raw;
        }
    };

    namespace constant {
        // Memory
        constexpr u64 BaseAddress{0x8000000}; //!< The address space base
        constexpr u64 DefStackSize{0x1E8480}; //!< The default amount of stack: 2 MB
        // Display
        constexpr u16 HandheldResolutionW{1280}; //!< The width component of the handheld resolution
        constexpr u16 HandheldResolutionH{720}; //!< The height component of the handheld resolution
        constexpr u16 DockedResolutionW{1920}; //!< The width component of the docked resolution
        constexpr u16 DockedResolutionH{1080}; //!< The height component of the docked resolution
        // Time
        constexpr u64 NsInSecond{1000000000}; //!< The amount of nanoseconds in a second
    }

    /**
     * @brief A wrapper over std::runtime_error with libfmt formatting
     */
    class exception : public std::runtime_error {
      public:
        /**
         * @param formatStr The exception string to be written, with libfmt formatting
         * @param args The arguments based on format_str
         */
        template<typename S, typename... Args>
        inline exception(const S &formatStr, Args &&... args) : runtime_error(fmt::format(formatStr, args...)) {}
    };

    namespace util {
        /**
         * @brief Returns the current time in nanoseconds
         * @return The current time in nanoseconds
         */
        inline u64 GetTimeNs() {
            static u64 frequency{};
            if (!frequency)
                asm("MRS %0, CNTFRQ_EL0" : "=r"(frequency));
            u64 ticks;
            asm("MRS %0, CNTVCT_EL0" : "=r"(ticks));
            return ((ticks / frequency) * constant::NsInSecond) + (((ticks % frequency) * constant::NsInSecond + (frequency / 2)) / frequency);
        }

        /**
         * @brief Returns the current time in arbitrary ticks
         * @return The current time in ticks
         */
        inline u64 GetTimeTicks() {
            u64 ticks;
            asm("MRS %0, CNTVCT_EL0" : "=r"(ticks));
            return ticks;
        }

        /**
         * @return The value aligned up to the next multiple
         * @note The multiple needs to be a power of 2
         */
        template<typename TypeVal, typename TypeMul>
        constexpr TypeVal AlignUp(TypeVal value, TypeMul multiple) {
            multiple--;
            return (value + multiple) & ~(multiple);
        }

        /**
         * @return The value aligned down to the previous multiple
         * @note The multiple needs to be a power of 2
         */
        template<typename TypeVal, typename TypeMul>
        constexpr TypeVal AlignDown(TypeVal value, TypeMul multiple) {
            return value & ~(multiple - 1);
        }

        /**
         * @return If the address is aligned with the multiple
         */
        template<typename TypeVal, typename TypeMul>
        constexpr bool IsAligned(TypeVal value, TypeMul multiple) {
            if ((multiple & (multiple - 1)) == 0)
                return !(value & (multiple - 1U));
            else
                return (value % multiple) == 0;
        }

        /**
         * @return If the value is page aligned
         */
        constexpr bool PageAligned(u64 value) {
            return IsAligned(value, PAGE_SIZE);
        }

        /**
         * @return If the value is word aligned
         */
        constexpr bool WordAligned(u64 value) {
            return IsAligned(value, WORD_BIT / 8);
        }

        /**
         * @param string The string to create a magic from
         * @return The magic of the supplied string
         */
        template<typename Type>
        constexpr Type MakeMagic(std::string_view string) {
            Type object{};
            size_t offset{};

            for (auto &character : string) {
                object |= static_cast<Type>(character) << offset;
                offset += sizeof(character) * 8;
            }

            return object;
        }

        constexpr u8 HexDigitToByte(char digit) {
            if (digit >= '0' && digit <= '9')
                return digit - '0';
            else if (digit >= 'a' && digit <= 'f')
                return digit - 'a' + 10;
            else if (digit >= 'A' && digit <= 'F')
                return digit - 'A' + 10;
            throw exception(fmt::format("Invalid hex char {}", digit));
        }

        template<size_t Size>
        constexpr std::array<u8, Size> HexStringToArray(std::string_view hexString) {
            if (hexString.size() != Size * 2)
                throw exception("Invalid size");
            std::array<u8, Size> result;
            for (size_t i{}; i < Size; i++) {
                size_t hexStrIndex{i * 2};
                result[i] = (HexDigitToByte(hexString[hexStrIndex]) << 4) | HexDigitToByte(hexString[hexStrIndex + 1]);
            }
            return result;
        }

        /**
         * @brief A compile-time hash function as std::hash isn't constexpr
         */
        constexpr std::size_t Hash(std::string_view view) {
            return frz::elsa<frz::string>{}(frz::string(view.data(), view.size()), 0);
        }
    }

    /**
     * @brief A custom wrapper over span that adds several useful methods to it
     * @note This class is completely transparent, it implicitly converts from and to span
     */
    template<typename T, size_t Extent = std::dynamic_extent>
    class span : public std::span<T, Extent> {
      public:
        using std::span<T, Extent>::span;
        using std::span<T, Extent>::operator=;

        typedef typename std::span<T, Extent>::element_type elementType;
        typedef typename std::span<T, Extent>::index_type indexType;

        constexpr span(const std::span<T, Extent> &spn) : std::span<T, Extent>(spn) {}

        /**
         * @brief We want to support implicitly casting from std::string_view -> span as it is just a specialization of a data view which span is a generic form of, the opposite doesn't hold true as not all data held by a span is string data therefore the conversion isn't implicit there
         */
        template<class Traits>
        constexpr span(const std::basic_string_view<T, Traits> &string) : std::span<T, Extent>(const_cast<T *>(string.data()), string.size()) {}

        template<typename Out>
        constexpr Out &as() {
            if (span::size_bytes() >= sizeof(Out))
                return *reinterpret_cast<Out *>(span::data());
            throw exception("Span size is less than Out type size (0x{:X}/0x{:X})", span::size_bytes(), sizeof(Out));
        }

        /**
         * @param nullTerminated If true and the string is null-terminated, a view of it will be returned (not including the null terminator itself), otherwise the entire span will be returned as a string view
         */
        constexpr std::string_view as_string(bool nullTerminated = false) {
            return std::string_view(reinterpret_cast<char *>(span::data()), nullTerminated ? (std::find(span::begin(), span::end(), 0) - span::begin()) : span::size_bytes());
        }

        template<typename Out, size_t OutExtent = std::dynamic_extent>
        constexpr span<Out> cast() {
            if (util::IsAligned(span::size_bytes(), sizeof(Out)))
                return span<Out, OutExtent>(reinterpret_cast<Out *>(span::data()), span::size_bytes() / sizeof(Out));
            throw exception("Span size not aligned with Out type size (0x{:X}/0x{:X})", span::size_bytes(), sizeof(Out));
        }

        /**
         * @brief Copies data from the supplied span into this one
         * @param amount The amount of elements that need to be copied (in terms of the supplied span), 0 will try to copy the entirety of the other span
         */
        template<typename In, size_t InExtent>
        constexpr void copy_from(const span<In, InExtent> spn, indexType amount = 0) {
            auto size{amount ? amount * sizeof(In) : spn.size_bytes()};
            if (span::size_bytes() < size)
                throw exception("Data being copied is larger than this span");
            std::memmove(span::data(), spn.data(), size);
        }

        /**
         * @brief Implicit type conversion for copy_from, this allows passing in std::vector/std::array in directly is automatically passed by reference which is important for any containers
         */
        template<typename In>
        constexpr void copy_from(const In &in, indexType amount = 0) {
            copy_from(span<typename std::add_const<typename In::value_type>::type>(in), amount);
        }

        /** Base Class Functions that return an instance of it, we upcast them **/
        template<size_t Count>
        constexpr span<T, Count> first() const noexcept {
            return std::span<T, Extent>::template first<Count>();
        }

        template<size_t Count>
        constexpr span<T, Count> last() const noexcept {
            return std::span<T, Extent>::template last<Count>();
        }

        constexpr span<elementType, std::dynamic_extent> first(indexType count) const noexcept {
            return std::span<T, Extent>::first(count);
        }

        constexpr span<elementType, std::dynamic_extent> last(indexType count) const noexcept {
            return std::span<T, Extent>::last(count);
        }

        template<size_t Offset, size_t Count = std::dynamic_extent>
        constexpr auto subspan() const noexcept -> span<T, Count != std::dynamic_extent ? Count : Extent - Offset> {
            return std::span<T, Extent>::template subspan<Offset, Count>();
        }

        constexpr span<T, std::dynamic_extent> subspan(indexType offset, indexType count = std::dynamic_extent) const noexcept {
            return std::span<T, Extent>::subspan(offset, count);
        }
    };

    /**
     * @brief Deduction guides required for arguments to span, CTAD will fail for iterators, arrays and containers without this
     */
    template<class It, class End, size_t Extent = std::dynamic_extent>
    span(It, End) -> span<typename std::iterator_traits<It>::value_type, Extent>;
    template<class T, size_t Size>
    span(T (&)[Size]) -> span<T, Size>;
    template<class T, size_t Size>
    span(std::array<T, Size> &) -> span<T, Size>;
    template<class T, size_t Size>
    span(const std::array<T, Size> &) -> span<const T, Size>;
    template<class Container>
    span(Container &) -> span<typename Container::value_type>;
    template<class Container>
    span(const Container &) -> span<const typename Container::value_type>;

    /**
     * @brief The Mutex class is a wrapper around an atomic bool used for low-contention synchronization
     */
    class Mutex {
        std::atomic_flag flag = ATOMIC_FLAG_INIT; //!< An atomic flag to hold the state of the mutex

      public:
        /**
         * @brief Wait on and lock the mutex
         */
        void lock();

        /**
         * @brief Try to lock the mutex if it is unlocked else return
         * @return If the mutex was successfully locked or not
         */
        inline bool try_lock() {
            return !flag.test_and_set(std::memory_order_acquire);
        }

        /**
         * @brief Unlock the mutex if it is held by this thread
         */
        inline void unlock() {
            flag.clear(std::memory_order_release);
        }
    };

    /**
     * @brief The GroupMutex class is a special type of mutex that allows two groups of users and only allows one group to run in parallel
     */
    class GroupMutex {
      public:
        /**
         * @brief All the possible owners of the mutex
         */
        enum class Group : u8 {
            None = 0,   //!< No group owns this mutex
            Group1 = 1, //!< Group 1 owns this mutex
            Group2 = 2, //!< Group 2 owns this mutex
        };

        /**
         * @brief Wait on and lock the mutex
         */
        void lock(Group group = Group::Group1);

        /**
         * @brief Unlock the mutex
         * @note Undefined behavior in case unlocked by thread in non-owner group
         */
        void unlock();

      private:
        std::atomic<Group> flag{Group::None}; //!< An atomic flag to hold which group holds the mutex
        std::atomic<Group> next{Group::None}; //!< An atomic flag to hold which group will hold the mutex next
        std::atomic<u8> num{}; //!< An atomic u8 keeping track of how many users are holding the mutex
        Mutex mtx; //!< A mutex to lock before changing of num and flag
    };

    /**
     * @brief The Logger class is to write log output to file and logcat
     */
    class Logger {
      private:
        std::ofstream logFile; //!< An output stream to the log file
        Mutex mtx; //!< A mutex to lock before logging anything

      public:
        enum class LogLevel {
            Error,
            Warn,
            Info,
            Debug,
        };

        LogLevel configLevel; //!< The minimum level of logs to write

        /**
         * @param path The path of the log file
         * @param configLevel The minimum level of logs to write
         */
        Logger(const std::string &path, LogLevel configLevel);

        /**
         * @brief Writes the termination message to the log file
         */
        ~Logger();

        /**
         * @brief Writes a header, should only be used for emulation starting and ending
         * @param str The value to be written
         */
        void WriteHeader(const std::string &str);

        /**
         * @brief Write a log to the log file
         * @param level The level of the log
         * @param str The value to be written
         */
        void Write(LogLevel level, std::string str);

        /**
         * @brief Write an error log with libfmt formatting
         * @param formatStr The value to be written, with libfmt formatting
         * @param args The arguments based on format_str
         */
        template<typename S, typename... Args>
        inline void Error(const S &formatStr, Args &&... args) {
            if (LogLevel::Error <= configLevel) {
                Write(LogLevel::Error, fmt::format(formatStr, args...));
            }
        }

        /**
         * @brief Write a debug log with libfmt formatting
         * @param formatStr The value to be written, with libfmt formatting
         * @param args The arguments based on format_str
         */
        template<typename S, typename... Args>
        inline void Warn(const S &formatStr, Args &&... args) {
            if (LogLevel::Warn <= configLevel) {
                Write(LogLevel::Warn, fmt::format(formatStr, args...));
            }
        }

        /**
         * @brief Write a debug log with libfmt formatting
         * @param formatStr The value to be written, with libfmt formatting
         * @param args The arguments based on format_str
         */
        template<typename S, typename... Args>
        inline void Info(const S &formatStr, Args &&... args) {
            if (LogLevel::Info <= configLevel) {
                Write(LogLevel::Info, fmt::format(formatStr, args...));
            }
        }

        /**
         * @brief Write a debug log with libfmt formatting
         * @param formatStr The value to be written, with libfmt formatting
         * @param args The arguments based on format_str
         */
        template<typename S, typename... Args>
        inline void Debug(const S &formatStr, Args &&... args) {
            if (LogLevel::Debug <= configLevel) {
                Write(LogLevel::Debug, fmt::format(formatStr, args...));
            }
        }
    };

    /**
     * @brief The Settings class is used to access the parameters set in the Java component of the application
     */
    class Settings {
      private:
        std::unordered_map<std::string, std::string> stringMap; //!< A mapping from all keys to their corresponding string value
        std::unordered_map<std::string, bool> boolMap; //!< A mapping from all keys to their corresponding boolean value
        std::unordered_map<std::string, int> intMap; //!< A mapping from all keys to their corresponding integer value

      public:
        /**
         * @param fd An FD to the preference XML file
         */
        Settings(int fd);

        /**
         * @brief Retrieves a particular setting as a string
         * @param key The key of the setting
         * @return The string value of the setting
         */
        std::string GetString(const std::string &key);

        /**
         * @brief Retrieves a particular setting as a boolean
         * @param key The key of the setting
         * @return The boolean value of the setting
         */
        bool GetBool(const std::string &key);

        /**
         * @brief Retrieves a particular setting as a integer
         * @param key The key of the setting
         * @return The integer value of the setting
         */
        int GetInt(const std::string &key);

        /**
         * @brief Writes all settings keys and values to syslog, this function is for development purposes
         */
        void List(const std::shared_ptr<Logger> &logger);
    };

    class NCE;
    class JvmManager;
    namespace gpu {
        class GPU;
    }
    namespace kernel {
        namespace type {
            class KProcess;
            class KThread;
        }
        class OS;
    }
    namespace audio {
        class Audio;
    }
    namespace input {
        class Input;
    }
    namespace loader {
        class Loader;
    }

    /**
     * @brief The state of the entire emulator is contained within this class, all objects related to emulation are tied into it
     */
    struct DeviceState {
        DeviceState(kernel::OS *os, std::shared_ptr<kernel::type::KProcess> &process, std::shared_ptr<JvmManager> jvmManager, std::shared_ptr<Settings> settings, std::shared_ptr<Logger> logger);

        kernel::OS *os;
        std::shared_ptr<kernel::type::KProcess> &process;
        thread_local static std::shared_ptr<kernel::type::KThread> thread; //!< The KThread of the thread which accesses this object
        thread_local static ThreadContext *ctx; //!< The context of the guest thread for the corresponding host thread
        std::shared_ptr<NCE> nce;
        std::shared_ptr<gpu::GPU> gpu;
        std::shared_ptr<audio::Audio> audio;
        std::shared_ptr<input::Input> input;
        std::shared_ptr<loader::Loader> loader;
        std::shared_ptr<JvmManager> jvm;
        std::shared_ptr<Settings> settings;
        std::shared_ptr<Logger> logger;
    };
}
