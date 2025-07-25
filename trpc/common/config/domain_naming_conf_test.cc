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

#include "trpc/common/config/domain_naming_conf.h"
#include "trpc/common/config/domain_naming_conf_parser.h"

#include "gtest/gtest.h"
#include "yaml-cpp/yaml.h"

TEST(LoadbalancerConfig, load_test) {
  trpc::naming::DomainSelectorConfig domain_selector_config;
  domain_selector_config.exclude_ipv6 = true;
  domain_selector_config.Display();

  YAML::convert<trpc::naming::DomainSelectorConfig> c;
  YAML::Node config_node = c.encode(domain_selector_config);

  trpc::naming::DomainSelectorConfig tmp;
  ASSERT_TRUE(c.decode(config_node, tmp));

  tmp.Display();
  ASSERT_EQ(domain_selector_config.exclude_ipv6, tmp.exclude_ipv6);
}
