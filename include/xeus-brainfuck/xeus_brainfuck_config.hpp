/***************************************************************************
* Copyright (c) 2026, Johan Mabille                                        *
* Copyright (c) 2026, QuantStack                                           *
* Copyright (c) 2026, 12 Monkeys                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_BRAINFUCK_CONFIG_HPP
#define XEUS_BRAINFUCK_CONFIG_HPP

// Project version
#define XEUS_BRAINFUCK_VERSION_MAJOR 0
#define XEUS_BRAINFUCK_VERSION_MINOR 1
#define XEUS_BRAINFUCK_VERSION_PATCH 0

// Composing the version string from major, minor and patch
#define XEUS_BRAINFUCK_CONCATENATE(A, B) XEUS_BRAINFUCK_CONCATENATE_IMPL(A, B)
#define XEUS_BRAINFUCK_CONCATENATE_IMPL(A, B) A##B
#define XEUS_BRAINFUCK_STRINGIFY(a) XEUS_BRAINFUCK_STRINGIFY_IMPL(a)
#define XEUS_BRAINFUCK_STRINGIFY_IMPL(a) #a

#define XEUS_BRAINFUCK_VERSION XEUS_BRAINFUCK_STRINGIFY(XEUS_BRAINFUCK_CONCATENATE(XEUS_BRAINFUCK_VERSION_MAJOR,   \
                 XEUS_BRAINFUCK_CONCATENATE(.,XEUS_BRAINFUCK_CONCATENATE(XEUS_BRAINFUCK_VERSION_MINOR,   \
                                  XEUS_BRAINFUCK_CONCATENATE(.,XEUS_BRAINFUCK_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_BRAINFUCK_EXPORTS
        #define XEUS_BRAINFUCK_API __declspec(dllexport)
    #else
        #define XEUS_BRAINFUCK_API __declspec(dllimport)
    #endif
#else
    #define XEUS_BRAINFUCK_API
#endif

#endif
