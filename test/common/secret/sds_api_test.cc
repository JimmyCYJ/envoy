#include <memory>

#include "envoy/api/v2/auth/cert.pb.h"
#include "envoy/common/exception.h"

#include "common/secret/sds_api.h"

#include "test/mocks/server/mocks.h"
#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Secret {
namespace {

class MockServer : public Server::MockInstance {
public:
  Init::Manager& initManager() { return initmanager_; }

private:
  class InitManager : public Init::Manager {
  public:
    void initialize(std::function<void()> callback);
    void registerTarget(Init::Target&) override {}
  };

  InitManager initmanager_;
};

class SdsApiTest : public testing::Test {};

TEST_F(SdsApiTest, SecretUpdateSuccess) {
  MockServer server;
  envoy::api::v2::core::ConfigSource config_source;
  SdsApi sds_api(server, config_source, "abc.com");

  std::string yaml =
      R"EOF(
  name: "abc.com"
  tls_certificate:
    certificate_chain:
      filename: "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_cert.pem"
    private_key:
      filename: "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_key.pem"
    )EOF";

  Protobuf::RepeatedPtrField<envoy::api::v2::auth::Secret> secret_resources;
  auto secret_config = secret_resources.Add();
  MessageUtil::loadFromYaml(TestEnvironment::substitute(yaml), *secret_config);
  sds_api.onConfigUpdate(secret_resources, "");

  const std::string cert_pem = "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_cert.pem";
  EXPECT_EQ(TestEnvironment::readFileToStringForTest(TestEnvironment::substitute(cert_pem)),
            sds_api.secret()->certificateChain());

  const std::string key_pem = "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_key.pem";
  EXPECT_EQ(TestEnvironment::readFileToStringForTest(TestEnvironment::substitute(key_pem)),
            sds_api.secret()->privateKey());
}

TEST_F(SdsApiTest, EmptyResource) {
  MockServer server;
  envoy::api::v2::core::ConfigSource config_source;
  SdsApi sds_api(server, config_source, "abc.com");

  Protobuf::RepeatedPtrField<envoy::api::v2::auth::Secret> secret_resources;
  sds_api.onConfigUpdate(secret_resources, "");
  EXPECT_EQ(nullptr, sds_api.secret());
}

TEST_F(SdsApiTest, SecretUpdateWrongSize) {
  MockServer server;
  envoy::api::v2::core::ConfigSource config_source;
  SdsApi sds_api(server, config_source, "abc.com");

  std::string yaml =
      R"EOF(
    name: "abc.com"
    tls_certificate:
      certificate_chain:
        filename: "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_cert.pem"
      private_key:
        filename: "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_key.pem"
      )EOF";

  Protobuf::RepeatedPtrField<envoy::api::v2::auth::Secret> secret_resources;
  auto secret_config_1 = secret_resources.Add();
  MessageUtil::loadFromYaml(TestEnvironment::substitute(yaml), *secret_config_1);
  auto secret_config_2 = secret_resources.Add();
  MessageUtil::loadFromYaml(TestEnvironment::substitute(yaml), *secret_config_2);

  EXPECT_THROW_WITH_MESSAGE(sds_api.onConfigUpdate(secret_resources, ""), EnvoyException,
                            "Unexpected SDS secrets length: 2");
}

TEST_F(SdsApiTest, SecretUpdateWrongSecretName) {
  MockServer server;
  envoy::api::v2::core::ConfigSource config_source;
  SdsApi sds_api(server, config_source, "abc.com");

  std::string yaml =
      R"EOF(
      name: "wrong.name.com"
      tls_certificate:
        certificate_chain:
          filename: "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_cert.pem"
        private_key:
          filename: "{{ test_rundir }}/test/common/ssl/test_data/selfsigned_key.pem"
        )EOF";

  Protobuf::RepeatedPtrField<envoy::api::v2::auth::Secret> secret_resources;
  auto secret_config = secret_resources.Add();
  MessageUtil::loadFromYaml(TestEnvironment::substitute(yaml), *secret_config);

  EXPECT_THROW_WITH_MESSAGE(sds_api.onConfigUpdate(secret_resources, ""), EnvoyException,
                            "Unexpected SDS secret (expecting abc.com): wrong.name.com");
}

} // namespace
} // namespace Secret
} // namespace Envoy