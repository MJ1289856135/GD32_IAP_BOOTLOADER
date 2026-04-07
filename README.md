# GD32_IAP_BOOTLOADER

## 📌 Overview

`GD32_IAP_BOOTLOADER` is an industrial-grade bootloader designed for in-application programming (IAP).
It works together with `GD32_IAP_APP` to implement a **reliable firmware upgrade solution**.

The Bootloader is responsible for:

* Detecting firmware update requests
* Reading firmware from external Flash (W25Q128 via FatFs)
* Programming internal Flash
* Validating and jumping to the application

---

## 🧩 Full System Architecture

```text
                ┌──────────────────────────────┐
                │           PC Tool            │
                │  (SecureCRT / TeraTerm)     │
                └──────────────┬──────────────┘
                               │ YModem
                               ▼
                ┌──────────────────────────────┐
                │        GD32_IAP_APP          │
                │  - Firmware Download        │
                │  - Store to W25Q128         │
                │  - Set Update Flag          │
                └──────────────┬──────────────┘
                               │ Reset
                               ▼
                ┌──────────────────────────────┐
                │     GD32_IAP_BOOTLOADER      │
                │  - Detect Update Flag        │
                │  - Flash Internal Memory     │
                │  - Validate Application      │
                │  - Jump to APP              │
                └──────────────────────────────┘
```

---

## 🚀 Key Features

* ✅ Automatic firmware upgrade detection via flash flag
* ✅ External Flash firmware loading (W25Q128 + FatFs)
* ✅ Safe internal Flash erase & programming
* ✅ Application validity check (SP / RESET vector)
* ✅ Robust jump mechanism (MSP + VTOR relocation)
* ✅ Upgrade flag auto-clear after success
* ✅ Fail-safe behavior (no valid APP → stay in Bootloader)

---

## ⚙️ Boot Flow

```text
        Power On / Reset
                │
                ▼
        Bootloader Start
                │
                ▼
      Check Update Flag
         │            │
        YES           NO
         │            │
         ▼            ▼
  Perform Upgrade   Check APP
         │            │
         ▼            ▼
  Flash Internal     Valid?
         │         │      │
         ▼        YES     NO
  Clear Flag       │      │
         ▼        ▼       ▼
     Jump APP   Jump APP  Stay in Bootloader
```

---

## 🧠 Update Flag Mechanism

Shared with APP:

```c
#define UPDATE_FLAG_ADDR   0x08030000
#define UPDATE_FLAG_VALUE  0xA5A5A5A5
```

### Structure (example)

```c
typedef struct
{
    uint32_t flag;
    uint32_t file_size;
    char     filename[32];
} UpdateInfo_t;
```

### Workflow

1. APP writes update info into Flash
2. Bootloader reads and validates it
3. If valid → perform firmware upgrade
4. After success → erase flag sector

---

## 🔥 Core Functions

### RunApp()

Main Bootloader entry:

* Initializes system
* Checks update flag
* Controls upgrade or normal boot

---

### Bootloader_FlashApp()

* Opens firmware file from FatFs
* Erases internal Flash sectors
* Writes firmware in aligned format
* Handles remaining bytes safely

---

### ExistApplication()

Validates application:

* Stack Pointer (SP) in SRAM range
* Reset Handler in Flash range

---

### JumpToApp()

Safe jump procedure:

* Disable interrupts
* Reset peripherals
* Set MSP
* Relocate vector table
* Jump to application

---

## ⚠️ Safety Design

### ✔ Application Validation

```c
if ((sp < SRAM_START) || (sp > SRAM_END))
    return 0;
```

---

### ✔ Flash Protection

* Sector-based erase
* Word-aligned programming
* Boundary checking

---

### ✔ Fail-Safe Mechanism

* Invalid APP → no jump
* Upgrade failure → stay in Bootloader
* Flag only cleared after success

---

## 💾 Flash Layout (Example)

| Region            | Address    | Description      |
| ----------------- | ---------- | ---------------- |
| Bootloader        | 0x08000000 | Boot code        |
| Update Flag       | 0x08030000 | Upgrade info     |
| Application (APP) | 0x08040000 | User firmware    |
| External Flash    | W25Q128    | Firmware storage |

---

## 🔁 Upgrade Sequence (End-to-End)

```text
APP:
  Download firmware (YModem)
        ↓
  Store in W25Q128
        ↓
  Set update flag
        ↓
  System reset

Bootloader:
  Detect flag
        ↓
  Read firmware
        ↓
  Flash internal memory
        ↓
  Clear flag
        ↓
  Jump to APP
```

---

## ❗ Common Issues

### 1. APP not jumping

* Incorrect linker address
* Invalid vector table

---

### 2. Upgrade failed

* Flash erase/write error
* File read failure

---

### 3. Stuck in Bootloader

* No valid APP
* Update flag not cleared

---

## 🔗 Related Project

👉 `GD32_IAP_APP` (Firmware download side)

This Bootloader must be used together with the APP project.

---

## 🛠️ Hardware Requirements

* GD32F470 series MCU
* External SPI Flash (W25Q128)
* UART interface (for firmware download via APP)

---

## 📄 License

MIT License
