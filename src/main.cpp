#include "api/memory/Hook.h"
#include <cstdint>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "BetterBrightness", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "BetterBrightness", __VA_ARGS__)

#if __aarch64__
// --- 1.26.20 RenderDragon Gamma Signature ---
static const char* GFX_GAMMA_SIGNATURE = "? ? ? 52 ? ? ? 2F ? ? ? 1E ? ? ? 72 ? ? ? 1E ? ? ? D1 03 01 27 1E ? ? ? D1 E0 03 15 AA ? ? ? 52 ? ? ? 52";

constexpr uint32_t MOV_W8_10   = 0x52800148;
constexpr uint32_t SCVTF_S2_W8 = 0x1E220102;

constexpr ptrdiff_t OFFSET_MOVK = 12;
constexpr ptrdiff_t OFFSET_FMOV = 16;

void PatchGfxGamma() {
    LOGI("Scanning for BetterBrightness signature...");
    
    // Resolve the signature using GlossHook's memory API
    uintptr_t base = memory::resolveSignature("libminecraftpe.so", GFX_GAMMA_SIGNATURE);
    
    if (base == 0) {
        LOGE("FATAL: Could not find BetterBrightness pattern in memory.");
        return;
    }

    LOGI("SUCCESS: Found Gamma signature at %lx", base);

    if (*reinterpret_cast<uint32_t*>(base + OFFSET_FMOV) != 0x1E2E1002) {
        LOGE("FATAL: Verification failed! Assembly instruction at offset 16 did not match.");
        return;
    }

    // Apply patches using GlossHook's built-in patcher
    bool patch1 = memory::patchMemory(reinterpret_cast<void*>(base + OFFSET_MOVK), &MOV_W8_10, sizeof(MOV_W8_10));
    bool patch2 = memory::patchMemory(reinterpret_cast<void*>(base + OFFSET_FMOV), &SCVTF_S2_W8, sizeof(SCVTF_S2_W8));

    if (patch1 && patch2) {
        LOGI("SUCCESS: Patched Gamma limits! Night vision active.");
    } else {
        LOGE("FATAL: GlossHook failed to apply the memory patch.");
    }
}
#endif

// Initialization function called when the library is loaded
__attribute__((constructor))
void BetterBrightness_Init() {
#if __aarch64__
    PatchGfxGamma();
#else
    LOGE("BetterBrightness only supports ARM64 architecture.");
#endif
}
