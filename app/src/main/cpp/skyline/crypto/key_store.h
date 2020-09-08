// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <array>
#include <vfs/backing.h>
#include "common.h"

namespace skyline::crypto {
    /**
     * @brief The KeyStore class looks for title.keys and prod.keys files in rootPath
     * @note Both files are created on kotlin side, prod.keys contains keys that are used to decrypt ROMs and title key, decrypted title keys are used for ctr backing.
     */
    class KeyStore {
      public:
        KeyStore(const std::string &rootPath);

        using Key128 = std::array<u8, 16>;
        using Key256 = std::array<u8, 32>;
        using IndexedKeys128 = std::array<std::optional<Key128>, 20>;

        std::optional<Key256> headerKey;

        IndexedKeys128 titleKek;
        IndexedKeys128 areaKeyApplication;
        IndexedKeys128 areaKeyOcean;
        IndexedKeys128 areaKeySystem;
      private:
        /**
         * @brief The 256-bit keys used for decryption
         */
        enum class Keys256 : u8 {
            Header
        };

        /**
         * @brief The 128-bit keys used for decryption
         * @note These are indexed because NCA header defines which key generation to use
         */
        enum class Keys128 : u8 {
            TitleKek,
            KeyAreaKeyApplication,
            KeyAreaKeyOcean,
            KeyAreaKeySystem
        };

        std::map<Key128, Key128> titleKeys;

        std::unordered_map<std::string_view, Keys256> key256Names{
            {"header_key", Keys256::Header}
        };

        std::unordered_map<std::string_view, Keys128> indexedKey128Names{
            {"titlekek_", Keys128::TitleKek},
            {"key_area_key_application_", Keys128::KeyAreaKeyApplication},
            {"key_area_key_ocean_", Keys128::KeyAreaKeyOcean},
            {"key_area_key_system_", Keys128::KeyAreaKeySystem}
        };

        using ReadPairsCallback = void (skyline::crypto::KeyStore::*)(const std::string_view &, const std::string_view &);

        void ReadPairs(const std::shared_ptr<vfs::Backing> &backing, ReadPairsCallback callback);

        void PopulateTitleKeys(const std::string_view &keyName, const std::string_view &value);

        void PopulateKeys(const std::string_view &keyName, const std::string_view &value);

      public:
        inline std::optional<Key128> GetTitleKey(const Key128 &title) {
            auto it{titleKeys.find(title)};
            if (it == titleKeys.end())
                return std::nullopt;
            return it->second;
        }
    };
}
