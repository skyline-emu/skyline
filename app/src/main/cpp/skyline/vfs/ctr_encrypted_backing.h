// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <crypto/aes_cipher.h>
#include <crypto/key_store.h>
#include "backing.h"

namespace skyline::vfs {
    /**
     * @brief A backing for decrypting AES-CTR data
     */
    class CtrEncryptedBacking : public Backing {
      private:
        crypto::KeyStore::Key128 ctr;
        crypto::AesCipher cipher;
        std::shared_ptr<Backing> backing;
        std::mutex mutex; //!< Synchronize all AES-CTR cipher state modifications
        size_t baseOffset; //!< The offset of the backing into the file is used to calculate the IV

        /**
         * @brief Calculates IV based on the offset
         */
        void UpdateCtr(u64 offset);

      protected:
        size_t ReadImpl(span<u8> output, size_t offset) override;

      public:
        CtrEncryptedBacking(crypto::KeyStore::Key128 ctr, crypto::KeyStore::Key128 key, std::shared_ptr<Backing> backing, size_t baseOffset);
    };
}
