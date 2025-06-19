#include <iostream>
#include <iomanip>
#include "simCore/EM/Propagation.h"

int main()
{
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== RF传播计算库调用示例 ===\n\n";

    // ========================================
    // 示例1: 雷达参数结构体的使用
    // ========================================
    std::cout << "1. 雷达参数结构体示例:\n";
    simCore::RadarParameters radar;
    radar.freqMHz = 3000.0;        // X波段雷达，3GHz
    radar.antennaGaindBi = 30.0;   // 天线增益30dBi
    radar.noiseFiguredB = 3.0;     // 噪声系数3dB
    radar.pulseWidth_uSec = 1.0;   // 脉冲宽度1微秒
    radar.systemLossdB = 5.0;      // 系统损耗5dB
    radar.xmtPowerKW = 100.0;      // 发射功率100kW
    radar.xmtPowerW = radar.xmtPowerKW * 1000.0;  // 转换为瓦特
    radar.hbwD = 3.0;              // 水平波束宽度3度

    std::cout << "   频率: " << radar.freqMHz << " MHz\n";
    std::cout << "   发射功率: " << radar.xmtPowerKW << " kW (" << radar.xmtPowerW << " W)\n";
    std::cout << "   天线增益: " << radar.antennaGaindBi << " dBi\n\n";

    // ========================================
    // 示例2: 自由空间雷达功率计算
    // ========================================
    std::cout << "2. 自由空间雷达功率计算示例:\n";
    
    // 雷达参数
    double range_m = 50000.0;      // 目标距离50km
    double freq_mhz = 3000.0;      // X波段3GHz
    double xmt_power_w = 100000.0; // 发射功率100kW
    double xmt_gain_db = 35.0;     // 发射天线增益35dB
    double rcv_gain_db = 35.0;     // 接收天线增益35dB (通常与发射相同)
    double target_rcs_sqm = 10.0;  // 目标RCS 10平方米
    double system_loss_db = 8.0;   // 系统损耗8dB

    double rcvd_power_db = simCore::getRcvdPowerFreeSpace(
        range_m, freq_mhz, xmt_power_w, xmt_gain_db, 
        rcv_gain_db, target_rcs_sqm, system_loss_db, false);

    std::cout << "   目标距离: " << range_m/1000.0 << " km\n";
    std::cout << "   目标RCS: " << target_rcs_sqm << " m²\n";
    std::cout << "   接收功率(自由空间): " << rcvd_power_db << " dB\n\n";

    // ========================================
    // 示例3: Blake方程功率计算
    // ========================================
    std::cout << "3. Blake方程功率计算示例:\n";
    
    double ppf_db = -6.0;  // 传播因子-6dB (考虑多径、大气等影响)
    
    double rcvd_power_blake = simCore::getRcvdPowerBlake(
        range_m, freq_mhz, xmt_power_w, xmt_gain_db, 
        rcv_gain_db, target_rcs_sqm, ppf_db, system_loss_db, false);

    std::cout << "   传播因子: " << ppf_db << " dB\n";
    std::cout << "   接收功率(Blake方程): " << rcvd_power_blake << " dB\n";
    std::cout << "   与自由空间差异: " << (rcvd_power_blake - rcvd_power_db) << " dB\n\n";

    // ========================================
    // 示例4: 单程通信链路计算
    // ========================================
    std::cout << "4. 单程通信链路计算示例:\n";
    
    double comm_range_m = 100000.0;    // 通信距离100km
    double comm_freq_mhz = 1500.0;     // L波段1.5GHz
    double comm_power_w = 1000.0;      // 发射功率1kW
    double tx_gain_db = 20.0;          // 发射天线增益20dB
    double rx_gain_db = 15.0;          // 接收天线增益15dB
    double comm_loss_db = 3.0;         // 通信系统损耗3dB

    double comm_rcvd_power = simCore::getRcvdPowerFreeSpace(
        comm_range_m, comm_freq_mhz, comm_power_w, tx_gain_db, 
        rx_gain_db, 1.0, comm_loss_db, true);  // oneWay=true, rcs=1.0(忽略)

    std::cout << "   通信距离: " << comm_range_m/1000.0 << " km\n";
    std::cout << "   工作频率: " << comm_freq_mhz << " MHz\n";
    std::cout << "   接收功率: " << comm_rcvd_power << " dB\n\n";

    // ========================================
    // 示例5: ESM接收机检测距离计算
    // ========================================
    std::cout << "5. ESM接收机检测距离计算示例:\n";
    
    double esm_xmt_gain_db = 25.0;     // 被探测雷达天线增益25dB
    double esm_freq_mhz = 9000.0;      // X波段9GHz
    double esm_power_w = 50000.0;      // 被探测雷达功率50kW
    double esm_sensitivity_dbm = -70.0; // ESM接收机灵敏度-70dBm
    double fs_loss_db = 0.0;           // 输出的自由空间损耗

    double detection_range_m = simCore::getOneWayFreeSpaceRangeAndLoss(
        esm_xmt_gain_db, esm_freq_mhz, esm_power_w, 
        esm_sensitivity_dbm, &fs_loss_db);

    std::cout << "   被探测雷达功率: " << esm_power_w/1000.0 << " kW\n";
    std::cout << "   ESM接收机灵敏度: " << esm_sensitivity_dbm << " dBm\n";
    std::cout << "   检测距离: " << detection_range_m/1000.0 << " km\n";
    std::cout << "   自由空间损耗: " << fs_loss_db << " dB\n\n";

    // ========================================
    // 示例6: 传播因子转换
    // ========================================
    std::cout << "6. 传播因子转换示例:\n";
    
    double slant_range_m = 75000.0;    // 斜距75km
    double path_freq_mhz = 5000.0;     // C波段5GHz
    double path_loss_db = 180.0;       // 路径损耗180dB
    
    double ppf_calculated = simCore::lossToPpf(slant_range_m, path_freq_mhz, path_loss_db);
    
    std::cout << "   斜距: " << slant_range_m/1000.0 << " km\n";
    std::cout << "   频率: " << path_freq_mhz << " MHz\n";
    std::cout << "   路径损耗: " << path_loss_db << " dB\n";
    std::cout << "   传播因子: " << ppf_calculated << " dB\n\n";

    // ========================================
    // 示例7: 频段分类示例
    // ========================================
    std::cout << "7. 频段分类示例:\n";
    
    double test_frequencies[] = {100.0, 500.0, 1500.0, 3000.0, 9000.0, 15000.0, 35000.0};
    int num_freqs = sizeof(test_frequencies) / sizeof(test_frequencies[0]);
    
    std::cout << "   频率(MHz)  | US ECM频段 | IEEE频段\n";
    std::cout << "   --------------------------------\n";
    
    for (int i = 0; i < num_freqs; i++)
    {
        double freq = test_frequencies[i];
        
        // US ECM频段分类
        simCore::FrequencyBandUsEcm usEcmBand = simCore::toUsEcm(freq);
        char usEcmName = 'A' + (usEcmBand - 1);  // 转换为字母
        
        // IEEE频段分类
        simCore::FrequencyBandIEEE ieeeBand = simCore::toIeeeBand(freq);
        std::string ieeeName;
        switch(ieeeBand)
        {
            case simCore::IEEE_FREQ_HF: ieeeName = "HF"; break;
            case simCore::IEEE_FREQ_VHF: ieeeName = "VHF"; break;
            case simCore::IEEE_FREQ_UHF: ieeeName = "UHF"; break;
            case simCore::IEEE_FREQ_L: ieeeName = "L"; break;
            case simCore::IEEE_FREQ_S: ieeeName = "S"; break;
            case simCore::IEEE_FREQ_C: ieeeName = "C"; break;
            case simCore::IEEE_FREQ_X: ieeeName = "X"; break;
            case simCore::IEEE_FREQ_KU: ieeeName = "Ku"; break;
            case simCore::IEEE_FREQ_K: ieeeName = "K"; break;
            case simCore::IEEE_FREQ_KA: ieeeName = "Ka"; break;
            case simCore::IEEE_FREQ_V: ieeeName = "V"; break;
            case simCore::IEEE_FREQ_W: ieeeName = "W"; break;
            default: ieeeName = "Unknown"; break;
        }
        
        std::cout << "   " << std::setw(8) << freq << "   |     " 
                  << usEcmName << "      |   " << ieeeName << "\n";
    }
    
    // ========================================
    // 示例8: 频段范围查询
    // ========================================
    std::cout << "\n8. 频段范围查询示例:\n";
    
    double min_freq, max_freq;
    
    // 查询X波段范围
    simCore::getFreqMhzRange(simCore::IEEE_FREQ_X, &min_freq, &max_freq);
    std::cout << "   IEEE X波段范围: " << min_freq << " - " << max_freq << " MHz\n";
    
    // 查询US ECM G段范围  
    simCore::getFreqMhzRange(simCore::USECM_FREQ_G, &min_freq, &max_freq);
    std::cout << "   US ECM G段范围: " << min_freq << " - " << max_freq << " MHz\n";

    // ========================================
    // 示例9: 实际雷达系统分析
    // ========================================
    std::cout << "\n9. 实际雷达系统综合分析:\n";
    
    // 模拟AN/SPY-1雷达参数
    std::cout << "   模拟宙斯盾雷达系统分析:\n";
    double spy1_freq = 3300.0;        // S波段3.3GHz
    double spy1_power = 4000000.0;     // 峰值功率4MW
    double spy1_gain = 42.0;           // 阵列天线增益42dB
    double spy1_loss = 6.0;            // 系统损耗6dB
    
    // 计算不同距离下的接收功率
    double ranges[] = {50000, 100000, 200000, 400000};  // 50, 100, 200, 400 km
    double aircraft_rcs = 5.0;  // 典型战斗机RCS 5平方米
    
    std::cout << "   目标RCS: " << aircraft_rcs << " m²\n";
    std::cout << "   距离(km) | 接收功率(dB) | S/N比估算\n";
    std::cout << "   ------------------------------------\n";
    
    for (int i = 0; i < 4; i++)
    {
        double range = ranges[i];
        double rcvd_pwr = simCore::getRcvdPowerFreeSpace(
            range, spy1_freq, spy1_power, spy1_gain, 
            spy1_gain, aircraft_rcs, spy1_loss, false);
        
        // 简单的信噪比估算 (假设噪声底-110dBm)
        double noise_floor = -110.0;
        double snr = rcvd_pwr - noise_floor;
        
        std::cout << "   " << std::setw(6) << range/1000.0 
                  << "   |    " << std::setw(6) << rcvd_pwr 
                  << "    |   " << std::setw(4) << snr << " dB\n";
    }

    std::cout << "\n=== 示例程序执行完成 ===\n";
    return 0;
}