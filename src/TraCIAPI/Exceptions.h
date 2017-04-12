#pragma once
#include <stdexcept>

namespace traci_api
{
    class NotImplementedError : public std::runtime_error
    {
    public:
        explicit NotImplementedError(const std::string& _Message)
            : runtime_error(_Message)
        {
        }

        explicit NotImplementedError(const char* _Message)
            : runtime_error(_Message)
        {
        }
    };

    class NoSuchVHCError : public std::runtime_error
    {
    public:
        explicit NoSuchVHCError(const std::string& _Message)
            : runtime_error(_Message)
        {
        }

        explicit NoSuchVHCError(const char* _Message)
            : runtime_error(_Message)
        {
        }
    };

    class NoSuchLNKError : public std::runtime_error
    {
    public:
        explicit NoSuchLNKError(const std::string& _Message)
            : runtime_error(_Message)
        {
        }

        explicit NoSuchLNKError(const char* _Message)
            : runtime_error(_Message)
        {
        }
    };

    class NoSuchNDEError : public std::runtime_error
    {
    public:
        explicit NoSuchNDEError(const std::string& _Message)
            : runtime_error(_Message)
        {
        }

        explicit NoSuchNDEError(const char* _Message)
            : runtime_error(_Message)
        {
        }
    };
}
