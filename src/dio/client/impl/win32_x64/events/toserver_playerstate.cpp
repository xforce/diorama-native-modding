#include <events/toserver_playerstate.h>

#include <hooking_patterns.h>

#include <dio_std_types.h>

namespace dio
{
    ToServerPlayerStateEvent::ToServerPlayerStateEvent()
    {
        hooking::Pattern pat("55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 08 56 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F1 89 75 F0 89 75 EC C7 45 ? ? ? ? ? C6 46 04 03");
        ((dio::ToServerPlayerStateEvent*(__thiscall *) (ToServerPlayerStateEvent*, __int64, int, __int64, int, __int64, int, char, char, int, char, string))(pat.Get(0).Address()))(this, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, string{});
    }
}