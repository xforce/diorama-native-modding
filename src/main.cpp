#include "peloader.h"
#include <iostream>
#include "hooking/hooking.h"
#include "hooking/hooking_patterns.h"
#include "vector.h"
#include "matrix.h"

int32_t dio_NetworkingClient_Update_Call = 0;

struct NetworkingClient
{
    static int* __thiscall Update(int _this, int a2)
    {
        math::Vector3f bpos;
        auto v12 = *(DWORD *)(_this + 96);
        if (v12)
        {
            auto v13 = *(DWORD *)(v12 + 32);
            if (v13)
            {
                bpos = *(math::Vector3f *)(v13 + 32);
            }
        }

        auto result = ((int*(__thiscall *) (int, int))(dio_NetworkingClient_Update_Call))(_this, a2);
        
        if (v12)
        {
            auto v13 = *(DWORD *)(v12 + 32);
            if (v13)
            {
                auto pos = *(math::Vector3f *)(v13 + 32);
                pos._y = bpos._y; // So we don't fall

                auto ypr = *(math::Vector3f *)(v13 + 44);

                *(math::Vector3f *)(v13 + 32) = pos;
                *(math::Vector3f *)(v13 + 44) = ypr;
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
    hooking::Pattern pat_dio_NetworkingClient_Update_Remote("E8 ? ? ? ? 8B 4E 10 8B 41 48");
    hooking::Pattern pat_dio_NetworkingClient_Update_Local("E8 ? ? ? ? 85 FF 74 73");
    

    auto pat_dio_NetworkingClient_Update_Remote_Address = pat_dio_NetworkingClient_Update_Remote.Get(0).Address();
    dio_NetworkingClient_Update_Call = *(int32_t*)(pat_dio_NetworkingClient_Update_Remote_Address + 1) + pat_dio_NetworkingClient_Update_Remote_Address + 5;
    hooking::call(pat_dio_NetworkingClient_Update_Remote_Address, &NetworkingClient::Update);
    hooking::call(pat_dio_NetworkingClient_Update_Local.Get(0).Address(), &NetworkingClient::Update);

    peLoader.Run();
}