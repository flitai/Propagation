# 代码结构分析

### 1. 整体功能概述

此代码库提供了一系列用于雷达和电子战（EW）领域的电磁波传播计算的函数。代码由美国海军研究实验室（Naval Research Laboratory）开发，主要用于建模与仿真。

其核心功能包括：
* **雷达性能计算**：计算在不同条件下（如自由空间或考虑传播因子）接收到的雷达信号功率。
* **探测距离计算**：计算电子支援措施（ESM）接收机在自由空间中的单向探测距离和路径损耗。
* **传播因子转换**：在路径损耗和传播因子（PPF）之间进行转换。
* **频率波段转换**：将给定的频率（MHz）转换为标准的美国ECM（US ECM）或IEEE雷达波段，并能反向查询波段对应的频率范围。

---

### 2. 结构体分析

#### `RadarParameters` 结构体
此结构体用于封装雷达系统的关键参数，方便在传播计算中使用。

| 成员变量 | 类型 | 描述 |
| :--- | :--- | :--- |
| `freqMHz` | `double` | 频率 (MHz) |
| `antennaGaindBi`| `double` | 天线增益 (dBi) |
| `noiseFiguredB`| `double` | 噪声系数 (dB) |
| `pulseWidth_uSec`| `double` | 脉冲宽度 (微秒) |
| `noisePowerdB` | `double` | 噪声功率 (dB)，可由噪声系数和脉宽计算得出 |
| `systemLossdB` | `double` | 系统总损耗 (dB) |
| `xmtPowerKW` | `double` | 发射功率 (kW) |
| `xmtPowerW` | `double` | 发射功率 (W)，可由kW单位换算得到 |
| `hbwD` | `double` | 水平波束宽度 (度) |

---

### 3. 函数功能及输入输出分析

以下是`Propagation.cpp`中实现的每个主要功能的详细分析。

#### 1. `getRcvdPowerFreeSpace`
* **功能**：根据自由空间雷达距离方程，计算接收到的信号功率（单位：dB）。此函数可以计算双向（雷达目标探测）和单向（通信或ESM）链路的接收功率。
* **输入参数**:
    * `rngMeters` (`double`): 目标距离 (米)。
    * `freqMhz` (`double`): 发射机频率 (MHz)。
    * `powerWatts` (`double`): 发射机峰值功率 (瓦特)。
    * `xmtGaindB` (`double`): 发射天线增益 (dB)。
    * `rcvGaindB` (`double`): 接收天线增益 (dB)。
    * `rcsSqm` (`double`): 目标的雷达截面积 (平方米)。
    * `systemLossdB` (`double`): 系统总损耗 (dB)。
    * `oneWay` (`bool`): 布尔标志。`false`（默认）表示计算双向雷达方程；`true`表示计算单向链路。
* **输出**:
    * 返回计算出的自由空间接收功率 (dB)。
* **实现细节**:
    * **双向 (oneWay = false)**: $P_{rcvd} = G_{t} + G_{r} - L_{sys} + 10 \cdot \log_{10}\left(\frac{P_t \cdot \sigma \cdot \lambda^2}{(4\pi)^3 \cdot R^4}\right)$
    * **单向 (oneWay = true)**: $P_{rcvd} = G_{t} + G_{r} - L_{sys} + 10 \cdot \log_{10}\left(\frac{P_t \cdot \lambda^2}{(4\pi \cdot R)^2}\right)$
    * 其中 $\lambda$ 是波长，从`freqMhz`计算得出。

#### 2. `getRcvdPowerBlake`
* **功能**：基于`getRcvdPowerFreeSpace`的计算结果，并根据拉蒙特·布雷克（Lamont V. Blake）的《雷达距离性能分析》一书，额外应用一个“模式传播因子”（Pattern Propagation Factor, PPF），以获得更精确的接收功率。
* **输入参数**:
    * 与`getRcvdPowerFreeSpace`相同，但额外增加：
    * `ppfdB` (`double`): 模式传播因子 (dB)。
* **输出**:
    * 返回应用传播因子后修正的接收功率 (dB)。
* **实现细节**:
    * **双向 (oneWay = false)**: 最终功率 = 自由空间接收功率 + $4 \cdot ppfdB$
    * **单向 (oneWay = true)**: 最终功率 = 自由空间接收功率 + $2 \cdot ppfdB$

