#pragma once

struct StaticFunctionTag;
class VirtualMachine;

namespace Papyrus
{
    bool RegisterPapyrus(VirtualMachine *vm);
    void RegisterFuncs(VirtualMachine* vm);
}
