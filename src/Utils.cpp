#include "Utils.h"

namespace Utils {
    bool ReadMemory(uintptr_t addr, void* data, size_t len) {
        UInt32 oldProtect;
        if (VirtualProtect((void *)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(data, (void*)addr, len);
            if (VirtualProtect((void *)addr, len, oldProtect, &oldProtect))
                return true;
        }
        return false;
    }
}