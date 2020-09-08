// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <crypto/aes_cipher.h>
#include <crypto/key_store.h>
#include "backing.h"

namespace skyline::vfs {
    /**
     * @brief This backing is used to decrypt AES-CTR data
     */
    class CtrEncryptedBacking : public Backing {
      private:
        crypto::KeyStore::Key128 ctr;

        crypto::AesCipher cipher;

        std::shared_ptr<Backing> backing;

        /**
         * @brief Offset of file is used to calculate the IV
         */
        size_t baseOffset;

        /**
         * @brief Calculates IV based on the offset
         */
        void UpdateCtr(size_t offset);

      public:
        CtrEncryptedBacking(crypto::KeyStore::Key128 &ctr, crypto::KeyStore::Key128 &key, const std::shared_ptr<Backing> &backing, size_t baseOffset);

        size_t Read(u8 *output, size_t offset, size_t size) override;
    };
}
