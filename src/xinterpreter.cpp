/***************************************************************************
* Copyright (c) 2026, Johan Mabille                                        *
* Copyright (c) 2026, QuantStack                                           *
* Copyright (c) 2026, 12 Monkeys                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <string>
#include <vector>
#include <iostream>

#include "nlohmann/json.hpp"

#include "xeus/xinput.hpp"
#include "xeus/xinterpreter.hpp"
#include "xeus/xhelper.hpp"

#include "xeus-brainfuck/xinterpreter.hpp"

namespace nl = nlohmann;

namespace xbf
{
 
    interpreter::interpreter(flavor fl)
        : m_execution_context()
        , m_flavor(fl)
    {
        xeus::register_interpreter(this);
        m_execution_context.input_handler = []() -> char
        {
            std::string res = xeus::blocking_input_request("", false);
            return res[0];
        };
        m_output_handler = [this](char c)
        {
            publish_stream("stdout", std::string(1u, c));
        };
        m_execution_context.output_handler = m_output_handler;
    }

    void interpreter::execute_request_impl(send_reply_callback cb, // Callback to send the result
                                  int /*execution_counter*/, // Typically the cell number
                                  const std::string& code, // Code to execute
                                  xeus::execute_request_config config,
                                  nl::json /*user_expressions*/) 
    {
        if (config.silent)
        {
            m_execution_context.output_handler = [](char) {};
        }
        execute_code(m_execution_context, code, m_flavor);
        cb(xeus::create_successful_reply());

        if (config.silent)
        {
            m_execution_context.output_handler = m_output_handler;
        }
    }

    void interpreter::configure_impl()
    {
    }

    nl::json interpreter::is_complete_request_impl(const std::string& /*code*/)
    {
        return xeus::create_is_complete_reply("complete", "   ");
    }

    nl::json interpreter::complete_request_impl(const std::string& /*code*/,
                                                int cursor_pos)
    {
        return xeus::create_complete_reply(
            nl::json::array(),  /*matches*/
            cursor_pos,         /*cursor_start*/
            cursor_pos          /*cursor_end*/
            );
    }

    nl::json interpreter::inspect_request_impl(const std::string& /*code*/,
                                               int /*cursor_pos*/,
                                               int /*detail_level*/)
    {
        return xeus::create_inspect_reply(true, 
            {{std::string("text/plain"), std::string("Hey, what did you inspect?")}}
        );
    }

    nl::json interpreter::shutdown_request_impl(bool /*restart*/)
    {
        return xeus::create_shutdown_reply(false);
    }

    nl::json interpreter::interrupt_request_impl()
    {
        return xeus::create_interrupt_reply();
    }

    nl::json interpreter::kernel_info_request_impl()
    {
        if (m_flavor == xbf::flavor::vanilla)
        {
            std::vector<std::string> supported_features = {};
            return xeus::create_info_reply(
                "xbrainfuck",
                XEUS_BRAINFUCK_VERSION,
                "Brainfuck",
                "1.0.0",
                "text/x-brainfuck",
                "bf",
                "",
                std::string(""),
                "",
                "xbrainfuck",
                nl::json::array(),
                supported_features
            );
        }
        else
        {
            std::vector<std::string> supported_features = {};
            return xeus::create_info_reply(
                "xbrainfuck",
                XEUS_BRAINFUCK_VERSION,
                "Ook",
                "1.0.0",
                "text/x-ook",
                "ook",
                "",
                std::string(""),
                "",
                "xook",
                nl::json::array(),
                supported_features
            );
        }
    }

}
