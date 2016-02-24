#include "peloader.h"
#include <iostream>
#include "hooking/hooking.h"
#include "hooking/hooking_patterns.h"
#include "vector.h"
#include "matrix.h"

int32_t dio_NetworkingClient_Update_Call = 0;
int32_t dio_ToServerPlayerStateEvent_ctor_Call = 0;

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

    struct Event
    {
        virtual ~Event() = 0 {};
        Event() = default;
        Event(EventIds eventId)
            : eventId_(eventId)
        {

        }
        EventIds eventId_;

        void * operator new(size_t size)
        {
            hooking::Pattern pat_malloc_call("55 8B EC EB 1F ");
            auto memory = ((void*(__cdecl *) (size_t))(pat_malloc_call.Get(0).Address()))(size);
            ZeroMemory(memory, size);
            return memory;
        }
    };
#pragma pack(push, 1)
    struct ToServerPlayerStateEvent : public Event
    {
        virtual ~ToServerPlayerStateEvent() {};
        ToServerPlayerStateEvent()
        {
            hooking::Pattern pat("55 8B EC F3 0F 7E 45 ? 8B 45 10 66 0F D6 41 ?");
            ((dio::ToServerPlayerStateEvent*(__thiscall *) (ToServerPlayerStateEvent*, __int64, int, __int64, int, __int64, int, char, char, int, char))(pat.Get(0).Address()))(this, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        }

        math::Vector3 chunk_id_;
        math::Vector3f xyz_;
        math::Vector3f ypr_;
        bool was_left_mouse_clicked_;
        bool was_right_mouse_clicked_;
        int block_id_;
        bool is_crouching_;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct ToServerChatEvent : public Event
    {
        std::string text_;

        virtual ~ToServerChatEvent() = default;
        ToServerChatEvent()
        {
            hooking::Pattern pat("55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 08 56 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F1 89 75 F0 89 75 EC C7 45 ? ? ? ? ? C6 46 04 05");
            ((dio::ToServerChatEvent*(__thiscall *) (ToServerChatEvent*, int, int, int, int, int, unsigned int))(pat.Get(0).Address()))(this, 0, 0, 0, 0, 0, 0);
        }
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct NetworkingClient
    {
        // +1E = rooms?
        char pad[0x40];  // 000 - 048 // This contains the player name
        uint32_t _state; // 048 - 04C

        virtual ~NetworkingClient() = 0;
        virtual void Function001() = 0;
        virtual void SendEventToServer(Event*) = 0;

        void SendChatMessage(std::string message)
        {
            auto chatEvent = new ToServerChatEvent();
            chatEvent->text_ = message;
            SendEventToServer(chatEvent);
        }

        void SendDestroySelectedBlock()
        {
            auto event = new ToServerPlayerStateEvent();
            //
            event->was_left_mouse_clicked_ = true;
            
            SendEventToServer(event);
        }

#pragma warning(disable : 4100)
        static int __thiscall ToServerChat(int this_, std::string text)
        {
            // &a2 = string content
            //auto text = std::string((const char*)a2, length);
            //__debugbreak();
            return 0;
        }

        static int* __thiscall Update(dio::NetworkingClient* client, int a2)
        {
            math::Vector3f bpos;
            auto v12 = *(DWORD *)((char*)client + 96);
            if (v12)
            {
                auto v13 = *(DWORD *)((char*)v12 + 32);
                if (v13)
                {
                    bpos = *(math::Vector3f *)((char*)v13 + 32);
                }
            }

            auto result = ((int*(__thiscall *) (NetworkingClient*, int))(dio_NetworkingClient_Update_Call))(client, a2);

            if (v12)
            {
                auto v13 = *(DWORD *)((char*)v12 + 32);
                if (v13)
                {
                    auto pos = *(math::Vector3f *)((char*)v13 + 32);
                    auto ypr = *(math::Vector3f *)((char*)v13 + 44);
                    //pos._y = bpos._y; // So we don't fall


                    *(math::Vector3f *)(v13 + 32) = pos;
                    //*(math::Vector3f *)(v13 + 44) = { 0.425204098f, -3.14000988f,0.000000000 };

                    //client->SendDestroySelectedBlock();
                    //client->SendChatMessage("Hey guys!!!");
                }
            }

            return result;
        }
    };
#pragma pack(pop)
}

int main()
{
    PELoader peLoader;
    peLoader.LoadFile("client_release_vX_X_X.exe");

    hooking::SetImageBase((uintptr_t)peLoader.GetImageBase());
    hooking::helpers::set_base((uintptr_t)peLoader.GetImageBase());
    hooking::Pattern pat_dio_NetworkingClient_Update_Remote("E8 ? ? ? ? 8B 4E 10 8B 41 48");
    hooking::Pattern pat_dio_NetworkingClient_Update_Local("E8 ? ? ? ? 85 FF 74 73");
    hooking::Pattern pat_dio_ToServerPlayerStateEvent_ctor("E8 ? ? ? ? 8B 4E 10 50");
    hooking::Pattern pat_malloc_call("74 73 6A 38");
    hooking::Pattern pat_malloc_call2("E8 ? ? ? ? 8B 4D CC C6 45 FC 00");
    
    auto dio_NetworkingClient_Update_Remote_Address = pat_dio_NetworkingClient_Update_Remote.Get(0).Address();
    dio_NetworkingClient_Update_Call = *(int32_t*)(dio_NetworkingClient_Update_Remote_Address + 1) + dio_NetworkingClient_Update_Remote_Address + 5;

    auto dio_ToServerPlayerStateEvent_ctor_Address = pat_dio_ToServerPlayerStateEvent_ctor.Get(0).Address();
    dio_ToServerPlayerStateEvent_ctor_Call = *(int32_t*)(dio_ToServerPlayerStateEvent_ctor_Address + 1) + dio_ToServerPlayerStateEvent_ctor_Address + 5;

    hooking::call(dio_NetworkingClient_Update_Remote_Address, &dio::NetworkingClient::Update);
    hooking::call(pat_dio_NetworkingClient_Update_Local.Get(0).Address(), &dio::NetworkingClient::Update);

    //hooking::call(pat_malloc_call2.Get(0).Address(), &dio::NetworkingClient::ToServerChat);
    //hooking::call(dio_ToServerPlayerStateEvent_ctor_Address, &dio::ToServerPlayerStateEvent::ctor);

    peLoader.Run();
}