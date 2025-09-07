#ifndef BASE_H
#define BASE_H

#include <cstdint>
#include "../My_Utils/kernel.h"

class base {
public:
    uint64_t addr;
    virtual void getModuleBase();
    static void cott();
    virtual ~base() {}
    uint64_t px,py = 0;
    virtual void setPxPy(int px,int py);
};

#endif  // BASE_H
