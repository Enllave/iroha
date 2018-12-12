/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_STORAGE_CLEANUP_STRATEGY_HPP
#define IROHA_YAC_STORAGE_CLEANUP_STRATEGY_HPP

#include <vector>

#include <boost/optional.hpp>

#include "consensus/round.hpp"
#include "consensus/yac/storage/storage_result.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class CleanupStrategy {
       public:
        /**
         * Collection of rounds type
         */
        using RoundsType = std::vector<Round>;

        /**
         * Notify strategy about new rounds
         * @param round - new round
         * @param answer - outcome of round
         * @return a collection of rounds for removing from the state
         */
        virtual boost::optional<RoundsType> finalize(Round round,
                                                     Answer answer) = 0;

        /**
         * The method checks should we add new round or not
         * @param round - interested round
         * @return true if round makes sense
         */
        virtual bool shouldCreateRound(const Round &round) = 0;

        virtual ~CleanupStrategy() = default;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_STORAGE_CLEANUP_STRATEGY_HPP
