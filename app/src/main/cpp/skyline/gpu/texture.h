// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <common.h>
#include <vulkan/vulkan_raii.hpp>

namespace skyline {
    namespace service::hosbinder {
        class GraphicBufferProducer;
    }
    namespace gpu {
        namespace texture {
            struct Dimensions {
                u32 width;
                u32 height;
                u32 depth;

                constexpr Dimensions() : width(0), height(0), depth(0) {}

                constexpr Dimensions(u32 width, u32 height) : width(width), height(height), depth(1) {}

                constexpr Dimensions(u32 width, u32 height, u32 depth) : width(width), height(height), depth(depth) {}

                auto operator<=>(const Dimensions &) const = default;
            };

            /**
             * @note Blocks refers to the atomic unit of a compressed format (IE: The minimum amount of data that can be decompressed)
             */
            struct Format {
                u8 bpb; //!< Bytes Per Block, this is used instead of bytes per pixel as that might not be a whole number for compressed formats
                u16 blockHeight; //!< The height of a block in pixels
                u16 blockWidth; //!< The width of a block in pixels
                vk::Format vkFormat;

                constexpr bool IsCompressed() {
                    return (blockHeight != 1) || (blockWidth != 1);
                }

                /**
                 * @param width The width of the texture in pixels
                 * @param height The height of the texture in pixels
                 * @param depth The depth of the texture in layers
                 * @return The size of the texture in bytes
                 */
                constexpr size_t GetSize(u32 width, u32 height, u32 depth = 1) {
                    return (((width / blockWidth) * (height / blockHeight)) * bpb) * depth;
                }

                constexpr size_t GetSize(Dimensions dimensions) {
                    return GetSize(dimensions.width, dimensions.height, dimensions.depth);
                }

                constexpr bool operator==(const Format &format) {
                    return vkFormat == format.vkFormat;
                }

                constexpr bool operator!=(const Format &format) {
                    return vkFormat != format.vkFormat;
                }

                /**
                 * @return If this format is actually valid or not
                 */
                constexpr operator bool() {
                    return bpb;
                }
            };

            /**
             * @brief The linearity of a texture, refer to Chapter 20.1 of the Tegra X1 TRM for information
             */
            enum class TileMode {
                Linear, //!< This is a purely linear texture
                Pitch,  //!< This is a pitch-linear texture
                Block,  //!< This is a 16Bx2 block-linear texture
            };

            /**
             * @brief The parameters of the tiling mode, covered in Table 76 in the Tegra X1 TRM
             */
            union TileConfig {
                struct {
                    u8 blockHeight; //!< The height of the blocks in GOBs
                    u8 blockDepth;  //!< The depth of the blocks in GOBs
                    u16 surfaceWidth;  //!< The width of a surface in samples
                };
                u32 pitch; //!< The pitch of the texture if it's pitch linear
            };

            enum class SwizzleChannel {
                Zero, //!< Write 0 to the channel
                One, //!< Write 1 to the channel
                Red, //!< Red color channel
                Green, //!< Green color channel
                Blue, //!< Blue color channel
                Alpha, //!< Alpha channel
            };

            struct Swizzle {
                SwizzleChannel red{SwizzleChannel::Red}; //!< Swizzle for the red channel
                SwizzleChannel green{SwizzleChannel::Green}; //!< Swizzle for the green channel
                SwizzleChannel blue{SwizzleChannel::Blue}; //!< Swizzle for the blue channel
                SwizzleChannel alpha{SwizzleChannel::Alpha}; //!< Swizzle for the alpha channel
            };
        }

        class Texture;

        /**
         * @brief A texture present in guest memory, it can be used to create a corresponding Texture object for usage on the host
         */
        class GuestTexture : public std::enable_shared_from_this<GuestTexture> {
          private:
            const DeviceState &state;

          public:
            u8 *pointer; //!< The address of the texture in guest memory
            std::weak_ptr<Texture> host; //!< A host texture (if any) that was created from this guest texture
            texture::Dimensions dimensions;
            texture::Format format;
            texture::TileMode tileMode;
            texture::TileConfig tileConfig;

            GuestTexture(const DeviceState &state, u8 *pointer, texture::Dimensions dimensions, texture::Format format, texture::TileMode tileMode = texture::TileMode::Linear, texture::TileConfig tileConfig = {});

            constexpr size_t Size() {
                return format.GetSize(dimensions);
            }

            /**
             * @brief Creates a corresponding host texture object for this guest texture
             * @param format The format of the host texture (Defaults to the format of the guest texture)
             * @param dimensions The dimensions of the host texture (Defaults to the dimensions of the host texture)
             * @param swizzle The channel swizzle of the host texture (Defaults to no channel swizzling)
             * @return A shared pointer to the host texture object
             * @note There can only be one host texture for a corresponding guest texture
             */
            std::shared_ptr<Texture> InitializeTexture(std::optional<texture::Format> format = std::nullopt, std::optional<texture::Dimensions> dimensions = std::nullopt, texture::Swizzle swizzle = {});
        };

        /**
         * @brief A texture which is backed by host constructs while being synchronized with the underlying guest texture
         */
        class Texture {
          private:
            const DeviceState &state;

          public:
            std::vector<u8> backing; //!< The object that holds a host copy of the guest texture (Will be replaced with a vk::Image)
            std::shared_ptr<GuestTexture> guest; //!< The guest texture from which this was created, it's required for syncing
            texture::Dimensions dimensions;
            texture::Format format;
            texture::Swizzle swizzle;

          public:
            Texture(const DeviceState &state, std::shared_ptr<GuestTexture> guest, texture::Dimensions dimensions, texture::Format format, texture::Swizzle swizzle);

          public:
            /**
             * @brief Convert this texture to the specified tiling mode
             * @param tileMode The tiling mode to convert it to
             * @param tileConfig The configuration for the tiling mode (Can be default argument for Linear)
             */
            void ConvertTileMode(texture::TileMode tileMode, texture::TileConfig tileConfig = {});

            /**
             * @brief Converts the texture dimensions to the specified ones (As long as they are within the GuestTexture's range)
             */
            void SetDimensions(texture::Dimensions dimensions);

            /**
             * @brief Converts the texture to have the specified format
             */
            void SetFormat(texture::Format format);

            /**
             * @brief Change the texture channel swizzle to the specified one
             */
            void SetSwizzle(texture::Swizzle swizzle);

            /**
             * @brief Synchronizes the host texture with the guest after it has been modified
             */
            void SynchronizeHost();

            /**
             * @brief Synchronizes the guest texture with the host texture after it has been modified
             */
            void SynchronizeGuest();
        };
    }
}
