# GD32_IAP_BOOTLOADER

## 📌 项目简介

GD32_IAP_BOOTLOADER 是一个工业级 IAP 引导程序，用于在系统启动时检测升级请求，并从外部 Flash（W25Q128）中读取固件，烧写到内部 Flash，最终跳转执行应用程序。

该 Bootloader 具备 **完整升级流程控制 + 应用有效性检测 + 安全跳转机制**。

---

## 🚀 核心功能

* ✅ 上电自动检测升级标志
* ✅ 从外部 Flash 读取固件（FatFs）
* ✅ 自动擦除并写入内部 Flash
* ✅ 应用有效性检测（SP / RESET 校验）
* ✅ 安全跳转到 APP
* ✅ 升级完成自动清除标志位
* ✅ 异常保护（无效 APP 不跳转）

---

## 🧩 启动流程

```
        上电复位
            │
            ▼
     Bootloader 启动
            │
            ▼
   检查升级标志位
        │        │
       是        否
        │        │
        ▼        ▼
 执行固件升级   检查 APP 是否存在
        │        │
        ▼        ▼
  写入内部 Flash   有 → 跳转
        │          无 → 停留 Bootloader
        ▼
 清除升级标志
        ▼
 跳转 APP
```

---

## 🔍 核心函数说明

### RunApp()

* Bootloader 主入口
* 控制升级流程 & 启动逻辑

---

### Bootloader_FlashApp()

* 从文件系统读取固件
* 擦除目标 Flash 区域
* 按字写入内部 Flash

---

### ExistApplication()

* 校验 APP 是否有效：

  * SP 是否在 SRAM 范围
  * RESET 是否在 Flash 范围

---

### JumpToApp()

* 设置 MSP
* 重定位向量表（VTOR）
* 跳转执行 APP

---

## ⚙️ Flash 规划（示例）

| 区域         | 地址          |
| ---------- | ----------- |
| Bootloader | 0x08000000  |
| APP        | 0x08040000  |
| 升级标志位      | 指定 Flash 扇区 |
| 外部 Flash   | W25Q128     |

---

## ⚠️ 关键设计点

### ✔ APP 有效性检测

```c
if ((sp < SRAM_START) || (sp > SRAM_END))
    return 0;
```

---

### ✔ 跳转前系统清理

* 关闭中断
* 复位 RCC
* 关闭串口

---

### ✔ 向量表重定位

```c
SCB->VTOR = app_addr;
```

---

## ❗ 常见问题

### 1️⃣ APP 无法跳转

* 向量表地址错误
* 链接地址未修改（必须匹配 APP1_ADDR）

---

### 2️⃣ 升级失败

* Flash 擦除失败
* 文件系统读取异常
* bin 文件损坏

---

### 3️⃣ 死机在 Bootloader

* APP 无效
* 未写入升级标志

---

## 🛠️ 适用场景

* 工业设备固件升级
* 嵌入式 OTA Bootloader
* 高可靠升级系统

---

## 📄 License

MIT License
