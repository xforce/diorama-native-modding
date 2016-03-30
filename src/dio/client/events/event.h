#pragma once

#include <cstdint>

namespace dio
{
    enum EventIds : uint8_t
    {
        TO_SERVER_PLAYER_JOINED = 0,
        TO_SERVER_PLAYER_HANDSHAKE_REQUEST = 1,
        TO_SERVER_PLAYER_INPUT_EVENT = 2,
        TO_SERVER_PLAYER_STATE_EVENT = 3,
        TO_SERVER_PLAYER_DISCONNECTED = 4,
        TO_SERVER_CHAT_EVENT = 5,
        TO_SERVER_DELIVERY_TIME_TEST = 6,
    };

// TODO(xforce): Pack macro, so we can use it on different compilers
    // We may need two macros, depends on how it works in clang/gcc

    struct Event
    {
        virtual ~Event() = 0 {};
        Event() = default;
        Event(EventIds eventId)
            : eventId_(eventId)
        {

        }
        EventIds eventId_;

        void * operator new(size_t size);
        void operator delete  (void* ptr);
    };
}