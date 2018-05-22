#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "envoy/common/pure.h"

namespace Envoy {
namespace Secret {

/**
 * Secret contains certificate chain and private key
 */
class Secret {
public:
  virtual ~Secret() {}

  /**
   * @return a name of the SDS secret
   */
  virtual const std::string& name() PURE;

  /**
   * @return a string of certificate chain
   */
  virtual const std::string& certificateChain() PURE;

  /**
   * @return a string of private key
   */
  virtual const std::string& privateKey() PURE;
};

typedef std::shared_ptr<Secret> SecretSharedPtr;

} // namespace Secret
} // namespace Envoy
