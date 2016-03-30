#include <events/event.h>
#include <hooking_patterns.h>

namespace dio
{
    void * Event::operator new(size_t size)
    {
        hooking::Pattern pat_malloc_call("55 8B EC EB 1F ");
        auto memory = ((void*(__cdecl *) (size_t))(pat_malloc_call.Get(0).Address()))(size);
        memset(memory, 0, size);
        return memory;
    }

    void Event::operator delete(void* ptr)
    {
        hooking::Pattern pat_malloc_call("55 8B EC FF 75 08 E8 ? ? ? ? 59 5D C3 55 8B EC F6 45 08 01");
        ((void(__cdecl *) (void*))(pat_malloc_call.Get(0).Address()))(ptr);
    }
}