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

#include "exegesis/x86/cleanup_instruction_set_utils.h"

#include "exegesis/proto/instructions.pb.h"
#include "exegesis/testing/test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/text_format.h"

namespace exegesis {
namespace x86 {
namespace {

using ::exegesis::testing::EqualsProto;

TEST(AddOperandSizeOverrideToInstructionProtoTest, AddsPrefix) {
  constexpr char kInstructionProto[] = R"proto(
    vendor_syntax {
      mnemonic: 'ADC'
      operands {
        name: 'r/m16'
        addressing_mode: ANY_ADDRESSING_WITH_FLEXIBLE_REGISTERS
        encoding: MODRM_RM_ENCODING
        value_size_bits: 16
      }
      operands {
        name: 'imm16'
        addressing_mode: NO_ADDRESSING
        encoding: IMMEDIATE_VALUE_ENCODING
        value_size_bits: 16
      }
    }
    raw_encoding_specification: '81 /2 iw'
    x86_encoding_specification {
      legacy_prefixes {} opcode: 0x81 modrm_usage: OPCODE_EXTENSION_IN_MODRM
      modrm_opcode_extension: 2
      immediate_value_bytes: 2
    })proto";
  constexpr char kExpectedInstructionProto[] = R"proto(
    vendor_syntax {
      mnemonic: 'ADC'
      operands {
        name: 'r/m16'
        addressing_mode: ANY_ADDRESSING_WITH_FLEXIBLE_REGISTERS
        encoding: MODRM_RM_ENCODING
        value_size_bits: 16
      }
      operands {
        name: 'imm16'
        addressing_mode: NO_ADDRESSING
        encoding: IMMEDIATE_VALUE_ENCODING
        value_size_bits: 16
      }
    }
    raw_encoding_specification: '66 81 /2 iw'
    x86_encoding_specification {
      legacy_prefixes { has_mandatory_operand_size_override_prefix: true }
      opcode: 0x81
      modrm_usage: OPCODE_EXTENSION_IN_MODRM
      modrm_opcode_extension: 2
      immediate_value_bytes: 2
    })proto";
  InstructionProto instruction;
  ASSERT_TRUE(::google::protobuf::TextFormat::ParseFromString(kInstructionProto,
                                                              &instruction));
  AddOperandSizeOverrideToInstructionProto(&instruction);
  EXPECT_THAT(instruction, EqualsProto(kExpectedInstructionProto));
}

TEST(AddOperandSizeOverrideToInstructionProtoTest, DoesNotDuplicatePrefix) {
  constexpr char kInstructionProto[] = R"proto(
    vendor_syntax {
      mnemonic: 'ADC'
      operands {
        name: 'r/m16'
        addressing_mode: ANY_ADDRESSING_WITH_FLEXIBLE_REGISTERS
        encoding: MODRM_RM_ENCODING
        value_size_bits: 16
      }
      operands {
        name: 'imm16'
        addressing_mode: NO_ADDRESSING
        encoding: IMMEDIATE_VALUE_ENCODING
        value_size_bits: 16
      }
    }
    raw_encoding_specification: '66 81 /2 iw'
    x86_encoding_specification {
      legacy_prefixes { has_mandatory_operand_size_override_prefix: true }
      opcode: 0x81
      modrm_usage: OPCODE_EXTENSION_IN_MODRM
      modrm_opcode_extension: 2
      immediate_value_bytes: 2
    })proto";
  InstructionProto instruction;
  ASSERT_TRUE(::google::protobuf::TextFormat::ParseFromString(kInstructionProto,
                                                              &instruction));
  AddOperandSizeOverrideToInstructionProto(&instruction);
  EXPECT_THAT(instruction, EqualsProto(kInstructionProto));
}

TEST(AddOperandSizeOverrideToInstructionProtoTest,
     DoesNotUpdateSpecificationIfNotParsed) {
  constexpr char kInstructionProto[] = R"proto(
    vendor_syntax {
      mnemonic: 'ADC'
      operands {
        name: 'r/m16'
        addressing_mode: ANY_ADDRESSING_WITH_FLEXIBLE_REGISTERS
        encoding: MODRM_RM_ENCODING
        value_size_bits: 16
      }
      operands {
        name: 'imm16'
        addressing_mode: NO_ADDRESSING
        encoding: IMMEDIATE_VALUE_ENCODING
        value_size_bits: 16
      }
    }
    raw_encoding_specification: '81 /2 iw')proto";
  constexpr char kExpectedInstructionProto[] = R"proto(
    vendor_syntax {
      mnemonic: 'ADC'
      operands {
        name: 'r/m16'
        addressing_mode: ANY_ADDRESSING_WITH_FLEXIBLE_REGISTERS
        encoding: MODRM_RM_ENCODING
        value_size_bits: 16
      }
      operands {
        name: 'imm16'
        addressing_mode: NO_ADDRESSING
        encoding: IMMEDIATE_VALUE_ENCODING
        value_size_bits: 16
      }
    }
    raw_encoding_specification: '66 81 /2 iw'
  )proto";
  InstructionProto instruction;
  ASSERT_TRUE(::google::protobuf::TextFormat::ParseFromString(kInstructionProto,
                                                              &instruction));
  AddOperandSizeOverrideToInstructionProto(&instruction);
  EXPECT_THAT(instruction, EqualsProto(kExpectedInstructionProto));
}

}  // namespace
}  // namespace x86
}  // namespace exegesis
