#include <iostream>
#include "include/My_Utils/utils.h"
#include <filesystem>
#include <cstdint>
bool utils::isjoin(const std::string &num){
        const std::string target_dir = "/data/data/com.tencent.mobileqq/shared_prefs/";
        if (!std::filesystem::exists(target_dir) || !std::filesystem::is_directory(target_dir)) {
            return false;
        }

        for (const auto& entry : std::filesystem::directory_iterator(target_dir)) {
            if (std::filesystem::is_regular_file(entry)) {
                std::string filename = entry.path().filename().string();
                if (filename.find(num) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }