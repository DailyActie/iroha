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

#include "module/shared_model/validators/validators_fixture.hpp"

#include "builders/protobuf/queries.hpp"

class QueryValidatorTest : public ValidatorsTest {
 public:
  shared_model::validation::DefaultQueryValidator query_validator;
};

using namespace iroha::protocol;
using namespace shared_model;

/**
 * @given Protobuf query object
 * @when Each query type created with valid fields
 * @then Answer has no errors
 */
TEST_F(QueryValidatorTest, StatelessValidTest) {
  iroha::protocol::Query qry;
  qry.mutable_payload()->set_creator_account_id(account_id);
  qry.mutable_payload()->set_created_time(created_time);
  qry.mutable_payload()->set_query_counter(counter);
  auto payload = qry.mutable_payload();

  // Iterate through all query types, filling query fields with valid values
  iterateContainer(
      [] {
        return iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
            "query");
      },
      [&](auto field) {
        // Set concrete type for new query
        return payload->GetReflection()->MutableMessage(payload, field);
      },
      [this](auto field, auto query) {
        // Will throw key exception in case new field is added
        try {
          field_setters.at(field->name())(query->GetReflection(), query, field);
        } catch (const std::out_of_range &e) {
          FAIL() << "Missing field setter: " << field->name();
        }
      },
      [&] {
        auto answer = query_validator.validate(
            detail::makePolymorphic<proto::Query>(qry));

        ASSERT_FALSE(answer.hasErrors()) << answer.reason();
      });
}

/**
 * @given Protobuf query object
 * @when Query has no fields set, and each query type has no fields set
 * @then Answer contains error
 */
TEST_F(QueryValidatorTest, StatelessInvalidTest) {
  iroha::protocol::Query qry;
  auto payload = qry.mutable_payload();

  // create queries from default constructors, which will have empty, therefore
  // invalid values
  iterateContainer(
      [] {
        return iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
            "query");
      },
      [&](auto field) {
        // Set concrete type for new query
        return payload->GetReflection()->MutableMessage(payload, field);
      },
      [](auto, auto) {
        // Note that no fields are set
      },
      [&] {
        auto answer = query_validator.validate(
            detail::makePolymorphic<proto::Query>(qry));

        ASSERT_TRUE(answer.hasErrors());
      });
}

/**
 * @given Protobuf query object
 * @when Each query type created with valid fields
 * @then Answer has no errors
 */
TEST_F(QueryValidatorTest, StatelessValidFromNearestFutureTest) {
  time_t gap = std::chrono::minutes(3) / std::chrono::milliseconds(1);
  iroha::protocol::Query qry;
  qry.mutable_payload()->set_created_time(created_time + gap);
  qry.mutable_payload()->set_creator_account_id(account_id);
  qry.mutable_payload()->set_query_counter(counter);
  auto payload = qry.mutable_payload();

  // Iterate through all query types, filling query fields with valid values
  iterateContainer(
      [] {
        return iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
            "query");
      },
      [&](auto field) {
        // Set concrete type for new query
        return payload->GetReflection()->MutableMessage(payload, field);
      },
      [this](auto field, auto query) {
        // Will throw key exception in case new field is added
        try {
          field_setters.at(field->name())(query->GetReflection(), query, field);
        } catch (const std::out_of_range &e) {
          FAIL() << "Missing field setter: " << field->name();
        }
      },
      [] {});
  auto answer =
      query_validator.validate(detail::makePolymorphic<proto::Query>(qry));

  ASSERT_FALSE(answer.hasErrors()) << answer.reason();
}

/**
 * @given Protobuf query object
 * @when Query is from far future(30 min)
 * @then Answer has an error
 */
TEST_F(QueryValidatorTest, StatelessInvalidFromFarFutureTest) {
  time_t gap = std::chrono::minutes(30) / std::chrono::milliseconds(1);
  iroha::protocol::Query qry;
  qry.mutable_payload()->set_created_time(created_time + gap);
  qry.mutable_payload()->set_creator_account_id(account_id);
  qry.mutable_payload()->set_query_counter(counter);
  auto payload = qry.mutable_payload();

  // Iterate through all query types, filling query fields with valid values
  iterateContainer(
      [] {
        return iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
            "query");
      },
      [&](auto field) {
        // Set concrete type for new query
        return payload->GetReflection()->MutableMessage(payload, field);
      },
      [this](auto field, auto query) {
        // Will throw key exception in case new field is added
        try {
          field_setters.at(field->name())(query->GetReflection(), query, field);
        } catch (const std::out_of_range &e) {
          FAIL() << "Missing field setter: " << field->name();
        }
      },
      [] {});
  auto answer =
      query_validator.validate(detail::makePolymorphic<proto::Query>(qry));

  ASSERT_TRUE(answer.hasErrors());
}
