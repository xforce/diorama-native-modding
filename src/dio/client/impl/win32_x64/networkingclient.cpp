#pragma once

#include <networkingclient.h>
#include <dio_std_types.h>

#include <events/toserver_playerstate.h>
#include <events/toserver_chat.h>

namespace dio
{
    void NetworkingClient::SendChatMessage(string message)
    {
        auto chatEvent = new ToServerChatEvent();
        chatEvent->text_ = message;
        SendEventToServer(chatEvent);
    }

    void NetworkingClient::SendDestroySelectedBlock()
    {
        auto event = new ToServerPlayerStateEvent();
        //
        event->was_left_mouse_clicked_ = true;

        SendEventToServer(event);
    }

}