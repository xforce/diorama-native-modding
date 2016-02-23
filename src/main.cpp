#include "peloader.h"
#include <iostream>
#include "hooking/hooking.h"
#include "hooking/hooking_patterns.h"

int32_t ProcessMaybeCall = 0;

struct Xyz
{
    float x;
    float y;
    float z;
};

struct NetworkingClient
{
    static int* __thiscall ProcessMaybe(int _this, int a2)
    {
        a2 = a2;
        _this = _this;
        
        auto result = ((int*(__thiscall *) (int, int))(ProcessMaybeCall))(_this, a2);
        auto v12 = *(DWORD *)(_this + 96);
        if (v12)
        {
            auto v13 = *(DWORD *)(v12 + 32);
            if (v13)
            {
                // v13 = PlayerSimState?
                auto pos = *(Xyz *)(v13 + 32);
                pos = pos;
                pos.y = 70; // Yay flying over spawn
                *(Xyz *)(v13 + 32) = pos;
            }
        }
        
        return result;
    }
};

int main()
{
    PELoader peLoader;
    peLoader.LoadFile("client_release_vX_X_X.exe");

    hooking::SetImageBase((uintptr_t)peLoader.GetImageBase());
    hooking::helpers::set_base((uintptr_t)peLoader.GetImageBase());
    hooking::Pattern pat("E8 ? ? ? ? 8B 4E 10 8B 41 48 ");

    auto address = pat.Get(0).Address();
    ProcessMaybeCall = *(int32_t*)(address + 1) + address + 5;
    hooking::call(address, &NetworkingClient::ProcessMaybe);

    peLoader.Run();
}