#include "PCH.h"

namespace
{
    struct Prolog : Xbyak::CodeGenerator
    {
        Prolog()
        {
            // save rcx to the stack
            push(rcx);
            // save rax to the stack
            push(rax);
        }
    };

    struct Epilog : Xbyak::CodeGenerator
    {
        Epilog()
        {
            // max name length value to r8d
            mov(r8d, 250);

            // restore rax from stack
            pop(rax);
            // restore rcx from stack
            pop(rcx);
        }
    };

    void Hook_GetMaxCharCount()
    {
        // No Settings in the ASI version
    }

    bool Install()
    {
        auto patchAddress = reinterpret_cast<uintptr_t>(Assembly::search_pattern<"48 8B 88 ?? ?? ?? ?? 44 89 81 ?? ?? ?? ?? C3">());
        if (patchAddress)
        {
            patchAddress += 7;
            INFO("Found patch address: {:x}. Game base: {:x}", patchAddress, Module::get().base());

            Trampoline::AllocTrampoline(128);

            Prolog prolog{};
            prolog.ready();
            Epilog epilog{};
            epilog.ready();
            
            const auto caveHookHandle = AddCaveHook(
                patchAddress,
                { 0, 7 }, 
                FUNC_INFO(Hook_GetMaxCharCount),
                &prolog,
                &epilog,
                HookFlag::kRestoreAfterEpilog);
            caveHookHandle->Enable();
            INFO("Patch applied");
            return true;
        }
        else
        {
            ERROR("Couldn't find the address to patch");
        }
        return false;
    }
} // namespace

DWORD WINAPI Thread(LPVOID param)
{
    // Settings::GetSingleton()->Load();
    return Install();
}


BOOL APIENTRY DllMain(HMODULE a_hModule, DWORD a_dwReason, LPVOID a_lpReserved)
{
    if (a_dwReason == DLL_PROCESS_ATTACH)
    {
#ifndef NDEBUG
        MessageBoxA(NULL, "Loaded. You can attach the debugger now", "SF LongerNames ASI Plugin", NULL);
#endif
        dku::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));
        INFO("Game: {}", dku::Hook::GetProcessName());

        CloseHandle(CreateThread(nullptr, 0, Thread, nullptr, 0, nullptr));
    }

    return TRUE;
}