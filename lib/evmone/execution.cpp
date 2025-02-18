// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2019-2020 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#include "execution.hpp"
#include "analysis.hpp"
#include <memory>

namespace evmone
{
evmc_result execute(evmc_vm* /*unused*/, const evmc_host_interface* host, evmc_host_context* ctx,
    evmc_revision rev, const evmc_message* msg, const uint8_t* code, size_t code_size) noexcept
{
    const auto analysis = analyze(rev, code, code_size);

    auto state =
        std::make_unique<execution_state>(*msg, rev, *host, ctx, code, code_size, analysis);

    const auto* instr = &state->analysis->instrs[0];
    while (instr != nullptr)
        instr = instr->fn(instr, *state);

    const auto gas_left =
        (state->status == EVMC_SUCCESS || state->status == EVMC_REVERT) ? state->gas_left : 0;

    evmc_result res_ptr = evmc::make_result(
        state->status, gas_left, &state->memory[state->output_offset], state->output_size);
    // save used_memory to evmc_result.padding
    const auto used_memory = state->memory.used_memory();
    memcpy(res_ptr.padding, &used_memory, sizeof(uint32_t));
    return res_ptr; 
}
}  // namespace evmone
