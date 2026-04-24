#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <sys/mman.h>
#include <vector>
#include <string>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "BetterBrightness", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "BetterBrightness", __VA_ARGS__)

const char* GFX_GAMMA_SIGNATURE = 
    "CA 92 06 F8 29 01 40 F9 C8 E2 06 F8 48 02 80 52 "
    "A8 03 16 38 28 0C 80 52 BF E3 1A 38 C9 92 02 F8 "
    "C8 12 03 78 E8 4D 82 52 01 E4 00 2F 00 10 2C 1E "
    "68 50 A7 72 02 10 2E 1E";

const uint32_t MOV_W8_10 = 0x52800148;
const uint32_t SCVTF_S2_W8 = 0x1E220102;
const uint32_t FMOV_S2_1_0 = 0x1E2E1002;

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
    LOGI("BetterBrightness thread started. Waiting for libminecraftpe.so...");

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
        if (!isLoaded) usleep(500000); 
    }

    LOGI("libminecraftpe.so is mapped! Scanning for gamma signature...");

    bool patchApplied = false;
    for (int attempts = 1; attempts <= 40; attempts++) {
        uintptr_t base_addr = ResolveSignature(GFX_GAMMA_SIGNATURE);
        if (base_addr != 0) {
            LOGI("Found Gamma signature! Applying memory patches...");
            
            uint32_t* fmov_addr = (uint32_t*)(base_addr + 52);
            uint32_t* movk_addr = (uint32_t*)(base_addr + 48);

            if (*fmov_addr != FMOV_S2_1_0) {
                LOGE("Instruction mismatch! FMOV not found. Signature might be outdated.");
                break;
            }

            long page_size = sysconf(_SC_PAGE_SIZE);

            // Unlock memory protection for the instructions
            uintptr_t page_start1 = ((uintptr_t)movk_addr) & ~(page_size - 1);
            mprotect((void*)page_start1, page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
            *movk_addr = MOV_W8_10; 

            uintptr_t page_start2 = ((uintptr_t)fmov_addr) & ~(page_size - 1);
            mprotect((void*)page_start2, page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
            *fmov_addr = SCVTF_S2_W8; 
            
            // Flush the instruction cache to ensure the CPU executes the new instructions
            __builtin___clear_cache((char*)movk_addr, (char*)(movk_addr + 1));
            __builtin___clear_cache((char*)fmov_addr, (char*)(fmov_addr + 1));

            LOGI("Better Brightness patch applied successfully! No Preloader required.");
            patchApplied = true;
            break;
        }
        sleep(1); 
    }

    if (!patchApplied) {
        LOGE("Failed to find BetterBrightness signature after 40 attempts.");
    }

    return nullptr;
}

__attribute__((constructor))
void BetterBrightness_Init() {
    pthread_t thread;
    pthread_create(&thread, nullptr, InjectionThread, nullptr);
    pthread_detach(thread);
}
