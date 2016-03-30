#include "peloader.h"
#include <iostream>

int main()
{
    PELoader peLoader;
#ifdef _AMD64_
    peLoader.LoadFile("client_Release_x64..exe");
#else
    peLoader.LoadFile("client_Release_x86..exe");
#endif
    peLoader.Run();
}