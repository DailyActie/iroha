/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <gmock/gmock.h>
#include <grpc++/grpc++.h>
#include "network/ordering_gate.hpp"
#include "ordering/impl/ordering_service_impl.hpp"

using namespace iroha::ordering;
using namespace iroha::model;
using namespace iroha::network;

using ::testing::_;
using ::testing::AtLeast;

class FakeOrderingGate : public OrderingGate,
                         public proto::OrderingGate::Service {
 public:
  MOCK_METHOD1(propagate_transaction, void(const Transaction&));
  MOCK_METHOD0(on_proposal, rxcpp::observable<Proposal>());
  MOCK_METHOD3(SendProposal,
               grpc::Status(::grpc::ServerContext*, const proto::Proposal*,
                            ::google::protobuf::Empty*));
};

TEST(OrderingServiceTest, SampleTest) {
  auto loop = uvw::Loop::getDefault();

  auto address = "0.0.0.0:50051";
  Peer peer;
  peer.address = address;

  auto service = std::make_shared<OrderingServiceImpl>(std::vector<Peer>{peer},
                                                       5, 1500, loop);
  std::unique_ptr<grpc::Server> server;

  auto gate = std::make_shared<FakeOrderingGate>();

  EXPECT_CALL(*gate, SendProposal(_, _, _)).Times(AtLeast(4));

  std::mutex mtx;
  std::condition_variable cv;

  auto s_thread = std::thread([&cv, address, &service, &server, &gate] {
    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials(), &port);
    builder.RegisterService(service.get());
    builder.RegisterService(gate.get());
    server = builder.BuildAndStart();
    ASSERT_NE(port, 0);
    ASSERT_TRUE(server);
    cv.notify_one();
    server->Wait();
  });

  std::unique_lock<std::mutex> lock(mtx);
  cv.wait(lock);

  auto l_thread = std::thread([&loop] { loop->run(); });

  auto stub = proto::OrderingService::NewStub(
      grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

  auto p1 = std::thread([&stub] {
    for (size_t i = 0; i < 10; ++i) {
      grpc::ClientContext context;

      google::protobuf::Empty reply;

      stub->SendTransaction(&context, iroha::protocol::Transaction(), &reply);
      std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }
  });
  auto p2 = std::thread([&stub] {
    for (size_t i = 0; i < 10; ++i) {
      grpc::ClientContext context;

      google::protobuf::Empty reply;

      stub->SendTransaction(&context, iroha::protocol::Transaction(), &reply);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });

  p1.join();
  p2.join();

  std::this_thread::sleep_for(std::chrono::seconds(5));

  loop->stop();
  server->Shutdown();
  if (l_thread.joinable()) {
    l_thread.join();
  }
  if (s_thread.joinable()) {
    s_thread.join();
  }
}