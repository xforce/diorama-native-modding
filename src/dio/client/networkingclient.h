#pragma once

#include "events/event.h"

#include <dio_std_types.h>

#include <cstdint>

namespace dio
{
    struct NetworkingClient
    {
        // +1E = rooms?
        char pad[0x40];  // 000 - 048 // This contains the player name
        uint32_t _state; // 048 - 04C

        virtual ~NetworkingClient() = 0;
        virtual void Function001() = 0;
        virtual void SendEventToServer(Event*) = 0;

        // Not part of diorama, but makes it easier at the moment to do shit
        void SendChatMessage(string message);

        void SendDestroySelectedBlock();
    };
}