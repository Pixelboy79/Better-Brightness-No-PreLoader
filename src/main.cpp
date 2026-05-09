#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <fstream>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "BetterBrightness", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "BetterBrightness", __VA_ARGS__)

namespace fs = std::filesystem;

// --- 1.26.20 RenderDragon Gamma Signature ---
static const char* GFX_GAMMA_SIGNATURE = "? ? ? 52 ? ? ? 2F ? ? ? 1E ? ? ? 72 ? ? ? 1E ? ? ? D1 03 01 27 1E ? ? ? D1 E0 03 15 AA ? ? ? 52 ? ? ? 52";

constexpr uint32_t MOV_W8_10   = 0x52800148;
constexpr uint32_t SCVTF_S2_W8 = 0x1E220102;

constexpr ptrdiff_t OFFSET_MOVK = 12;
constexpr ptrdiff_t OFFSET_FMOV = 16;

// Function to write a log file directly to the phone's storage
void WriteDebugLog(const std::string& message) {
    std::string path = "/storage/emulated/0/Android/data/com.mojang.minecraftpe.nv/files/mods/BetterBrightness_Log.txt";
    std::error_code ec;
    fs::create_directories(fs::path(path).parent_path(), ec);
    std::ofstream logFile(path, std::ios::app);
    if (logFile.is_open()) {
        logFile << message << "\n";
        logFile.close();
    }
}

// Function to safely overwrite assembly instructions
static bool PatchMemory(void* addr, uint32_t insn) {
    uintptr_t page_start = (uintptr_t)addr & ~(uintptr_t)4095;
    size_t    page_size  = 4096; 
    
    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) != 0) {
        WriteDebugLog("ERROR: mprotect WRITE denied by Android at " + std::to_string((uintptr_t)addr));
        return false;
    }
    
    memcpy(addr, &insn, sizeof(insn));
    __builtin___clear_cache((char*)addr, (char*)addr + sizeof(insn));
    
    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_EXEC) != 0) {
        WriteDebugLog("ERROR: mprotect EXEC denied by Android at " + std::to_string((uintptr_t)addr));
        return false;
    }
    
    return true;
}

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

void* InjectionThread(void* arg) {
    // Clear old log file
    std::string path = "/storage/emulated/0/Android/data/com.mojang.minecraftpe.nv/files/mods/BetterBrightness_Log.txt";
    std::remove(path.c_str());
    WriteDebugLog("--- BetterBrightness Booted ---");

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

    WriteDebugLog("SUCCESS: libminecraftpe.so mapped. Scanning for pattern...");

    bool patchApplied = false;
    for (int attempts = 1; attempts <= 100; attempts++) {
        uintptr_t base = ResolveSignature(GFX_GAMMA_SIGNATURE);
        if (base != 0) {
            WriteDebugLog("SUCCESS: Found Gamma signature at memory address: " + std::to_string(base));
            
            if (*reinterpret_cast<uint32_t*>(base + OFFSET_FMOV) != 0x1E2E1002) {
                WriteDebugLog("FATAL: Verification failed! Assembly instruction at offset 16 did not match.");
            } else {
                if (PatchMemory(reinterpret_cast<void*>(base + OFFSET_MOVK), MOV_W8_10) &&
                    PatchMemory(reinterpret_cast<void*>(base + OFFSET_FMOV), SCVTF_S2_W8)) {
                    WriteDebugLog("SUCCESS: Patched Gamma limits perfectly! Night vision should be active.");
                    patchApplied = true;
                }
            }
            break; 
        }
        usleep(50000); 
    }

    if (!patchApplied) {
        WriteDebugLog("FATAL: Could not find BetterBrightness pattern in memory after 100 attempts.");
    }

    return nullptr;
}

__attribute__((constructor))
void BetterBrightness_Init() {
    pthread_t thread;
    pthread_create(&thread, nullptr, InjectionThread, nullptr);
    pthread_detach(thread);
}
