/***************************************************************************
* Copyright (c) 2026, Johan Mabille                                        *
* Copyright (c) 2026, QuantStack                                           *
* Copyright (c) 2026, 12 Monkeys                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "xeus_brainfuck_config.hpp"

namespace xbf
{
    
    enum class flavor
    {
        vanilla,
        ook,
    };

    struct XEUS_BRAINFUCK_API execution_context
    {
        using output_handler_t = std::function<void(char)>;
        using input_handler_t = std::function<char()>;

        execution_context();

        std::vector<char> tape;
        std::size_t tape_index;

        output_handler_t output_handler;
        input_handler_t input_handler;
    };

    XEUS_BRAINFUCK_API
    void execute_code(execution_context& ctx, const std::string& code, flavor fl = flavor::vanilla);

    XEUS_BRAINFUCK_API
    std::string convert_code(const std::string& code, flavor input_flavor, flavor output_flavor);
}

