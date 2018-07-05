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

#include "exegesis/x86/cleanup_instruction_set_operand_info.h"

#include "exegesis/base/cleanup_instruction_set_test_utils.h"
#include "exegesis/testing/test_util.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/text_format.h"
#include "util/task/status.h"

namespace exegesis {
namespace x86 {
namespace {

using ::exegesis::testing::StatusIs;
using ::exegesis::util::Status;
using ::exegesis::util::error::INVALID_ARGUMENT;

TEST(AddOperandInfoTest, AddInfo) {
  constexpr char kInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STOS'
        operands { name: 'BYTE PTR [RDI]' }
        operands { name: 'AL' }
      }
      x86_encoding_specification { legacy_prefixes {} }
    }
    instructions {
      vendor_syntax {
        mnemonic: 'FMUL'
        operands { name: 'ST(0)' }
        operands { name: 'ST(i)' }
      }
      x86_encoding_specification {
        legacy_prefixes {}
        modrm_usage: OPCODE_EXTENSION_IN_MODRM
      }
    }
    instructions {
      vendor_syntax {
        mnemonic: 'VMOVD'
        operands { name: 'xmm1' encoding: MODRM_REG_ENCODING }
        operands { name: 'r32' }
      }
      x86_encoding_specification {
        vex_prefix {
          prefix_type: VEX_PREFIX
          vector_size: VEX_VECTOR_SIZE_128_BIT
          mandatory_prefix: MANDATORY_PREFIX_OPERAND_SIZE_OVERRIDE
          map_select: MAP_SELECT_0F
          vex_w_usage: VEX_W_IS_ZERO
        }
        modrm_usage: FULL_MODRM
      }
    })proto";
  constexpr char kExpectedInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STOS'
        operands {
          name: 'BYTE PTR [RDI]'
          addressing_mode: INDIRECT_ADDRESSING_BY_RDI
          encoding: IMPLICIT_ENCODING
          value_size_bits: 8
        }
        operands {
          name: 'AL'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
          value_size_bits: 8
        }
      }
      x86_encoding_specification { legacy_prefixes {} }
    }
    instructions {
      vendor_syntax {
        mnemonic: 'FMUL'
        operands {
          name: 'ST(0)'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
          value_size_bits: 80
        }
        operands {
          name: 'ST(i)'
          addressing_mode: DIRECT_ADDRESSING
          encoding: MODRM_RM_ENCODING
          value_size_bits: 80
        }
      }
      x86_encoding_specification {
        legacy_prefixes {}
        modrm_usage: OPCODE_EXTENSION_IN_MODRM
      }
    }
    instructions {
      vendor_syntax {
        mnemonic: 'VMOVD'
        operands {
          name: 'xmm1'
          addressing_mode: DIRECT_ADDRESSING
          encoding: MODRM_REG_ENCODING
          value_size_bits: 128
        }
        operands {
          name: 'r32'
          addressing_mode: DIRECT_ADDRESSING
          encoding: MODRM_RM_ENCODING
          value_size_bits: 32
        }
      }
      x86_encoding_specification {
        vex_prefix {
          prefix_type: VEX_PREFIX
          vector_size: VEX_VECTOR_SIZE_128_BIT
          mandatory_prefix: MANDATORY_PREFIX_OPERAND_SIZE_OVERRIDE
          map_select: MAP_SELECT_0F
          vex_w_usage: VEX_W_IS_ZERO
        }
        modrm_usage: FULL_MODRM
      }
    })proto";
  TestTransform(AddOperandInfo, kInstructionSetProto,
                kExpectedInstructionSetProto);
}

TEST(AddOperandInfoTest, DetectsInconsistentEncodings) {
  constexpr const char* const kInstructionSetProtos[] = {
      // The instruction encoding does not use the ModR/M byte, so the operands
      // can't use MODRM_RM_ENCODING.
      R"proto(
        instructions {
          vendor_syntax {
            mnemonic: 'STOS'
            operands { name: 'BYTE PTR [RDI]' encoding: MODRM_RM_ENCODING }
            operands { name: 'AL' }
          }
          x86_encoding_specification { legacy_prefixes {} }
        })proto",
      // Only one operand can be encoded in the opcode.
      R"proto(
        instructions {
          vendor_syntax {
            mnemonic: 'FMUL'
            operands { name: 'ST(0)' encoding: OPCODE_ENCODING }
            operands { name: 'ST(i)' encoding: MODRM_RM_ENCODING }
          }
          x86_encoding_specification {
            legacy_prefixes {}
            operand_in_opcode: FP_STACK_REGISTER_IN_OPCODE
          }
        })proto"};
  for (const char* const instruction_set_proto : kInstructionSetProtos) {
    InstructionSetProto instruction_set;
    ASSERT_TRUE(::google::protobuf::TextFormat::ParseFromString(
        instruction_set_proto, &instruction_set));
    EXPECT_THAT(AddOperandInfo(&instruction_set), StatusIs(INVALID_ARGUMENT));
  }
}

