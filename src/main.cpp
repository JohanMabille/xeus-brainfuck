/***************************************************************************
* Copyright (c) 2026, Johan Mabille                                        *
* Copyright (c) 2026, QuantStack                                           *
* Copyright (c) 2026, 12 Monkeys                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

#ifdef __GLIBC__
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "xeus/xeus_context.hpp"
#include "xeus/xhelper.hpp"
#include "xeus/xhistory_manager.hpp"
#include "xeus/xkernel.hpp"
#include "xeus/xkernel_configuration.hpp"
#include "xeus/xserver.hpp"

#include "xeus-zmq/xserver_zmq_split.hpp"
#include "xeus-zmq/xzmq_context.hpp"


#include "xeus-brainfuck/xinterpreter.hpp"
#include "xeus-brainfuck/xeus_brainfuck_config.hpp"

#ifdef __GLIBC__
void handler(int sig)
{
    void* array[10];

    // get void*'s for all entries on the stack
    size_t size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif

xbf::flavor get_flavor(const char* flavor_arg)
{
        std::cout << "Flavor arg = " << flavor_arg << std::endl; 
    if (!strcmp(flavor_arg, "vanilla"))
    {
        std::cout << "instantiating vanilla" << std::endl;
        return xbf::flavor::vanilla;
    }
    else
    {
        std::cout << "instantiating ook" << std::endl;
        return xbf::flavor::ook;
    }
}
int main(int argc, char* argv[])
{
    if (xeus::should_print_version(argc, argv))
    {
        std::clog << "xbrainfuck " << XEUS_BRAINFUCK_VERSION  << std::endl;
        return 0;
    }

    // If we are called from the Jupyter launcher, silence all logging. This
    // is important for a JupyterHub configured with cleanup_servers = False:
    // Upon restart, spawned single-user servers keep running but without the
    // std* streams. When a user then tries to start a new kernel, xbrainfuck
    // will get a SIGPIPE and exit.
    if (std::getenv("JPY_PARENT_PID") != NULL)
    {
        std::clog.setstate(std::ios_base::failbit);
    }

    // Registering SIGSEGV handler
#ifdef __GLIBC__
    std::clog << "registering handler for SIGSEGV" << std::endl;
    signal(SIGSEGV, handler);
#endif

    std::unique_ptr<xeus::xcontext> context = xeus::make_zmq_context();

    xbf::flavor fl = get_flavor(argv[4]);
    //
    // Instantiating the xeus xinterpreter
    using interpreter_ptr = std::unique_ptr<xbf::interpreter>;
    interpreter_ptr interpreter = interpreter_ptr(new xbf::interpreter(fl));

    using history_manager_ptr = std::unique_ptr<xeus::xhistory_manager>;
    history_manager_ptr hist = xeus::make_in_memory_history_manager();

    std::string connection_filename = xeus::extract_filename(argc, argv);

    if (!connection_filename.empty())
    {

        xeus::xconfiguration config = xeus::load_configuration(connection_filename);
        xeus::xkernel kernel(config,
                             xeus::get_user_name(),
                             std::move(context),
                             std::move(interpreter),
                             xeus::make_xserver_shell_main,
                             std::move(hist),
                             xeus::make_console_logger(xeus::xlogger::msg_type,
                                                       xeus::make_file_logger(xeus::xlogger::content, "xeus.log")));

        std::cout <<
            "Starting xbrainfuck kernel...\n\n"
            "If you want to connect to this kernel from an other client, you can use"
            " the " + connection_filename + " file."
            << std::endl;

        kernel.start();
    }
    else
    {
        xeus::xkernel kernel(xeus::get_user_name(),
                             std::move(context),
                             std::move(interpreter),
                             xeus::make_xserver_shell_main,
                             std::move(hist),
                             nullptr);

        const auto& config = kernel.get_config();
        std::cout <<
            "Starting xbrainfuck kernel...\n\n"
            "If you want to connect to this kernel from an other client, just copy"
            " and paste the following content inside of a `kernel.json` file. And then run for example:\n\n"
            "# jupyter console --existing kernel.json\n\n"
            "kernel.json\n```\n{\n"
            "    \"transport\": \"" + config.m_transport + "\",\n"
            "    \"ip\": \"" + config.m_ip + "\",\n"
            "    \"control_port\": " + config.m_control_port + ",\n"
            "    \"shell_port\": " + config.m_shell_port + ",\n"
            "    \"stdin_port\": " + config.m_stdin_port + ",\n"
            "    \"iopub_port\": " + config.m_iopub_port + ",\n"
            "    \"hb_port\": " + config.m_hb_port + ",\n"
            "    \"signature_scheme\": \"" + config.m_signature_scheme + "\",\n"
            "    \"key\": \"" + config.m_key + "\"\n"
            "}\n```"
            << std::endl;

        kernel.start();
    }

    return 0;
}
