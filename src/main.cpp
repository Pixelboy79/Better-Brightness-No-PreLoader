#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <dobby.h> // Using Dobby to bypass Android Memory Protection

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "BetterBrightness", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "BetterBrightness", __VA_ARGS__)

// --- 1.26.20 RenderDragon Gamma Signature ---
static const char* GFX_GAMMA_SIGNATURE = "? ? ? 52 ? ? ? 2F ? ? ? 1E ? ? ? 72 ? ? ? 1E ? ? ? D1 03 01 27 1E ? ? ? D1 E0 03 15 AA ? ? ? 52 ? ? ? 52";

constexpr uint32_t MOV_W8_10   = 0x52800148;
constexpr uint32_t SCVTF_S2_W8 = 0x1E220102;

constexpr ptrdiff_t OFFSET_MOVK = 12;
constexpr ptrdiff_t OFFSET_FMOV = 16;

// Standalone memory scanner
static uintptr_t ResolveSignature(const char* sig) {
    std::vector<int> pattern;
    const char* p = sig;
    while (*p) {
        if (*p == ' ') { p++; continue; }
        if (*p == '?') { pattern.push_back(-1); p++; if(*p=='?') p++; continue; }
        pattern.push_back(strtol(p, nullptr, 16));
        p += 2;
    }

    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (!strstr(line, "libminecraftpe.so") || !strstr(line, "r-x")) continue; 
        
        uintptr_t start, end;
        if (sscanf(line, "%lx-%lx", &start, &end) != 2) continue;

        uint8_t* scan_base = (uint8_t*)start;
        size_t size = end - start;
        if (size < pattern.size()) continue;

        for (size_t i = 0; i < size - pattern.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < pattern.size(); j++) {
                if (pattern[j] != -1 && scan_base[i + j] != pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                fclose(fp);
                return (uintptr_t)(scan_base + i);
            }
        }
    }
    fclose(fp);
    return 0;
}

// Background Turbo-Thread
void* InjectionThread(void* arg) {
    LOGI("BetterBrightness Turbo Thread started.");

    bool isLoaded = false;
    while (!isLoaded) {
        FILE* fp = fopen("/proc/self/maps", "r");
        if (fp) {
            char line[512];
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "libminecraftpe.so") && strstr(line, "r-x")) {
                    isLoaded = true;
                    break;
                }
            }
            fclose(fp);
        }
        if (!isLoaded) usleep(10000); 
    }

    LOGI("libminecraftpe.so mapped! Scanning memory instantly...");

    bool patchApplied = false;
    for (int attempts = 1; attempts <= 100; attempts++) {
        uintptr_t base = ResolveSignature(GFX_GAMMA_SIGNATURE);
        if (base != 0) {
            LOGI("SUCCESS: Found Gamma signature at %lx", base);
            
            // Verify we are patching the correct instructions
            if (*reinterpret_cast<uint32_t*>(base + OFFSET_FMOV) != 0x1E2E1002) {
                LOGE("Verification failed: unexpected assembly instruction at offset 16.");
            } else {
                // Apply the Gamma Unlocks using DobbyCodePatch safely
                DobbyCodePatch(reinterpret_cast<void*>(base + OFFSET_MOVK), (uint8_t*)&MOV_W8_10, sizeof(MOV_W8_10));
                DobbyCodePatch(reinterpret_cast<void*>(base + OFFSET_FMOV), (uint8_t*)&SCVTF_S2_W8, sizeof(SCVTF_S2_W8));
                
                LOGI("SUCCESS: Patched Gamma limits via Dobby! Night vision active.");
                patchApplied = true;
            }
            break; 
        }
        usleep(50000); 
    }

    if (!patchApplied) {
        LOGE("FATAL: Could not find BetterBrightness pattern in memory.");
    }

    return nullptr;
}

__attribute__((constructor))
void BetterBrightness_Init() {
    pthread_t thread;
    pthread_create(&thread, nullptr, InjectionThread, nullptr);
    pthread_detach(thread);
}