TEST(AddMissingOperandUsageTest, AddMissingOperandUsage) {
  constexpr char kInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STUFF'
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
        }
        operands { name: 'imm64' encoding: IMMEDIATE_VALUE_ENCODING }
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
        }
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
          usage: USAGE_WRITE
        }
        operands {
          name: 'xmm1'
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_V_ENCODING
        }
        operands {
          addressing_mode: NO_ADDRESSING
          encoding: IMPLICIT_ENCODING
          name: "1"
        }
      }
    })proto";
  constexpr char kExpectedInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STUFF'
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
          usage: USAGE_WRITE
        }
        operands {
          name: 'imm64'
          encoding: IMMEDIATE_VALUE_ENCODING
          usage: USAGE_READ
        }
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
          usage: USAGE_READ
        }
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
          usage: USAGE_WRITE
        }
        operands {
          name: 'xmm1'
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_V_ENCODING
          usage: USAGE_READ
        }
        operands {
          addressing_mode: NO_ADDRESSING
          encoding: IMPLICIT_ENCODING
          name: "1"
          usage: USAGE_READ
        }
      }
    })proto";
  TestTransform(AddMissingOperandUsage, kInstructionSetProto,
                kExpectedInstructionSetProto);
}

TEST(AddMissingOperandUsageToVblendInstructionsTest, AddMissingOperandUsage) {
  constexpr char kInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STUFF'
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
        }
        operands { name: 'imm64' encoding: IMMEDIATE_VALUE_ENCODING }
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
        }
      }
    }
    instructions {
      vendor_syntax {
        mnemonic: "VBLENDVPD"
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: MODRM_REG_ENCODING
          value_size_bits: 128
          name: "xmm1"
          usage: USAGE_WRITE
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_V_ENCODING
          value_size_bits: 128
          name: "xmm2"
          usage: USAGE_READ
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: INDIRECT_ADDRESSING
          encoding: MODRM_RM_ENCODING
          value_size_bits: 128
          name: "m128"
          usage: USAGE_READ
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_SUFFIX_ENCODING
          value_size_bits: 128
          name: "xmm4"
          register_class: VECTOR_REGISTER_128_BIT
        }
      }
    }
    instructions {
      vendor_syntax {
        mnemonic: "VBLENDVPD"
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: MODRM_REG_ENCODING
          value_size_bits: 128
          name: "xmm1"
          usage: USAGE_WRITE
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_V_ENCODING
          value_size_bits: 128
          name: "xmm2"
          usage: USAGE_READ
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: INDIRECT_ADDRESSING
          encoding: MODRM_RM_ENCODING
          value_size_bits: 128
          name: "m128"
          usage: USAGE_READ
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_SUFFIX_ENCODING
          value_size_bits: 128
          name: "xmm4"
          register_class: VECTOR_REGISTER_128_BIT
          usage: USAGE_READ_WRITE
        }
      }
    })proto";
  constexpr char kExpectedInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STUFF'
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
        }
        operands { name: 'imm64' encoding: IMMEDIATE_VALUE_ENCODING }
        operands {
          name: 'r64'
          addressing_mode: DIRECT_ADDRESSING
          encoding: IMPLICIT_ENCODING
        }
      }
    }
    instructions {
      vendor_syntax {
        mnemonic: "VBLENDVPD"
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: MODRM_REG_ENCODING
          value_size_bits: 128
          name: "xmm1"
          usage: USAGE_WRITE
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_V_ENCODING
          value_size_bits: 128
          name: "xmm2"
          usage: USAGE_READ
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: INDIRECT_ADDRESSING
          encoding: MODRM_RM_ENCODING
          value_size_bits: 128
          name: "m128"
          usage: USAGE_READ
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_SUFFIX_ENCODING
          value_size_bits: 128
          name: "xmm4"
          register_class: VECTOR_REGISTER_128_BIT
          usage: USAGE_READ
        }
      }
    }
    instructions {
      vendor_syntax {
        mnemonic: "VBLENDVPD"
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: MODRM_REG_ENCODING
          value_size_bits: 128
          name: "xmm1"
          usage: USAGE_WRITE
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_V_ENCODING
          value_size_bits: 128
          name: "xmm2"
          usage: USAGE_READ
          register_class: VECTOR_REGISTER_128_BIT
        }
        operands {
          addressing_mode: INDIRECT_ADDRESSING
          encoding: MODRM_RM_ENCODING
          value_size_bits: 128
          name: "m128"
          usage: USAGE_READ
        }
        operands {
          addressing_mode: DIRECT_ADDRESSING
          encoding: VEX_SUFFIX_ENCODING
          value_size_bits: 128
          name: "xmm4"
          register_class: VECTOR_REGISTER_128_BIT
          usage: USAGE_READ_WRITE
        }
      }
    })proto";
  TestTransform(AddMissingOperandUsageToVblendInstructions,
                kInstructionSetProto, kExpectedInstructionSetProto);
}

TEST(AddRegisterClassToOperandsTest, AddRegisterClassToOperands) {
  constexpr char kInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STUFF'
        operands { name: 'r64' }
        operands { name: 'imm64' }
        operands { name: 'm8' }
        operands { name: 'k' }
        operands { name: 'xmm1' }
      }
    })proto";
  constexpr char kExpectedInstructionSetProto[] = R"proto(
    instructions {
      vendor_syntax {
        mnemonic: 'STUFF'
        operands { name: 'r64' register_class: GENERAL_PURPOSE_REGISTER_64_BIT }
        operands { name: 'imm64' }
        operands { name: 'm8' register_class: INVALID_REGISTER_CLASS }
        operands { name: 'k' register_class: MASK_REGISTER }
        operands { name: 'xmm1' register_class: VECTOR_REGISTER_128_BIT }
      }
    })proto";
  TestTransform(AddRegisterClassToOperands, kInstructionSetProto,
                kExpectedInstructionSetProto);
}

}  // namespace
}  // namespace x86
}  // namespace exegesis
