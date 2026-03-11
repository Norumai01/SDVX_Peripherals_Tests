# CLion + Pico 2 Setup Guide (Windows)

---

## First-Time Setup

Only needs to be done once per machine.

### 1. Install Python

Download and run the standalone installer from [python.org](https://python.org). During install, check **"Add Python to PATH"**.

Verify in a new PowerShell window:
```powershell
python --version
```

### 2. Install Git

Download from [git-scm.com](https://git-scm.com). Verify in a new PowerShell window:
```powershell
git --version
```

### 3. Clone the Pico SDK and Examples

Create a folder for your Pico work (example used here: `C:\pico`), then clone everything into it:
```powershell
cd C:\pico

git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
cd ..

git clone https://github.com/raspberrypi/pico-examples.git --branch master
git clone https://github.com/raspberrypi/pico-extras.git
```

> You can use any folder you like. Just keep track of it — you'll need the path in later steps.

### 4. Install the ARM GNU Toolchain

Download the Windows toolchain from the [ARM Developer site](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Extract it into your pico folder, e.g.:
```
C:\pico\arm_gnu_toolchain\
```

You should see folders called `bin`, `lib`, `arm-none-eabi`, etc. inside it.

### 5. Set Environment Variables

1. Press `Win` → search **"Environment Variables"** → click **"Edit the system environment variables"**
2. Click **Environment Variables...**
3. Under **User variables**, click **New** for each of the following:

| Variable Name | Value (adjust to your actual folder) |
|---|---|
| `PICO_SDK_PATH` | `C:\pico\pico-sdk` |
| `PICO_TOOLCHAIN_PATH` | `C:\pico\arm_gnu_toolchain` |
| `PICO_EXTRAS_PATH` | `C:\pico\pico-extras` |

4. Click OK on all dialogs, then **restart any open terminals or CLion** for changes to take effect.

### 6. Install CLion

Download and install CLion from [jetbrains.com/clion](https://www.jetbrains.com/clion/).

### 7. Configure the CLion Toolchain

Go to **File → Settings → Build, Execution, Deployment → Toolchains**

- Click **+** and select **MinGW**
- Set the C Compiler to your toolchain's gcc, e.g.:
  ```
  C:\pico\arm_gnu_toolchain\bin\arm-none-eabi-gcc.exe
  ```
- Set the C++ Compiler to:
  ```
  C:\pico\arm_gnu_toolchain\bin\arm-none-eabi-g++.exe
  ```

> **Why MinGW?** Picotool is a Windows binary that gets built during compilation using CLion's bundled MinGW GCC. Your actual Pico firmware uses the ARM compiler. Both are needed.

### 8. Install pyserial (for serial monitor)

```powershell
pip install pyserial
```

---

## Per-Project Setup

Do this every time you create a new project.

### 1. Create the Project Folder Structure

```
your_project/
├── CMakeLists.txt
├── pico_sdk_import.cmake    ← copy this from the SDK
└── src/
    └── main.c
```

Copy `pico_sdk_import.cmake` from your SDK folder:
```
<your PICO_SDK_PATH>\external\pico_sdk_import.cmake
```

### 2. CMakeLists.txt Template

Replace `YOUR_PROJECT_NAME` with your actual project name:

```cmake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(YOUR_PROJECT_NAME C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(YOUR_PROJECT_NAME
    src/main.c
)

target_link_libraries(YOUR_PROJECT_NAME
    pico_stdlib
    hardware_gpio
)

pico_enable_stdio_usb(YOUR_PROJECT_NAME 1)
pico_enable_stdio_uart(YOUR_PROJECT_NAME 0)

pico_add_extra_outputs(YOUR_PROJECT_NAME)
```

### 3. Open the Project in CLion

**File → Open** → select your project folder → OK

### 4. Configure CMake Settings

Go to **File → Settings → Build, Execution, Deployment → CMake**

**Toolchain:** MinGW

**CMake options** (replace `<TOOLCHAIN_PATH>` with your actual `PICO_TOOLCHAIN_PATH`):
```
-G Ninja -DPICO_BOARD=pico2 -DCMAKE_C_COMPILER="<TOOLCHAIN_PATH>\bin\arm-none-eabi-gcc.exe" -DCMAKE_CXX_COMPILER="<TOOLCHAIN_PATH>\bin\arm-none-eabi-g++.exe"
```

**Environment** (replace each `<...>` with your actual paths):
```
PICO_SDK_PATH=<your pico-sdk folder>;PICO_TOOLCHAIN_PATH=<your arm_gnu_toolchain folder>;PICO_EXTRAS_PATH=<your pico-extras folder>
```

Click **Apply → OK**.

### 5. Reset CMake and Build

**Tools → CMake → Reset Cache and Reload Project**

Then select your target from the dropdown at the top right and hit the **hammer icon** to build.

### 6. Flash to Pico

1. Hold **BOOTSEL** and plug in the Pico via USB
2. It mounts as a drive called `RP2350`
3. Find your `.uf2` in:
   ```
   your_project\cmake-build-debug-mingw\your_project.uf2
   ```
4. Drag and drop it onto the `RP2350` drive
5. The Pico reboots and runs your firmware

### 7. View Serial Output

Run in Python (with Pico already plugged in and running):

**Requirements:**
```bash
pip install pyserial
```

**Run:**
```bash
python serial_monitor.py
```

Defaults to `COM3` at `115200` baud. Edit `PORT` at the top of the script if your Pico is on a different port (check Device Manager → Ports (COM & LPT)).

> Check **Device Manager → Ports (COM & LPT)** if your Pico is not on COM3.