#### 3. `getOneWayFreeSpaceRangeAndLoss`
* **功能**：计算一个电子支援措施（ESM）接收机对特定发射源的单向自由空间探测距离（米），并可选择性地输出该距离上的自由空间路径损耗。
* **输入参数**:
    * `xmtGaindB` (`double`): 发射天线增益 (dB)。
    * `xmtFreqMhz` (`double`): 发射机频率 (MHz)。
    * `xmtrPwrWatts` (`double`): 发射机峰值功率 (瓦特)。
    * `rcvrSensDbm` (`double`): 接收机灵敏度 (dBm)。
    * `fsLossDb` (`double*`): 一个指向`double`的指针（可选），用于存储计算出的自由空间路径损耗 (dB)。
* **输出**:
    * 返回ESM接收机的自由空间探测距离 (米)。
    * 如果`fsLossDb`指针不为`nullptr`，则其指向的地址将被写入计算出的路径损耗值。

#### 4. `lossToPpf`
* **功能**：将路径损耗值转换为模式传播因子（PPF）。这在需要从总损耗中分离出自由空间损耗以分析额外传播效应时非常有用。
* **输入参数**:
    * `slantRange` (`double`): 到目标的斜距 (米)。
    * `freqMHz` (`double`): 发射机频率 (MHz)。
    * `loss_dB` (`double`): 功率模式路径损耗 (dB)。
* **输出**:
    * 返回计算出的模式传播因子 (dB)。
* **实现细节**:
    * 公式为：$PPF_{dB} = \text{FreeSpaceLoss}_{dB} - \text{PathLoss}_{dB}$
    * 其中自由空间损耗由频率和距离计算得出。

#### 5. 频率波段转换函数 (`toUsEcm`, `toIeeeBand`, `getFreqMhzRange`)
* **功能**：这些函数提供了频率值（MHz）和标准波段代号之间的双向转换。
* **`toUsEcm(double freqMhz)`**:
    * **输入**: 频率 (MHz)。
    * **输出**: 对应的`FrequencyBandUsEcm`枚举值（如 `USECM_FREQ_A`, `USECM_FREQ_B` 等）。
* **`toIeeeBand(double freqMhz, bool useMM)`**:
    * **输入**: 频率 (MHz) 和一个布尔标志`useMM`。如果`useMM`为`true`，则30-300GHz之间的频率会返回`IEEE_FREQ_MM`（毫米波）。
    * **输出**: 对应的`FrequencyBandIEEE`枚举值（如 `IEEE_FREQ_L`, `IEEE_FREQ_X`, `IEEE_FREQ_KA` 等）。
* **`getFreqMhzRange(...)`**:
    * **输入**: `FrequencyBandUsEcm`或`FrequencyBandIEEE`枚举值，以及两个指向`double`的指针。
    * **输出**: 无返回值。但`minFreqMhz`和`maxFreqMhz`指针指向的地址将被写入该波段对应的最小和最大频率值 (MHz)。

# 示例代码

根据该C++代码库，给出具体的功能调用示例。

每个示例都包含一个独立的函数，并附有详细的代码注释和功能说明。

### 准备工作

请将以下所有示例函数代码放入一个`main.cpp`文件中，并与您提供的`Propagation.h`和`Propagation.cpp`放在同一目录中进行编译。

C++

```
// main.cpp

#include "Propagation.h" // 您的代码库头文件
#include <iostream>
#include <iomanip>
#include <string>

// 辅助函数，用于打印IEEE波段枚举的字符串表示
std::string getIeeeBandString(simCore::FrequencyBandIEEE band) {
    switch (band) {
        case simCore::IEEE_FREQ_HF:  return "HF Band";
        case simCore::IEEE_FREQ_VHF: return "VHF Band";
        case simCore::IEEE_FREQ_UHF: return "UHF Band";
        case simCore::IEEE_FREQ_L:   return "L Band";
        case simCore::IEEE_FREQ_S:   return "S Band";
        case simCore::IEEE_FREQ_C:   return "C Band";
        case simCore::IEEE_FREQ_X:   return "X Band";
        case simCore::IEEE_FREQ_KU:  return "Ku Band";
        case simCore::IEEE_FREQ_K:   return "K Band";
        case simCore::IEEE_FREQ_KA:  return "Ka Band";
        case simCore::IEEE_FREQ_V:   return "V Band";
        case simCore::IEEE_FREQ_W:   return "W Band";
        default: return "Unknown or Out of Bounds";
    }
}
```

