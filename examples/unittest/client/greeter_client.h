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

#pragma once

#include "examples/unittest/helloworld.trpc.pb.h"

namespace test {
namespace unittest {

using GreeterServiceProxyPtr = std::shared_ptr<::trpc::test::unittest::GreeterServiceProxy>;

class GreeterClient {
 public:
  explicit GreeterClient(GreeterServiceProxyPtr& proxy) : proxy_(proxy) {}

  bool SayHello();

  ::trpc::Future<> AsyncSayHello();

 private:
  GreeterServiceProxyPtr proxy_;
};

}  // namespace unittest
}  // namespace test