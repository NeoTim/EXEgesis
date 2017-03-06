// Copyright 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cpu_instructions/util/proto_util.h"

#include <cstdio>
#include "strings/string.h"

#include "cpu_instructions/proto/instructions.pb.h"
#include "cpu_instructions/testing/test_util.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/text_format.h"
#include "strings/str_cat.h"

namespace cpu_instructions {
namespace {

using ::cpu_instructions::testing::EqualsProto;

TEST(ProtoUtilTest, ReadWriteTextProtoOrDie) {
  constexpr char kExpected[] = R"(
    llvm_mnemonic: 'ADD32mr')";
  const InstructionProto page =
      ParseProtoFromStringOrDie<InstructionProto>(kExpected);
  const string filename = StrCat(getenv("TEST_TMPDIR"), "/test.pbtxt");
  WriteTextProtoOrDie(filename, page);
  const InstructionProto read_proto =
      ReadTextProtoOrDie<InstructionProto>(filename);
  EXPECT_THAT(read_proto, EqualsProto(kExpected));
}

TEST(ProtoUtilTest, ParseProtoFromStringOrDie) {
  EXPECT_THAT(
      ParseProtoFromStringOrDie<InstructionProto>("llvm_mnemonic: 'ADD32mr'"),
      EqualsProto("llvm_mnemonic: 'ADD32mr'"));
}

TEST(ProtoUtilDeathTest, ParseProtoFromStringOrDie) {
  EXPECT_DEATH(ParseProtoFromStringOrDie<InstructionProto>("doesnotexist: 1"),
               "");
}

}  // namespace
}  // namespace cpu_instructions