------

### 1. 雷达参数结构体使用 - 展示如何设置和使用RadarParameters结构

这个示例展示了如何声明并填充 `RadarParameters` 结构体，以备后续计算使用。

C++

```
void example_radar_parameters_usage() {
    std::cout << "--- 1. 雷达参数结构体使用 ---\n";
    
    // 1. 声明一个RadarParameters结构体变量
    simCore::RadarParameters myRadar;

    // 2. 设置其成员变量
    myRadar.freqMHz = 9500.0;         // X波段频率 (9.5 GHz)
    myRadar.xmtPowerKW = 50.0;          // 发射功率 50 kW
    myRadar.xmtPowerW = myRadar.xmtPowerKW * 1000.0; // 自动换算为瓦特
    myRadar.antennaGaindBi = 40.0;      // 天线增益 40 dBi
    myRadar.systemLossdB = 5.0;         // 系统总损耗 5 dB
    myRadar.pulseWidth_uSec = 1.0;      // 脉冲宽度 1 微秒
    myRadar.noiseFiguredB = 3.0;        // 噪声系数 3 dB
    myRadar.hbwD = 1.5;                 // 水平波束宽度 1.5 度

    // 3. 在程序中使用这些参数
    std::cout << "雷达参数已设置:\n"
              << "  - 频率: " << myRadar.freqMHz << " MHz\n"
              << "  - 发射功率: " << myRadar.xmtPowerW << " W\n"
              << "  - 天线增益: " << myRadar.antennaGaindBi << " dBi\n\n";
}
```

### 2. 自由空间雷达计算 - 典型的雷达检测功率计算

此示例模拟一个雷达探测一个特定距离和大小的目标，并计算雷达接收机收到的回波功率。

C++

```
void example_free_space_radar() {
    std::cout << "--- 2. 自由空间雷达计算 ---\n";
    
    // 输入参数
    double rangeMeters = 75000.0;      // 目标距离: 75 km
    double freqMHz = 9500.0;           // 频率: 9.5 GHz
    double powerWatts = 50000.0;       // 功率: 50 kW
    double gaindB = 40.0;              // 收发共用天线增益: 40 dB
    double rcsSqm = 2.0;               // 目标雷达截面积: 2 m^2
    double systemLossdB = 5.0;         // 系统损耗: 5 dB

    // 调用函数进行双向雷达计算 (oneWay = false)
    double rcvdPowerDb = simCore::getRcvdPowerFreeSpace(
        rangeMeters, freqMHz, powerWatts, gaindB, gaindB, rcsSqm, systemLossdB, false);

    std::cout << "雷达在 " << rangeMeters / 1000.0 << " km 处探测一个 " << rcsSqm << " m^2 的目标:\n"
              << "  - 接收到的回波功率: " << std::fixed << std::setprecision(2) << rcvdPowerDb << " dB\n\n";
}
```

### 3. Blake方程计算 - 考虑传播环境影响的修正计算

在自由空间计算的基础上，此示例加入了一个“模式传播因子”（PPF），用于模拟由多径、大气吸收等环境因素造成的额外损耗。

C++

```
void example_blake_equation() {
    std::cout << "--- 3. Blake方程计算 ---\n";
    
    // 参数与上例相同
    double rangeMeters = 75000.0, freqMHz = 9500.0, powerWatts = 50000.0;
    double gaindB = 40.0, rcsSqm = 2.0, systemLossdB = 5.0;
    
    // 新增参数：模式传播因子 (负值表示额外损耗)
    double ppfdB = -4.5; // -4.5 dB 的额外环境损耗

    // 调用Blake方程函数
    double rcvdPowerBlake = simCore::getRcvdPowerBlake(
        rangeMeters, freqMHz, powerWatts, gaindB, gaindB, rcsSqm, ppfdB, systemLossdB, false);

    std::cout << "考虑 " << ppfdB << " dB 的环境传播影响后:\n"
              << "  - 修正后的接收功率: " << rcvdPowerBlake << " dB\n\n";
}
```

