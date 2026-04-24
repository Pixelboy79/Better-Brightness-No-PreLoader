# BetterBrightness (Standalone / Snail Edition)

In vanilla Minecraft, brightness ranges from 0 to 100. With BetterBrightness, the brightness range is extended significantly, allowing for much brighter gameplay (up to 10x normal brightness).

This specific fork has been **completely rewritten** to strip out all Levi Launcher and `libpreloader.so` dependencies. It is now a fully standalone `.so` library optimized for raw memory injection via the **Snail Method**.

## 🚀 Key Features

* **Snail Method Compatible:** Injects perfectly as a standalone library without requiring secondary loaders.
* **No Levi Environment Required:** Completely removes the LiteLDev preloader dependencies.
* **Safe Background Polling:** Utilizes a background thread to safely wait for `libminecraftpe.so` to unpack, preventing race conditions and segmentation faults during early injection.
* **Native Memory Patching:** Uses lightweight C++ `mprotect` to override raw AArch64 assembly memory instructions instantly, requiring zero external hooking frameworks.

## 📋 Requirements

* **Injector:** A Minecraft Bedrock APK modified to inject custom `.so` libraries (The Snail Method).
* **Architecture:** `arm64-v8a` (Android).

## 🛠️ Installation

1. Download the compiled `libBetterBrightness.so` from the Actions tab or build it yourself.
2. Add the `libBetterBrightness.so` mod to your Snail Method injection path in your patched Minecraft APK.
3. Launch Minecraft. The mod will run silently in the background and patch the game's gamma limits automatically.

## 🏗️ Building the Project

This project uses standard CMake and features an automated GitHub Actions CI/CD pipeline. 

### Automated Build (Recommended)
You do not need to install the Android NDK locally. 
1. Fork or push your code to GitHub.
2. The GitHub Actions workflow will automatically compile the code for `arm64-v8a` and upload the standalone `libBetterBrightness.so` to the **Actions** tab.

### Manual Local Build
If you prefer to compile locally, ensure you have the Android NDK (r26b recommended) and CMake installed.

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/your/android-ndk/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 -DCMAKE_BUILD_TYPE=Release
make
```
## 📜 Credits & Copyright

**© 2026 Pixelboypro** — *Standalone Snail Method Conversion, Levi Launcher/Preloader Dependency Removal, and Native C++ Memory Patching Rewrite.*

* Original concept and AArch64 gamma memory signatures created by **RadiantByte**.
