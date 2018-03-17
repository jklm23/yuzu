// Copyright 2018 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/bit_field.h"
#include "common/common_funcs.h"
#include "common/common_types.h"
#include "video_core/memory_manager.h"

namespace Tegra {
namespace Engines {

class Maxwell3D final {
public:
    explicit Maxwell3D(MemoryManager& memory_manager);
    ~Maxwell3D() = default;

    /// Write the value to the register identified by method.
    void WriteReg(u32 method, u32 value);

    /// Register structure of the Maxwell3D engine.
    /// TODO(Subv): This structure will need to be made bigger as more registers are discovered.
    struct Regs {
        static constexpr size_t NUM_REGS = 0xE36;

        enum class QueryMode : u32 {
            Write = 0,
            Sync = 1,
        };

        static constexpr size_t MaxShaderProgram = 6;
        enum class ShaderProgram : u32 {
            VertexA = 0,
            VertexB = 1,
            TesselationControl = 2,
            TesselationEval = 3,
            Geometry = 4,
            Fragment = 5,
        };

        enum class ShaderType : u32 {
            Vertex = 0,
            TesselationControl = 1,
            TesselationEval = 2,
            Geometry = 3,
            Fragment = 4,
        };

        union {
            struct {
                INSERT_PADDING_WORDS(0x582);
                struct {
                    u32 code_address_high;
                    u32 code_address_low;

                    GPUVAddr CodeAddress() const {
                        return static_cast<GPUVAddr>(
                            (static_cast<GPUVAddr>(code_address_high) << 32) | code_address_low);
                    }
                } code_address;
                INSERT_PADDING_WORDS(1);
                struct {
                    u32 vertex_end_gl;
                    u32 vertex_begin_gl;
                } draw;
                INSERT_PADDING_WORDS(0x139);
                struct {
                    u32 query_address_high;
                    u32 query_address_low;
                    u32 query_sequence;
                    union {
                        u32 raw;
                        BitField<0, 2, QueryMode> mode;
                        BitField<4, 1, u32> fence;
                        BitField<12, 4, u32> unit;
                    } query_get;

                    GPUVAddr QueryAddress() const {
                        return static_cast<GPUVAddr>(
                            (static_cast<GPUVAddr>(query_address_high) << 32) | query_address_low);
                    }
                } query;

                INSERT_PADDING_WORDS(0x13C);

                struct {
                    union {
                        BitField<0, 1, u32> enable;
                        BitField<4, 4, ShaderProgram> program;
                    };
                    u32 start_id;
                    INSERT_PADDING_WORDS(1);
                    u32 gpr_alloc;
                    ShaderType type;
                    INSERT_PADDING_WORDS(9);
                } shader_config[6];

                INSERT_PADDING_WORDS(0x5D0);

                struct {
                    u32 shader_code_call;
                    u32 shader_code_args;
                } shader_code;
                INSERT_PADDING_WORDS(0x10);
            };
            std::array<u32, NUM_REGS> reg_array;
        };
    } regs{};

    static_assert(sizeof(Regs) == Regs::NUM_REGS * sizeof(u32), "Maxwell3D Regs has wrong size");

private:
    /// Handles a write to the QUERY_GET register.
    void ProcessQueryGet();

    /// Handles a write to the VERTEX_END_GL register, triggering a draw.
    void DrawArrays();

    MemoryManager& memory_manager;
};

#define ASSERT_REG_POSITION(field_name, position)                                                  \
    static_assert(offsetof(Maxwell3D::Regs, field_name) == position * 4,                           \
                  "Field " #field_name " has invalid position")

ASSERT_REG_POSITION(code_address, 0x582);
ASSERT_REG_POSITION(draw, 0x585);
ASSERT_REG_POSITION(query, 0x6C0);
ASSERT_REG_POSITION(shader_config[0], 0x800);
ASSERT_REG_POSITION(shader_code, 0xE24);

#undef ASSERT_REG_POSITION

} // namespace Engines
} // namespace Tegra