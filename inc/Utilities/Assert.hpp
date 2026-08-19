#pragma once

#include <string>
#include <iostream>
#include <exception>
#include <Utilities/Logger.hpp>

#ifndef NDEBUG

#define AERO_ASSERT(ag_cond, ag_msg)                                      \
    do                                                                   \
    {                                                                    \
        if (!(ag_cond))                                                  \
        {                                                                \
            std::ostringstream ss;                                       \
            ss << "Assertion failed: (" << #ag_cond << "), file "        \
               << __FILE__ << ", line " << __LINE__ << ": " << ag_msg;  \
                                                                         \
            Aero::Utilities::Logger::instance().log(                     \
                Aero::Utilities::Logger::Severity::ERROR, ss.str());     \
                                                                         \
            std::terminate();                                            \
        }                                                                \
    } while (false)

#else

#define AERO_ASSERT(ag_cond, ag_msg) \
    do                               \
    {                                \
        (void)sizeof(ag_cond);       \
    } while (false)

#endif
