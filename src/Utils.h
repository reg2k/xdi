#pragma once

namespace Utils {
    bool ReadMemory(uintptr_t addr, void* data, size_t len);

    template <typename T>
    T GetVirtualFunction(void* baseObject, int vtblIndex) {
        uintptr_t* vtbl = reinterpret_cast<uintptr_t**>(baseObject)[0];
        return reinterpret_cast<T>(vtbl[vtblIndex]);
    }

    template <typename T>
    T GetOffset(const void* baseObject, int offset) {
        return *reinterpret_cast<T*>((uintptr_t)baseObject + offset);
    }

    template<typename T>
    T* GetOffsetPtr(const void * baseObject, int offset)
    {
        return reinterpret_cast<T*>((uintptr_t)baseObject + offset);
    }

    template <typename T>
    T Hook(T hook, uintptr_t hookTarget, int offset = 1, int instructionLength = 5) {
        SInt32 rel32 = 0;
        ReadMemory(hookTarget + offset, &rel32, sizeof(UInt32));
        g_branchTrampoline.Write5Call(hookTarget, (uintptr_t)hook);
        return reinterpret_cast<T>(hookTarget + instructionLength + rel32); // return original address
    }
}