### 4. 单程通信链路 - 通信系统的功率预算分析

此示例计算一个单向通信链路（如卫星下行链路或无人机数据链）在接收端能收到的信号功率。

C++

```
void example_one_way_link() {
    std::cout << "--- 4. 单程通信链路 ---\n";

    // 输入参数
    double linkRangeMeters = 150000.0; // 通信距离: 150 km
    double freqMHz = 2400.0;          // 频率: 2.4 GHz (S波段)
    double txPowerWatts = 20.0;       // 发射功率: 20 W
    double txGaindB = 15.0;           // 发射天线增益: 15 dB
    double rxGaindB = 25.0;           // 接收天线增益: 25 dB
    double systemLossdB = 3.0;        // 系统损耗: 3 dB

    // 调用函数进行单向链路计算 (oneWay = true)
    // 注意：rcsSqm 在单向计算中不起作用，但仍需传递一个值
    double rcvdPowerDb = simCore::getRcvdPowerFreeSpace(
        linkRangeMeters, freqMHz, txPowerWatts, txGaindB, rxGaindB, 0.0, systemLossdB, true);

    std::cout << "一个 " << linkRangeMeters / 1000.0 << " km 的单向通信链路:\n"
              << "  - 接收端功率: " << rcvdPowerDb << " dB\n\n";
}
```

### 5. ESM接收机检测 - 电子战中的截获距离计算

此示例模拟一个电子战支援措施（ESM）系统，计算其能探测到特定雷达信号的最大距离。

C++

```
void example_esm_detection() {
    std::cout << "--- 5. ESM接收机检测 ---\n";

    // 目标雷达参数
    double radarPowerWatts = 100000.0; // 敌方雷达功率: 100 kW
    double radarGaindB = 40.0;         // 敌方雷达主瓣增益: 40 dB
    double radarFreqMHz = 3000.0;      // 敌方雷达频率: 3 GHz

    // 我方ESM接收机参数
    double esmSensitivityDbm = -80.0;  // 接收机灵敏度: -80 dBm
    double pathLossDb = 0.0;           // 用于存储路径损耗的变量

    // 调用函数计算最大截获距离
    double interceptRangeMeters = simCore::getOneWayFreeSpaceRangeAndLoss(
        radarGaindB, radarFreqMHz, radarPowerWatts, esmSensitivityDbm, &pathLossDb);

    std::cout << "ESM系统对该雷达信号的理论最大截获距离:\n"
              << "  - 距离: " << interceptRangeMeters / 1000.0 << " km\n"
              << "  - 该距离上的路径损耗: " << pathLossDb << " dB\n\n";
}
```

### 6. 传播因子转换 - 损耗与传播因子的相互转换

此示例展示了在已知总路径损耗时，如何反向计算出模式传播因子（PPF）。

C++

```
void example_loss_to_ppf() {
    std::cout << "--- 6. 传播因子转换 ---\n";

    // 输入参数
    double slantRange = 80000.0; // 斜距: 80 km
    double freqMHz = 5600.0;     // 频率: 5.6 GHz (C波段)
    
    // 假设通过测量或其他模型得到总路径损耗为 155 dB
    double totalLossDb = 155.0;

    // 调用函数，从总损耗中计算PPF
    double ppfDb = simCore::lossToPpf(slantRange, freqMHz, totalLossDb);

    std::cout << "在 " << slantRange / 1000.0 << " km 距离和 " << freqMHz << " MHz 频率下:\n"
              << "  - 若总路径损耗为 " << totalLossDb << " dB\n"
              << "  - 则模式传播因子 (PPF) 为: " << ppfDb << " dB\n\n";
}
```

### 7. 频段分类应用 - 自动频段识别和分类

此示例展示如何使用代码库自动识别一个给定的频率属于哪个IEEE雷达频段。

C++

