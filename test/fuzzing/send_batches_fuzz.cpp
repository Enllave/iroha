/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/tx_presence_cache_impl.hpp"
#include "fuzzing/fuzzing_logger.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "ordering_service_fixture.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, std::size_t size) {
  static fuzzing::OrderingServiceFixture fixture;

  if (size < 1) {
    return 0;
  }

  std::shared_ptr<OnDemandOrderingServiceImpl> ordering_service_;
  std::shared_ptr<OnDemandOsServerGrpc> server_;

  auto proposal_factory = std::make_unique<MockUnsafeProposalFactory>();
  auto storage = std::make_shared<NiceMock<iroha::ametsuchi::MockStorage>>();
  auto cache = std::make_shared<iroha::ametsuchi::TxPresenceCacheImpl>(storage);
  ordering_service_ =
      std::make_shared<OnDemandOrderingServiceImpl>(data[0],
                                                    std::move(proposal_factory),
                                                    std::move(cache),
                                                    getFuzzLogger("OdOs"));
  server_ =
      std::make_shared<OnDemandOsServerGrpc>(ordering_service_,
                                             fixture.transaction_factory_,
                                             fixture.batch_parser_,
                                             fixture.transaction_batch_factory_,
                                             getFuzzLogger("OdOsServerGrpc"));

  proto::BatchesRequest request;
  if (protobuf_mutator::libfuzzer::LoadProtoInput(
          true, data + 1, size - 1, &request)) {
    grpc::ServerContext context;
    google::protobuf::Empty response;
    server_->SendBatches(&context, &request, &response);
  }

  return 0;
}
