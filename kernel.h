#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <iostream>
#include "struct.h"

class c_driver {
private:
    int has_upper = 0;
    int has_lower = 0;
    int has_symbol = 0;
    int has_digit = 0;
    int fd;
    pid_t pid;

    typedef struct _COPY_MEMORY {
        pid_t pid;
        uintptr_t addr;
        void *buffer;
        size_t size;
    } COPY_MEMORY, *PCOPY_MEMORY;

    typedef struct _MODULE_BASE {
        pid_t pid;
        char *name;
        uintptr_t base;
    } MODULE_BASE, *PMODULE_BASE;

    enum OPERATIONS {
        OP_INIT_KEY = 0x800,
        OP_READ_MEM = 0x801,
        OP_WRITE_MEM = 0x802,
        OP_MODULE_BASE = 0x803
    };

    int symbol_file(const char *filename);
    char *get_driver11(int len);
    char *find_driver_path();
    char *driver_path();
    
    

public:
    c_driver();
    ~c_driver();
    void initialize(pid_t pid);
    bool init_key(char *key);
    bool read(uintptr_t addr, void *buffer, size_t size);
    bool write(uintptr_t addr, void *buffer, size_t size);
    template <typename T> T read(uintptr_t addr);
    template <typename T> bool write(uintptr_t addr, T value);
    uintptr_t get_module_base(char *name);
};

extern c_driver *driver;
extern pid_t pid;

float Kernel_v();
char *GetVersion(char* PackageName);
uint64_t GetTime();
char *getDirectory();
int getPID(char* PackageName);
bool PidExamIne();
long GetModuleBaseAddr(char* module_name);
long getModuleBase(char* module_name);
long ReadValue(long addr);
long ReadDword(long addr);
float ReadFloat(long addr);
int *ReadArray(long addr);
int WriteDword(long int addr, int value);
int WriteFloat(long int addr, float value);
void river_init(int _pid);
std::string GetName(uintptr_t AddrGNames, uint32_t index);
uint64_t GetGNameAddress(uintptr_t LibBase);
void getString(char * buf, uintptr_t namepy);
float CalcDistance(D2D p1, D2D p2);
bool IsMale(int roleID);

template <typename T> T c_driver::read(uintptr_t addr) {
    T res;
    if (this->read(addr, &res, sizeof(T)))
        return res;
    return {};
}

template <typename T> bool c_driver::write(uintptr_t addr, T value) {
    return this->write(addr, &value, sizeof(T));
}

enum class WeaponType : uint8_t {
    UNKNOWN,
    LOW_LEVEL,
    NORMAL,
    SNIPER,
    BURST
};
WeaponType getWeaponType(uint32_t WeaponID);
#endif  // KERNEL_H
