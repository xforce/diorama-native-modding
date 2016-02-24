#pragma once

#include <memory>

namespace hooking
{
    static ptrdiff_t baseAddressDifference;

    namespace helpers
    {
        inline void set_base(uintptr_t address)
        {
            uintptr_t addressDiff = (address - 0x400000);
            baseAddressDifference = *(ptrdiff_t*)&addressDiff;
        }

        template<typename T>
        inline uintptr_t get_adjusted(T address)
        {
            return (uintptr_t)address + baseAddressDifference;
        }

        template<typename T>
        inline void adjust_base(T& address)
        {
            *(uintptr_t*)&address += baseAddressDifference;
        }

        template<typename ValueType, typename AddressType>
        inline void put(AddressType address, ValueType value)
        {
            DWORD oldProtect;
            VirtualProtect((LPVOID)address, sizeof(value), PAGE_EXECUTE_READWRITE, &oldProtect);
            memcpy((void*)address, &value, sizeof(value));
            VirtualProtect((LPVOID)address, sizeof(value), oldProtect, &oldProtect);
        }

    }

    template<typename T>
    inline void jump(uintptr_t address, T func)
    {
        helpers::put<uint8_t>(address, 0xE9);
        helpers::put<int>(address + 1, (intptr_t)func - (intptr_t)address - 5);
    }

    template<typename T>
    inline void call(uintptr_t address, T func)
    {
        helpers::put<uint8_t>(address, 0xE8);
        helpers::put<int>(address + 1, (intptr_t)func - (intptr_t)address - 5);
    }

	template<typename T>
	inline void nop(T address, size_t number)
	{
		DWORD oldProtect;
		VirtualProtect((LPVOID)address, number, PAGE_EXECUTE_READWRITE, &oldProtect);
		memset((void*)address, 0x90, number);
		VirtualProtect((LPVOID)address, number, oldProtect, &oldProtect);
	}
}