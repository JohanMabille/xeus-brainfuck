/***************************************************************************
* Copyright (c) 2026, Johan Mabille                                        *
* Copyright (c) 2026, QuantStack                                           *
* Copyright (c) 2026, 12 Monkeys                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_BRAINFUCK_INTERPRETER_HPP
#define XEUS_BRAINFUCK_INTERPRETER_HPP

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

#include <string>
#include <memory>

#include "nlohmann/json.hpp"

#include "xbrainfuck.hpp"
#include "xeus_brainfuck_config.hpp"
#include "xeus/xinterpreter.hpp"


namespace nl = nlohmann;

namespace xbf
{
    class XEUS_BRAINFUCK_API interpreter : public xeus::xinterpreter
    {
    public:

        interpreter(flavor fl);
        virtual ~interpreter() = default;

    protected:

        void configure_impl() override;


        void execute_request_impl(send_reply_callback cb,
                                  int execution_counter,
                                  const std::string& code,
                                  xeus::execute_request_config config,
                                  nl::json user_expressions) override;

        nl::json complete_request_impl(const std::string& code, int cursor_pos) override;

        nl::json inspect_request_impl(const std::string& code,
                                      int cursor_pos,
                                      int detail_level) override;

        nl::json is_complete_request_impl(const std::string& code) override;

        nl::json kernel_info_request_impl() override;

        nl::json shutdown_request_impl(bool restart) override;

        nl::json interrupt_request_impl() override;

    private:

        execution_context m_execution_context;
        flavor m_flavor;
        std::function<void(char)> m_output_handler;
    };
}

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

#endif
