#ifndef UTILS_H  // 防止头文件重复包含（关键）
#define UTILS_H

#include "kernel.h"
class utils {
private:    
    
public:    
    int result;
    void getGame();
    char* packageNames[3] = {"com.netease.yhtj", "com.tencent.tmgp.codev", "com.larus.nova"};
    
};

#endif  // UTILS_H
