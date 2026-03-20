/***************************************************************************
* Copyright (c) 2026, Johan Mabille                                        *
* Copyright (c) 2026, QuantStack                                           *
* Copyright (c) 2026, 12 Monkeys                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <fstream>
#include <map>
#include <stack>
#include <string_view>

#include "xeus-brainfuck/xbrainfuck.hpp"

namespace xbf
{
    namespace
    {
        enum class instruction
        {
            plus,
            minus,
            previous,
            next,
            output,
            input,
            loop_start,
            loop_end,
            ignore
        };

        template <flavor f>
        struct bf_parser;

        template <>
        struct bf_parser<flavor::vanilla>
        {
            instruction get_instruction(const std::string& code, std::size_t i) const
            {
                switch(code[i])
                {
                case '+':
                    return instruction::plus;
                case '-':
                    return instruction::minus;
                case '<':
                    return instruction::previous;
                case '>':
                    return instruction::next;
                case '.':
                    return instruction::output;
                case ',':
                    return instruction::input;
                case '[':
                    return instruction::loop_start;
                case ']':
                    return instruction::loop_end;
                default:
                    return instruction::ignore;
                }
            }

            std::size_t next_index(std::size_t i) const
            {
                return ++i;
            }
        };

        template <>
        struct bf_parser<flavor::ook>
        {
            instruction get_instruction(const std::string& code, std::size_t i) const
            {
                if (i + TOKEN_SIZE >= code.size())
                {
                    return instruction::ignore;
                }

                auto token = std::string_view(code.data() + i, TOKEN_SIZE-1);
                if (token == std::string_view("Ook. Ook."))
                {
                    return instruction::plus;
                }
                else if (token == std::string_view("Ook! Ook!"))
                {
                    return instruction::minus;
                }
                else if (token == std::string_view("Ook? Ook."))
                {
                    return instruction::previous;
                }
                else if (token == std::string_view("Ook. Ook?"))
                {
                    return instruction::next;
                }
                else if (token == std::string_view("Ook! Ook."))
                {
                    return instruction::output;
                }
                else if (token == std::string_view("Ook. Ook!"))
                {
                    return instruction::input;
                }
                else if (token == std::string_view("Ook! Ook?"))
                {
                    return instruction::loop_start;
                }
                else if (token == std::string_view("Ook? Ook!"))
                {
                    return instruction::loop_end;
                }
                else
                {
                    return instruction::ignore;
                }
            }

            std::size_t next_index(std::size_t i) const
            {
                return i + TOKEN_SIZE;
            }

        private:

            static constexpr std::size_t TOKEN_SIZE = 10u;
        };

        struct matching_brackets
        {
            using matching_map_t = std::map<std::size_t, std::size_t>;
            matching_map_t forward;
            matching_map_t backward;
        };

        template <flavor fl>
        matching_brackets get_matching_brackets(const std::string& code)
        {
            std::stack<std::size_t> stack;
            bf_parser<fl> parser;
            matching_brackets res;

            for (std::size_t i = 0; i < code.size(); i = parser.next_index(i))
            {
                instruction inst = parser.get_instruction(code, i);
                if (inst == instruction::loop_start)
                {
                    stack.push(i);
                }
                else if (inst == instruction::loop_end)
                {
                    std::size_t j = stack.top();
                    res.backward[i] = j;
                    res.forward[j] = i;
                    stack.pop();
                }
            }
            return res;
        }
    }

    template <flavor fl>
    void execute_code_impl(execution_context& ctx, const std::string& code)
    {
        matching_brackets brackets = get_matching_brackets<fl>(code);

        std::size_t inst_idx = 0;
        std::size_t& data_idx = ctx.tape_index;

        bf_parser<fl> parser;

        constexpr int MOD = 256;

        while (inst_idx < code.size())
        {
            instruction inst = parser.get_instruction(code, inst_idx);
            switch(inst)
            {
            case instruction::plus:
                ctx.tape[data_idx] = (ctx.tape[data_idx] + 1) % MOD;
                inst_idx = parser.next_index(inst_idx);
                break;
            case instruction::minus:
                ctx.tape[data_idx] = (ctx.tape[data_idx] - 1) % MOD;
                inst_idx = parser.next_index(inst_idx);
                break;
            case instruction::previous:
                --data_idx;
                inst_idx = parser.next_index(inst_idx);
                break;
            case instruction::next:
                ++data_idx;
                inst_idx = parser.next_index(inst_idx);
                break;
            case instruction::output:
                ctx.output_handler(ctx.tape[data_idx]);
                inst_idx = parser.next_index(inst_idx);
                break;
            case instruction::input:
                ctx.tape[data_idx] = ctx.input_handler();
                inst_idx = parser.next_index(inst_idx);
                break;
            case instruction::loop_start:
            {
                std::size_t j = brackets.forward[inst_idx];
                inst_idx = ctx.tape[data_idx] ? parser.next_index(inst_idx) : parser.next_index(j);
                break;
            }
            case instruction::loop_end:
            {
                std::size_t j = brackets.backward[inst_idx];
                inst_idx = ctx.tape[data_idx] ? parser.next_index(j) : parser.next_index(inst_idx);
                break;
            }
            case instruction::ignore:
            default:
                inst_idx = parser.next_index(inst_idx);
                break;
            }
        }
    }

    execution_context::execution_context()
        : tape(30000)
        , tape_index(0u)
    {
    }

    void execute_code(execution_context& ctx, const std::string& code, flavor fl)
    {
        if (fl == flavor::vanilla)
        {
            execute_code_impl<flavor::vanilla>(ctx, code);
        }
        else
        {
            execute_code_impl<flavor::ook>(ctx, code);
        }
    }

    // Utility tool to convert from one brainfuck flavor to another one.

    namespace
    {
        std::string vanilla_to_ook(const std::string& code)
        {
            std::size_t size = code.size();
            std::string res;
            for (std::size_t i = 0; i < size; ++i)
            {
                switch(code[i])
                {
                case '+':
                    res.append("Ook. Ook. ");
                    break;
                case '-':
                    res.append("Ook! Ook! ");
                    break;
                case '<':
                    res.append("Ook? Ook. ");
                    break;
                case '>':
                    res.append("Ook. Ook? ");
                    break;
                case '.':
                    res.append("Ook! Ook. ");
                    break;
                case ',':
                    res.append("Ook. Ook! ");
                    break;
                case '[':
                    res.append("Ook! Ook? ");
                    break;
                case ']':
                    res.append("Ook? Ook! ");
                    break;
                default:
                    break;
                }
            }
            return res;
        }
    }

    std::string convert_code(const std::string& code, flavor input_flavor, flavor /*output_flavor*/)
    {
        if (input_flavor == flavor::vanilla)
        {
            return vanilla_to_ook(code);
        }
        else
        {
            return std::string("not implemented");
        }
    }
}
