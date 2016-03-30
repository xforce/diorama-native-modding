#pragma once

#include <cstdint>
#include <cstring>

namespace dio
{

    template <typename T>
    struct mallocator {
        using value_type = T;

        mallocator() = default;
        template <class U>
        mallocator(const mallocator<U>&) {}

        T* allocate(std::size_t n)
        {
            return (T*)(mallocator<void>::alloc_internal(n));
        }
        void deallocate(T* ptr, std::size_t n)
        {
            mallocator<void>::dealloc_internal(ptr, n);
        }


    public:
        static void * alloc_internal(std::size_t n);
        static void dealloc_internal(void *, std::size_t);
    };

    template <typename T, typename U>
    inline bool operator == (const mallocator<T>&, const mallocator<U>&) {
        return true;
    }

    template <typename T, typename U>
    inline bool operator != (const mallocator<T>& a, const mallocator<U>& b) {
        return !(a == b);
    }
    
}