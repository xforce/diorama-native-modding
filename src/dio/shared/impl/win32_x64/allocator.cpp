#include <allocator.h>
#include <hooking_patterns.h>

namespace dio
{
    template <>
    void mallocator<void>::dealloc_internal(void * ptr, std::size_t)
    {
        hooking::Pattern pat_malloc_call("55 8B EC FF 75 08 E8 ? ? ? ? 59 5D C3 55 8B EC F6 45 08 01");
        ((void(__cdecl *) (void*))(pat_malloc_call.Get(0).Address()))(ptr);
    }

    template <>
    void * dio::mallocator<void>::alloc_internal(std::size_t n)
    {
        hooking::Pattern pat_malloc_call("55 8B EC EB 1F");
        auto memory = ((void*(__cdecl *) (size_t))(pat_malloc_call.Get(0).Address()))(n);
        return memory;
    }
}

