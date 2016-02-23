#include "peloader.h"
#include <iostream>

int main()
{
    PELoader peLoader;
    peLoader.LoadFile("client_release_vX_X_X.exe");
    peLoader.Run();
}