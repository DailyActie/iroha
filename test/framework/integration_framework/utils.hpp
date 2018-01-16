/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_INTEGRATION_UTLS_HPP
#define IROHA_INTEGRATION_UTLS_HPP

#include "crypto/keys_manager_impl.hpp"
#include "model/model_crypto_provider_impl.hpp"
#include "model/transaction.hpp"

/**
 * Creates the new copy of the tx and sign it
 *
 * @param t transaction for signing
 * @param keypair
 * @return signed tx
 */
inline iroha::model::Transaction addSignature(
    const iroha::model::Transaction &t, iroha::keypair_t keypair) {
  iroha::model::Transaction tx = t;
  tx.created_ts = iroha::time::now();
  iroha::model::ModelCryptoProviderImpl(keypair).sign(tx);
  return tx;
}

#endif  // IROHA_INTEGRATION_UTLS_HPP
