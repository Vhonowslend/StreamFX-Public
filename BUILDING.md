# Building
This document intends to guide you through the process of building StreamFX. It requires understanding of the tools used, and may require you to learn tools yourself before you can advance further in the guide. It is intended to be used by developers and contributors.

## Building Bundled
<details open><summary>The main method to build StreamFX is to first set up an OBS Studio copy and then integrate the StreamFX repository into it.</summary>

1. [Uninstall](Uninstallation) any currently installed versions of StreamFX to prevent conflicts.
2. Follow the [OBS Studio build guide](https://obsproject.com/wiki/install-instructions) for automated building on your platform of choice.
    - **MacOS:** You will need to use the XCode generator to build StreamFX as the Ninja generator does not support the flags StreamFX requires.
3. Integrate StreamFX into the OBS Studio build flow:
    1. Navigate to `<obs studio source>/UI/frontend-plugins`
    2. Open a `git` enabled shell (git-bash on windows).
    3. Run `git submodule add 'https://github.com/Xaymar/obs-StreamFX.git' streamfx`.
    4. Run `git submodule update --init --recursive`.
    5. Append the line `add_subdirectory(streamfx)` to the `CMakeLists.txt` file in the same directory.
4. Run the same steps from the build guide in step 2 again.
5. Done. StreamFX is now part of the build.

</details>

## Building CI-Style
<details><summary>This method is designed for continuous integration and releases, and requires significant knowledge of CMake, OBS, and various other tools. Additionally it is not guaranteed to work on every machine, as it is only designed for use in continuous integration and nowhere else. It may even stop being maintained entirely with no warning whatsoever. You are entirely on your own when you choose this method.</summary>

#### Install Prerequisites / Dependencies
- [Git](https://git-scm.com/)
    - **Debian / Ubuntu:** `sudo apt install git`
- [CMake](https://cmake.org/) 3.20 (or newer)
    - **Debian / Ubuntu:** `sudo apt install cmake`
- A compatible Compiler:
    - **Windows**  
      [Visual Studio](https://visualstudio.microsoft.com/vs/) 2019 (or newer)
    - **MacOS**  
      Xcode 11.x (or newer) for x86_64  
      Xcode 12.x (or newer) for arm64
    - **Debian / Ubuntu**
        - Essential Build Tools:  
          `sudo apt install build-essential pkg-config checkinstall make ninja-build` 
        - One of:
            - GCC 11 (or newer)  
              `sudo apt install gcc-11 g++-11`
            - [LLVM](https://releases.llvm.org/) Clang 14 (or newer)  
              `sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"`
        - One of:
            - ld or gold  
              `sudo apt install binutils`
            - [LLVM](https://releases.llvm.org/) lld  
              `sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"`
            - [mold](https://github.com/rui314/mold)  
              `sudo apt install mold`
- [Qt](https://www.qt.io/) 6:
    - **Windows**  
      A Node.JS based tool is provided toread and parse the `/third-party/obs-studio/buildspec.json` file. See `/.github/workflows/main.yml` on usage and output parsing.
    - **MacOS**  
      A Node.JS based tool is provided toread and parse the `/third-party/obs-studio/buildspec.json` file. See `/.github/workflows/main.yml` on usage and output parsing.
    - **Debian / Ubuntu:**  
      `sudo apt install qt6-base-dev qt6-base-private-dev libqt6svg6-dev`
- [CURL](https://curl.se/):
    - **Windows**  
      A Node.JS based tool is provided toread and parse the `/third-party/obs-studio/buildspec.json` file. See `/.github/workflows/main.yml` on usage and output parsing.
    - **MacOS**  
      A Node.JS based tool is provided toread and parse the `/third-party/obs-studio/buildspec.json` file. See `/.github/workflows/main.yml` on usage and output parsing.
    - **Debian / Ubuntu:**
      `sudo apt install libcurl4-openssl-dev`
- [FFmpeg](https://ffmpeg.org/) (Optional, for FFmpeg component only):
    - **Windows**  
      A Node.JS based tool is provided toread and parse the `/third-party/obs-studio/buildspec.json` file. See `/.github/workflows/main.yml` on usage and output parsing.
    - **MacOS**  
      A Node.JS based tool is provided toread and parse the `/third-party/obs-studio/buildspec.json` file. See `/.github/workflows/main.yml` on usage and output parsing.
    - **Debian / Ubuntu**  
      `sudo apt install libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libswresample-dev libswscale-dev`
- [LLVM](https://releases.llvm.org/) (Optional, for clang-format and clang-tidy integration only):
    - **Debian / Ubuntu**  
      `sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" all`
- [InnoSetup](https://jrsoftware.org/isinfo.php) (Optional, for **Windows** installer only)

### Cloning the Project
Using your preferred tool of choice for git, clone the repository including all submodules into a directory. If you use git directly, then you can clone the entire project with `git clone --recursive https://github.com/Xaymar/obs-StreamFX.git streamfx`.

### Configuring with CMake
There are two ways to handle this step, with the GUI variant of CMake and with the command line version of CMake. This guide will focus on the GUI variant, but all the steps below can be done with the command line version as well.

1. Launch CMake-GUI and wait for it to open.
2. Click the button named `Browse Build` and point it at an empty folder. For example, create a folder in the project called `build` and select that folder.
3. Click the button named `Browse Source` and point it at the project itself.
4. Click the button named `Configure`, select your preferred Generator (the default is usually fine), and wait for it to complete. This will most likely result in an error which is expected.
5. Adjust the variables in the variable list as necessary. Take a look at [the documentation](#CMake-Options) for what each option does.
6. Click the button named `Generate`, which will also run `Configure`. Both together should succeed if you did everything correctly.
7. If available, you can now click the button named `Open Project` to immediately jump into your IDE of choice.

</details>

## CMake Options
<details><summary>The project is intended to be versatile and configurable, so we offer almost everything to be configured on a silver platter directly in CMake (if possible). If StreamFX detects that it is being built together with other projects, it will automatically prefix all options with `StreamFX_` to prevent collisions.</summary>

### Generic
- `GIT` (not prefixed)  
  Path to the `git` binary on your system, for use with features that require git during configuration and generation.
- `VERSION`  
  Set or override the version of the project with a custom one. Allowed formats are: SemVer 2.0.0, CMake.

### Code
- `ENABLE_CLANG`  
  Enable integration of `clang-format` and `clang-tidy`
- `CLANG_PATH` (not prefixed, only with `ENABLE_CLANG`)  
  Path to the `clang` installation containing `clang-format` and `clang-tidy`. Only used as a hint.
- `CLANG_FORMAT_PATH` and `CLANG_TIDY_PATH` (not prefixed)
  Path to `clang-format` and `clang-tidy` that will be used.  

### Dependencies
- `LibObs_DIR`  
  Path to the obs-studio libraries.
- `Qt5_DIR`, `Qt6_DIR` or `Qt_DIR` (autodetect)  
  Path to Qt5 (OBS Studio 27.x and lower) or Qt6 (OBS Studio 28.x and higher).
- `FFmpeg_DIR`  
  Path to compatible FFmpeg libraries and headers.
- `CURL_DIR`  
  Path to compatible CURL libraries and headers.
- `AOM_DIR`  
  Path to compatible AOM libraries and headers.

### Compiling
- `ENABLE_FASTMATH`  
  Enable fast math optimizations if the compiler supports them. This trades precision for performance, and is usually good enough anyway.
- `ENABLE_LTO`  
  Enable link time optimization for faster binaries in exchange for longer build times.
- `ENABLE_PROFILING`  
  Enable CPU and GPU profiling code, this option reduces performance drastically.
- `TARGET_*`  
  Specify which architecture target the generated binaries will use.

### Components
- `COMPONENT_<NAME>`  
  Enable the component by the given name.

### Installing & Packaging
These options are only available in CI-Style mode.

- `CMAKE_INSTALL_PREFIX`  
  The path in which installed content should be placed when building the `install` target.
- `STRUCTURE_PACKAGEMANAGER`  
  If enabled will install files in a layout compatible with package managers.
- `STRUCTURE_UNIFIED`  
  Enable to install files in a layout compatible with an OBS Studio plugin manager.
- `PACKAGE_NAME`  
  The name of the packaged archive, excluding the prefix, suffix and extension.
- `PACKAGE_PREFIX`  
  The path in which the packages should be placed.
- `PACKAGE_SUFFIX`  
  The suffix to attach to the name, before the file extension. If left blank will attach the current version string to the package.
- `STRUCTURE_UNIFIED`  
  Enable to replace the PACKAGE_ZIP target with a target that generates a single `.obs` file instead.

</details>
