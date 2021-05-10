// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "gpu/presentation_engine.h"

using CacheAddr = std::uintptr_t;
[[nodiscard]] inline CacheAddr ToCacheAddr(const void* host_ptr) {
    return reinterpret_cast<CacheAddr>(host_ptr);
}

[[nodiscard]] inline u8* FromCacheAddr(CacheAddr cache_addr) {
    return reinterpret_cast<u8*>(cache_addr);
}

namespace Core {
namespace Frontend {
class EmuWindow;
}
class System;
} // namespace Core

namespace VideoCore {
class RendererBase;
class ShaderNotify;
} // namespace VideoCore

namespace Tegra {

enum class RenderTargetFormat : u32 {
    NONE = 0x0,
    R32B32G32A32_FLOAT = 0xC0,
    R32G32B32A32_SINT = 0xC1,
    R32G32B32A32_UINT = 0xC2,
    R16G16B16A16_UNORM = 0xC6,
    R16G16B16A16_SNORM = 0xC7,
    R16G16B16A16_SINT = 0xC8,
    R16G16B16A16_UINT = 0xC9,
    R16G16B16A16_FLOAT = 0xCA,
    R32G32_FLOAT = 0xCB,
    R32G32_SINT = 0xCC,
    R32G32_UINT = 0xCD,
    R16G16B16X16_FLOAT = 0xCE,
    B8G8R8A8_UNORM = 0xCF,
    B8G8R8A8_SRGB = 0xD0,
    A2B10G10R10_UNORM = 0xD1,
    A2B10G10R10_UINT = 0xD2,
    A8B8G8R8_UNORM = 0xD5,
    A8B8G8R8_SRGB = 0xD6,
    A8B8G8R8_SNORM = 0xD7,
    A8B8G8R8_SINT = 0xD8,
    A8B8G8R8_UINT = 0xD9,
    R16G16_UNORM = 0xDA,
    R16G16_SNORM = 0xDB,
    R16G16_SINT = 0xDC,
    R16G16_UINT = 0xDD,
    R16G16_FLOAT = 0xDE,
    B10G11R11_FLOAT = 0xE0,
    R32_SINT = 0xE3,
    R32_UINT = 0xE4,
    R32_FLOAT = 0xE5,
    R5G6B5_UNORM = 0xE8,
    A1R5G5B5_UNORM = 0xE9,
    R8G8_UNORM = 0xEA,
    R8G8_SNORM = 0xEB,
    R8G8_SINT = 0xEC,
    R8G8_UINT = 0xED,
    R16_UNORM = 0xEE,
    R16_SNORM = 0xEF,
    R16_SINT = 0xF0,
    R16_UINT = 0xF1,
    R16_FLOAT = 0xF2,
    R8_UNORM = 0xF3,
    R8_SNORM = 0xF4,
    R8_SINT = 0xF5,
    R8_UINT = 0xF6,
};

enum class DepthFormat : u32 {
    D32_FLOAT = 0xA,
    D16_UNORM = 0x13,
    S8_UINT_Z24_UNORM = 0x14,
    D24X8_UNORM = 0x15,
    D24S8_UNORM = 0x16,
    D24C8_UNORM = 0x18,
    D32_FLOAT_S8X24_UINT = 0x19,
};


namespace skyline::gpu {
    /**
     * @brief An interface to host GPU structures, anything concerning host GPU/Presentation APIs is encapsulated by this
     */
    class GPU {
      private:
        static vk::raii::Instance CreateInstance(const DeviceState &state, const vk::raii::Context &context);

        static vk::raii::DebugReportCallbackEXT CreateDebugReportCallback(const DeviceState &state, const vk::raii::Instance &instance);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *layerPrefix, const char *message, Logger *logger);

        static vk::raii::PhysicalDevice CreatePhysicalDevice(const DeviceState &state, const vk::raii::Instance &instance);

        static vk::raii::Device CreateDevice(const DeviceState &state, const vk::raii::PhysicalDevice &physicalDevice, typeof(vk::DeviceQueueCreateInfo::queueCount)& queueConfiguration);

      public:
        vk::raii::Context vkContext; //!< An overarching context for Vulkan with
        vk::raii::Instance vkInstance; //!< An instance of Vulkan with all application context
        vk::raii::DebugReportCallbackEXT vkDebugReportCallback; //!< An RAII Vulkan debug report manager which calls into DebugCallback
        vk::raii::PhysicalDevice vkPhysicalDevice; //!< The underlying physical Vulkan device from which we derieve our logical device
        typeof(vk::DeviceQueueCreateInfo::queueCount) vkQueueFamilyIndex{}; //!< The index of the family the queue is from
        vk::raii::Device vkDevice; //!< The logical Vulkan device which we want to render using
        vk::raii::Queue vkQueue; //!< A Vulkan Queue supporting graphics and compute operations

        PresentationEngine presentation;

        GPU(const DeviceState &state);
    };
}