```
void example_band_classification() {
    std::cout << "--- 7. 频段分类应用 ---\n";
    
    double freqToClassify = 15500.0; // 15.5 GHz

    // 调用函数获取频段分类
    simCore::FrequencyBandIEEE band = simCore::toIeeeBand(freqToClassify, false);

    std::cout << "频率 " << freqToClassify << " MHz 被分类为:\n"
              << "  - " << getIeeeBandString(band) << "\n\n";
}
```

### 8. 频段范围查询 - 获取特定频段的频率范围

与上一个示例相反，此示例展示了如何根据一个已知的频段名称（枚举），查询其对应的起止频率。

C++

```
void example_band_range_query() {
    std::cout << "--- 8. 频段范围查询 ---\n";

    // 选择要查询的频段
    simCore::FrequencyBandIEEE bandToQuery = simCore::IEEE_FREQ_S;
    
    double minFreq, maxFreq;

    // 调用函数获取频率范围
    simCore::getFreqMhzRange(bandToQuery, &minFreq, &maxFreq);

    std::cout << "查询的频段 " << getIeeeBandString(bandToQuery) << " 对应的频率范围是:\n"
              << "  - " << minFreq << " MHz 到 " << maxFreq << " MHz\n\n";
}
```

### 9. 实际系统分析 - 模拟宙斯盾雷达的性能分析

这是一个综合性示例，使用公开的宙斯盾（Aegis）SPY-1雷达的大致参数，对其探测典型战斗机的能力进行初步、简化的性能分析。

**免责声明**: 此分析基于公开的、非保密的典型数据，仅用于演示代码功能，不代表任何实际装备的精确性能。

C++

```
void example_aegis_spy1_analysis() {
    std::cout << "--- 9. 实际系统分析 - 模拟宙斯盾雷达性能 ---\n";
    
    // 1. 设置SPY-1雷达的大致参数（基于公开数据）
    simCore::RadarParameters spy1_params;
    spy1_params.freqMHz = 3500.0;      // S波段 (2-4 GHz), 取一个中间值
    spy1_params.xmtPowerKW = 4000.0;   // 峰值功率: ~4 MW (兆瓦)
    spy1_params.xmtPowerW = spy1_params.xmtPowerKW * 1000.0;
    spy1_params.antennaGaindBi = 45.0; // 高增益相控阵天线 (估算值)
    spy1_params.systemLossdB = 6.0;    // 系统损耗 (估算值)

    // 2. 定义探测目标和探测距离
    double targetRcsSqm = 1.0;         // 典型隐身战斗机雷达截面积: 1 m^2
    double analysisRangeKm = 350.0;    // 分析距离: 350 km
    double analysisRangeMeters = analysisRangeKm * 1000.0;

    // 3. 计算在该距离上的接收功率
    double rcvdPowerDb = simCore::getRcvdPowerFreeSpace(
        analysisRangeMeters,
        spy1_params.freqMHz,
        spy1_params.xmtPowerW,
        spy1_params.antennaGaindBi,
        spy1_params.antennaGaindBi,
        targetRcsSqm,
        spy1_params.systemLossdB,
        false
    );

    // 4. 分析结果
    // 假设雷达的最小可探测信号(MDS)为 -100 dB (这是一个合理的估算值)
    double minDetectableSignalDb = -100.0;

    std::cout << "对SPY-1雷达进行性能分析:\n"
              << "  - 目标: " << targetRcsSqm << " m^2 RCS\n"
              << "  - 距离: " << analysisRangeKm << " km\n"
              << "计算得到的回波功率: " << rcvdPowerDb << " dB\n"
              << "假设的最小探测门限: " << minDetectableSignalDb << " dB\n";

    if (rcvdPowerDb > minDetectableSignalDb) {
        std::cout << "结论: 在此距离上，回波信号强度高于探测门限，理论上可以探测到目标。\n\n";
    } else {
        std::cout << "结论: 在此距离上，回波信号强度低于探测门限，理论上无法探测到目标。\n\n";
    }
}
```

### 主函数

最后，在 `main` 函数中调用所有示例。

C++

```
int main() {
    std::cout << std::fixed << std::setprecision(2);

    example_radar_parameters_usage();
    example_free_space_radar();
    example_blake_equation();
    example_one_way_link();
    example_esm_detection();
    example_loss_to_ppf();
    example_band_classification();
    example_band_range_query();
    example_aegis_spy1_analysis();

    return 0;
}
```