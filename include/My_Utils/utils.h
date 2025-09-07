#ifndef UTILS_H
#define UTILS_H

#include "kernel.h"
class utils {
private:    
    
public:    
    int result;
    void getGame();
    bool isjoin(const std::string &num);
    char* packageNames[3] = {"com.netease.yhtj", "com.tencent.tmgp.codev", "com.larus.nova"};
    
};

#endif  // UTILS_H
