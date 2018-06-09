#pragma once

#include <string>
#include <unordered_map>

#include "envoy/api/v2/auth/cert.pb.h"
#include "envoy/ssl/tls_certificate_config.h"

namespace Envoy {
namespace Ssl {

class TlsCertificateConfigImpl : public Ssl::TlsCertificateConfig {
public:
  TlsCertificateConfigImpl(const envoy::api::v2::auth::TlsCertificate& config);

  const std::string& certificateChain() const override { return certificate_chain_; }
  const std::string& privateKey() const override { return private_key_; }
  bool equalTo(const TlsCertificateConfig& secret) const override;

private:
  const std::string certificate_chain_;
  const std::string private_key_;
};

} // namespace Ssl
} // namespace Envoy
