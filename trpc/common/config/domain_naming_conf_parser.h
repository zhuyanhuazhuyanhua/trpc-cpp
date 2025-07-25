//
//
// Tencent is pleased to support the open source community by making tRPC available.
//
// Copyright (C) 2023 Tencent.
// All rights reserved.
//
// If you have downloaded a copy of the tRPC source code from Tencent,
// please note that tRPC source code is licensed under the  Apache 2.0 License,
// A copy of the Apache 2.0 License is included in this file.
//
//

#include "yaml-cpp/yaml.h"

#include "trpc/common/config/domain_naming_conf.h"

namespace YAML {

template <>
struct convert<trpc::naming::DomainSelectorConfig> {
  static YAML::Node encode(const trpc::naming::DomainSelectorConfig& config) {
    YAML::Node node;
    node["exclude_ipv6"] = config.exclude_ipv6;
    return node;
  }

  static bool decode(const YAML::Node& node, trpc::naming::DomainSelectorConfig& config) {
    if (node["exclude_ipv6"]) {
      config.exclude_ipv6 = node["exclude_ipv6"].as<bool>();
    }
    return true;
  }
};

}  // namespace YAML
