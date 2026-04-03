# Toolchain for GD32F470, ARM Cortex-M4
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(ARM_TOOLCHAIN "G:/soft/Clion_Tool/gcc-arm-none-eabi-10.3-2021.10-win32/gcc-arm-none-eabi-10.3-2021.10/bin")

set(CMAKE_C_COMPILER   "${ARM_TOOLCHAIN}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${ARM_TOOLCHAIN}/arm-none-eabi-gcc.exe")
set(CMAKE_AR           "${ARM_TOOLCHAIN}/arm-none-eabi-ar.exe")
set(CMAKE_OBJCOPY      "${ARM_TOOLCHAIN}/arm-none-eabi-objcopy.exe")
set(CMAKE_OBJDUMP      "${ARM_TOOLCHAIN}/arm-none-eabi-objdump.exe")
set(SIZE               "${ARM_TOOLCHAIN}/arm-none-eabi-size.exe")

# -------------------------
# 禁用浮点，启用软浮点
# -------------------------
set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -mthumb-interwork -mfloat-abi=soft -ffunction-sections -fdata-sections -fno-common -fmessage-length=0")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -std=gnu++17")
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp -mcpu=cortex-m4 -mthumb")

# -------------------------
# 定义宏
# -------------------------
add_compile_definitions(GD32F470 ARM_MATH_CM4 ARM_MATH_MATRIX_CHECK ARM_MATH_ROUNDING)

# -------------------------
# 禁止 CMake 测试编译器
# -------------------------
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

message(STATUS "Using GD32F470 ARM Cortex-M4 toolchain with soft-float")
