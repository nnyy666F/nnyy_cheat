#include <iostream>
#include "include/My_Utils/utils.h"

void utils::getGame() {
    const int kPackageCount = sizeof(packageNames) / sizeof(packageNames[0]);
    
    int pidArray[kPackageCount] = {-1, -1, -1};
    int validCount = 0;

    for (int i = 0; i < kPackageCount; i++) {
        pidArray[i] = getPID(packageNames[i]);
        printf("%d",pidArray[i]);
        if (pidArray[i] > 0) {
            validCount++;
        }
    }
    
    if (validCount == 0) {
        std::cout<<"没有游戏被打开......";
        return;
    } 
    else if (validCount == 1) {
        // 单一进程
        for (int i = 0; i < kPackageCount; i++) {
            if (pidArray[i] > 0) {
                std::cout<<"go......";
                river_init(pidArray[i]);
                break;
            }
        }
    } 
    else {
        // 多个有效PID，提示用户手动选择
        std::cout << "检测到多个运行中的游戏进程，请选择：" << std::endl;
        for (int i = 0; i < kPackageCount; i++) {
            if (pidArray[i] > 0) {
                std::cout << i + 1 << ". 包名：" << packageNames[i] << " | PID：" << pidArray[i] << std::endl;
            }
        }

        // 读取用户选择（简单输入校验）
        int userChoice = 0;
        while (true) {
            std::cout << "请输入选择的序号（1-" << validCount << "）：";
            std::cin >> userChoice;

            // 校验选择是否有效
            int tempCount = 0;
            for (int i = 0; i < kPackageCount; i++) {
                if (pidArray[i] > 0) {
                    tempCount++;
                    if (tempCount == userChoice) {
                        river_init(pidArray[i]);
                    }
                }
            }
            std::cout << "输入序号无效，请重新选择！" << std::endl;
        }
    }
}