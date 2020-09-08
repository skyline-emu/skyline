// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <functional>
#include <vfs/os_filesystem.h>
#include "key_store.h"

namespace skyline::crypto {
    KeyStore::KeyStore(const std::string &rootPath) {
        vfs::OsFileSystem root(rootPath);
        if (root.FileExists("title.keys"))
            ReadPairs(root.OpenFile("title.keys"), &KeyStore::PopulateTitleKeys);
        if (root.FileExists("prod.keys"))
            ReadPairs(root.OpenFile("prod.keys"), &KeyStore::PopulateKeys);
    }

    void KeyStore::ReadPairs(const std::shared_ptr<vfs::Backing> &backing, ReadPairsCallback callback) {
        std::vector<char> fileContent(backing->size);
        backing->Read(fileContent.data(), 0, fileContent.size());

        auto lineStart{fileContent.begin()};
        std::vector<char>::iterator lineEnd;
        while ((lineEnd = std::find(lineStart, fileContent.end(), '\n')) != fileContent.end()) {
            auto keyEnd{std::find(lineStart, lineEnd, '=')};
            if (keyEnd == lineEnd) {
                throw exception("Invalid key file");
            }

            std::string_view key(&*lineStart, keyEnd - lineStart);
            std::string_view value(&*(keyEnd + 1), lineEnd - keyEnd - 1);
            (this->*callback)(key, value);

            lineStart = lineEnd + 1;
        }
    }

    void KeyStore::PopulateTitleKeys(const std::string_view &keyName, const std::string_view &value) {
        Key128 key{util::HexStringToArray<16>(keyName)};
        Key128 valueArray{util::HexStringToArray<16>(value)};
        titleKeys.insert({std::move(key), std::move(valueArray)});
    }

    void KeyStore::PopulateKeys(const std::string_view &keyName, const std::string_view &value) {
        auto it{key256Names.find(keyName)};
        if (it != key256Names.end()) {
            switch (it->second) {
                case Keys256::Header:
                    headerKey = util::HexStringToArray<32>(value);
                    return;
            }
        }

        for (const auto &pair : indexedKey128Names) {
            const auto &keyPrefix{pair.first};
            if (keyName.size() == keyPrefix.size() + 2 && keyName.substr(0, keyPrefix.size()) == keyPrefix) {
                size_t index{std::stoul(std::string(keyName.substr(keyPrefix.size())), nullptr, 16)};
                Key128 key{util::HexStringToArray<16>(value)};
                switch (pair.second) {
                    case Keys128::TitleKek:
                        titleKek[index] = key;
                        return;
                    case Keys128::KeyAreaKeyApplication:
                        areaKeyApplication[index] = key;
                        return;
                    case Keys128::KeyAreaKeyOcean:
                        areaKeyOcean[index] = key;
                        return;
                    case Keys128::KeyAreaKeySystem:
                        areaKeySystem[index] = key;
                        return;
                }
            }
        }
    }
}
