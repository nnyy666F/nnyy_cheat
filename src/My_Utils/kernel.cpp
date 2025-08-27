#include "include/My_Utils/kernel.h"
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sstream>
#include <string>
#include <map>
#include <cstdint>
#include <unordered_set>

// 全局变量定义
c_driver *driver = new c_driver();
pid_t pid;
pid_t temp_tempPID = -99999;

// c_driver 类私有成员函数实现
int c_driver::symbol_file(const char *filename) {
    // 判断文件名是否含小写并且不含大写不含数字不含符号
    int length = strlen(filename);
    for (int i = 0; i < length; i++) {
        if (islower(filename[i])) {
            has_lower = 1;
        } else if (isupper(filename[i])) {
            has_upper = 1;
        } else if (ispunct(filename[i])) {
            has_symbol = 1;
        } else if (isdigit(filename[i])) {
            has_digit = 1;
        }
    }
    return has_lower && !has_upper && !has_symbol && !has_digit;
}

char* c_driver::get_driver11(int len)
{
    DIR *dir;
    struct dirent *ent;
    struct dirent *ent2;
    FILE *file;
    FILE *file2;
    int isrun = 1;
    char path[256], cmdline[256], comm[256], link[256], sbwj[256], open_file[256], 
         sbid[256];
    char path2[256];
    char path3[256];
    char cmd[100];
    char* result = NULL;
    dir = opendir("/proc");
    if (dir != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_DIR)
            {
                snprintf(path, sizeof(path), "/proc/%s/cmdline", ent->d_name);
                snprintf(comm, sizeof(comm), "/proc/%s/comm", ent->d_name);

                file = fopen(path, "r");
                if (file != NULL)
                {
                    fgets(cmdline, sizeof(cmdline), file);
                    fclose(file);

                    file = fopen(comm, "r");
                    if (file != NULL)
                    {
                        fgets(comm, sizeof(comm), file);
                        fclose(file);
                        if (strstr(cmdline, "/data/") && strlen(cmdline) - 6 == len)
                        {
                            snprintf(sbwj, sizeof(sbwj), "%s", comm);

                            snprintf(path2, sizeof(path2), "/proc/%s/fd", ent->d_name);
                            DIR *fd_dir = opendir(path2);
                            if (fd_dir == NULL) {
                                perror("opendir fd_dir failed");
                                break;
                            }
                            while (isrun && ((ent2 = readdir(fd_dir)) != NULL))
                            {
                                if (ent2->d_type == DT_LNK)
                                {
                                    snprintf(path3, sizeof(path3), "/proc/%s/fd/%s", ent->d_name,
                                             ent2->d_name);
                                    readlink(path3, link, sizeof(link));
                                    std::string q = "";
                                    for (int i = 0; i < len; i++)
                                    {
                                        q += sbwj[i];
                                    }
                                    std::string a1 = q;
                                    std::string aa = "/dev/" + a1 + " (deleted)";
                                    if (strstr(link, aa.c_str()))
                                    {
                                        snprintf(open_file, sizeof(open_file), "/proc/%s/fd/%s",
                                                 ent->d_name, ent2->d_name);
                                        isrun = 0;
                                        sprintf(sbwj, a1.c_str());
                                    }
                                }
                            }
                            closedir(fd_dir);
                            if (strlen(open_file) > 0)
                            {
                                printf("驱动:/dev/%s\n", sbwj);
                                std::string a = sbwj;
                                std::string fb = "/dev/" + a;
                                std::ostringstream oss;
                                oss << "sbid=$(ls -L -l " << open_file << " | sed 's/\\([^,]*\\).*/\\1/' | sed 's/.*root //');" << " echo \"/dev/" << a << "\";" << " rm -Rf \"/dev/" << a << "\";" << " mknod \"/dev/" << a << "\" c \"$sbid\" 0;";
                                std::string m = oss.str();
                                const char* cmd = m.c_str();

                                printf("%s", cmd);
                                FILE *fp = popen(cmd, "r");
                                char buffer[128];  
                                if (fp == NULL) {  
                                    perror("popen failed");  
                                    closedir(dir);
                                    result = (char*)malloc(1);
                                    if (result != NULL) *result = '\0';
                                    return result;
                                }  
                                while (fgets(buffer, sizeof(buffer), fp) != NULL) {  
                                    printf("%s", buffer);  
                                }  
                                pclose(fp);  

                                result = (char*)malloc(fb.size() + 1);
                                if (result != NULL)
                                {
                                    strcpy(result, fb.c_str());
                                }
                                closedir(dir);
                                return result;
                            }
                            else
                            {
                                printf("没有刷入驱动\n");
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }
        }
        closedir(dir);
    }

    result = (char*)malloc(1);
    if (result != NULL)
    {
        *result = '\0';
    }
    return result;
}

char* c_driver::driver_path() {
		struct dirent* de;
		DIR* dr = opendir("/proc");
		char* device_path = NULL;

		if (dr == NULL) {
			printf("Could not open /proc directory");
			return NULL;
		}



		while ((de = readdir(dr)) != NULL) {
			if (strlen(de->d_name) != 6 || strcmp(de->d_name, "NVTSPI") == 0 || strcmp(de->d_name, "ccci_log") == 0 || strcmp(de->d_name, "aputag") == 0 || strcmp(de->d_name, "asound") == 0 || strcmp(de->d_name, "clkdbg") == 0 || strcmp(de->d_name, "crypto") == 0 || strcmp(de->d_name, "modules") == 0 || strcmp(de->d_name, "mounts") == 0 || strcmp(de->d_name, "pidmap") == 0 || strcmp(de->d_name, "phoenix") == 0 || strcmp(de->d_name, "uptime") == 0 || strcmp(de->d_name, "vmstat") == 0) {
				continue;
			}
			int is_valid = 1;
			for (int i = 0; i < 6; i++) {
				if (!isalnum(de->d_name[i])) {
					is_valid = 0;
					break;
				}
			}
			if (is_valid) {
				device_path = (char*)malloc(11 + strlen(de->d_name));
				sprintf(device_path, "/proc/%s", de->d_name);
				struct stat sb;
				if (stat(device_path, &sb) == 0 && S_ISREG(sb.st_mode)) {
					break;
				}
				else {
					free(device_path);
					device_path = NULL;
				}
			}
		}
		puts(device_path);
		closedir(dr);
		return device_path;
	}

char *c_driver::find_driver_path() {
    // 打开目录
    const char *dev_path = "/dev";
    DIR *dir = opendir(dev_path);
    if (dir == NULL){
        printf("无法打开/dev目录\n");
        return NULL;
    }

    char *files[] = { "wanbai", "CheckMe", "Ckanri", "lanran","video188"};
    struct dirent *entry;
    char *file_path = NULL;
    while ((entry = readdir(dir)) != NULL) {
        // 跳过当前目录和上级目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        size_t path_length = strlen(dev_path) + strlen(entry->d_name) + 2;
        file_path = (char *)malloc(path_length);
        snprintf(file_path, path_length, "%s/%s", dev_path, entry->d_name);
        for (int i = 0; i < 5; i++) {
            if (strcmp(entry->d_name, files[i]) == 0) {
                //printf("驱动文件：%s\n", file_path);
                closedir(dir);
                return file_path;
            }
        }

        // 获取文件stat结构
        struct stat file_info;
        if (stat(file_path, &file_info) < 0) {
            free(file_path);
            file_path = NULL;
            continue;
        }

        // 跳过gpio接口
        if (strstr(entry->d_name, "gpiochip") != NULL) {
            free(file_path);
            file_path = NULL;
            continue;
        }

        // 检查是否为驱动文件
        if ((S_ISCHR(file_info.st_mode) || S_ISBLK(file_info.st_mode))
            && strchr(entry->d_name, '_') == NULL && strchr(entry->d_name, '-') == NULL 
            && strchr(entry->d_name, ':') == NULL) {
            // 过滤标准输入输出
            if (strcmp(entry->d_name, "stdin") == 0 || strcmp(entry->d_name, "stdout") == 0
                || strcmp(entry->d_name, "stderr") == 0) {
                free(file_path);
                file_path = NULL;
                continue;
            }
            
            size_t file_name_length = strlen(entry->d_name);
            time_t current_time;
            time(&current_time);
            int current_year = localtime(&current_time)->tm_year + 1900;
            int file_year = localtime(&file_info.st_ctime)->tm_year + 1900;
            // 跳过1980年前的文件
            if (file_year <= 1980) {
                free(file_path);
                file_path = NULL;
                continue;
            }
            
            time_t atime = file_info.st_atime;
            time_t ctime = file_info.st_ctime;
            // 检查最近访问时间和修改时间是否一致并且文件名是否是symbol文件
            if ((atime == ctime)/* && symbol_file(entry->d_name)*/) {
                // 检查mode权限类型是否为S_IFREG(普通文件)和大小还有gid和uid是否为0(root)并且文件名称长度在7位或7位以下
                if ((file_info.st_mode & S_IFMT) == 8192 && file_info.st_size == 0
                    && file_info.st_gid == 0 && file_info.st_uid == 0 && file_name_length <= 9) {
                    //printf("驱动文件：%s\n", file_path);
                    closedir(dir);
                    return file_path;
                }
            }
        }
        free(file_path);
        file_path = NULL;
    }
    closedir(dir);
    return NULL;
}

// c_driver 类公有成员函数实现
c_driver::c_driver() {
    char *device_name;
    device_name= find_driver_path();
    if (device_name == NULL)  {
        device_name = get_driver11(6);
        if (device_name == NULL) {
            device_name = driver_path();
        }
    }
    if (!device_name) {
        fprintf(stderr, "未找到驱动文件\n");
    }

    fd = open(device_name, O_RDWR);
    free(device_name);

    if (fd == -1) {
        perror("[-] 链接驱动失败");
       // exit(EXIT_FAILURE);
    }
}

c_driver::~c_driver() {
    // wont be called
    if (fd > 0)
        close(fd);
}

void c_driver::initialize(pid_t pid) { 
    this->pid = pid; 
}

bool c_driver::init_key(char *key) {
    char buf[0x100];
    strcpy(buf, key);
    if (ioctl(fd, OP_INIT_KEY, buf) != 0) {
        return false;
    }
    return true;
}

bool c_driver::read(uintptr_t addr, void *buffer, size_t size) {
    COPY_MEMORY cm;

    cm.pid = this->pid;
    cm.addr = addr;
    cm.buffer = buffer;
    cm.size = size;

    if (ioctl(fd, OP_READ_MEM, &cm) != 0) {
      return false;
    }
    return true;
}

bool c_driver::write(uintptr_t addr, void *buffer, size_t size) {
    COPY_MEMORY cm;

    cm.pid = this->pid;
    cm.addr = addr;
    cm.buffer = buffer;
    cm.size = size;

    if (ioctl(fd, OP_WRITE_MEM, &cm) != 0) {
        return false;
    }
    return true;
}

uintptr_t c_driver::get_module_base(char *name) {
    MODULE_BASE mb;
    char buf[0x100];
    strcpy(buf, name);
    mb.pid = this->pid;
    mb.name = buf;

    if (ioctl(fd, OP_MODULE_BASE, &mb) != 0) {
        return 0;
    }
    return mb.base;
}

// 全局函数实现
float Kernel_v()
{
    const char* command = "uname -r | sed 's/\\.[^.]*$//g'";
    FILE* file = popen(command, "r");
    if (file == NULL) {
        return 0.0f;  // 修复：返回合理的默认值而非NULL（float不能为NULL）
    }
    static char result[512];
    if (fgets(result, sizeof(result), file) == NULL) {
        pclose(file);
        return 0.0f;  // 修复：处理读取失败的情况
    }
    pclose(file);
    result[strlen(result)-1] = '\0';
    return atof(result);
}

char *GetVersion(char* PackageName)
{
    char command[256];
    sprintf(command, "dumpsys package %s|grep versionName|sed 's/=/\\n/g'|tail -n 1", PackageName);
    FILE* file = popen(command, "r");
    if (file == NULL) {
        return NULL;
    }
    static char result[512];
    if (fgets(result, sizeof(result), file) == NULL) {
        pclose(file);
        return NULL;
    }
    pclose(file);
    result[strlen(result)-1] = '\0';
    return result;
}

uint64_t GetTime()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    return (ts.tv_sec*1000 + ts.tv_nsec/(1000*1000));
}

char *getDirectory()
{
    static char buf[128];
    int rslt = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (rslt < 0 || (rslt >= sizeof(buf) - 1))
    {
        return NULL;
    }
    buf[rslt] = '\0';
    for (int i = rslt; i >= 0; i--)
    {
        if (buf[i] == '/')
        {
            buf[i] = '\0';
            break;
        }
    }
    return buf;
}

int getPID(char* PackageName)
{
    int _tempPID = -1;
    FILE* fp;
    char cmd[0x100] = "pidof ";
    strcat(cmd, PackageName);
    if (fp == NULL) {
        perror("popen pidof failed");
        return -1;
    }
    fp = popen(cmd,"r");
    if (fscanf(fp,"%d", &_tempPID) != 1) {
        _tempPID = -1;
    }
    pclose(fp);   
    if(_tempPID == temp_tempPID){return 0;}
    temp_tempPID = _tempPID;
    return _tempPID;
}

void river_init(int _tempPID) {
    if (_tempPID > 0)
    {
        pid = _tempPID;
        driver->initialize(_tempPID);
    }
}

bool PidExamIne()
{
    char path[128];
    sprintf(path, "/proc/%d", pid);
    if (access(path, F_OK) != 0)
    {
        printf("\033[31;1m");
        puts("获取进程PID失败!");
        exit(1);
    }
    return true;
}

long GetModuleBaseAddr(char* module_name)
{
    long addr = 0;
    char filename[32];
    char line[1024];
    if (pid < 0)
    {
        snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
    }
    else
    {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }
    FILE *fp = fopen(filename, "r");
    if (fp != NULL)
    {
        while (fgets(line, sizeof(line), fp))
        {
            if (strstr(line, module_name))
            {
                sscanf(line,"%lx-%*lx",&addr);
                break;
            }
        }
        fclose(fp);
    }
    return addr;
}

long getModuleBase(char* module_name)
{
    uintptr_t base=0;
    if (Kernel_v() >= 6.0)
        base = GetModuleBaseAddr(module_name);
    else
        base = driver->get_module_base(module_name);
    return base;
}

long ReadValue(long addr)
{
    long he=0;
    if (addr < 0xFFFFFFFF){
        driver->read(addr, &he, 4);
    } else {
        driver->read(addr, &he, 8);
        he=he&0xFFFFFFFFFFFF;
    }
    return he& 0xFFFFFFFFFFFFFF;
}

long ReadDword(long addr)
{
    long he=0;
    driver->read(addr, &he, 4);
    return he;
}

float ReadFloat(long addr)
{
    float he=0;
    driver->read(addr, &he, 4);
    return he;
}

int *ReadArray(long addr)
{
    int *he = (int *) malloc(12);
    driver->read(addr, he, 12);
    return he;
}

int WriteDword(long int addr, int value)
{
    driver->write(addr, &value, 4);
    return 0;
}

int WriteFloat(long int addr, float value)
{
    driver->write(addr, &value, 4);
    return 0;
}

std::string GetName(uintptr_t AddrGNames, uint32_t index) {
	uint32_t block = index >> 16;
	uint16_t offset = index & 0xFFFF;

	uintptr_t gnames = AddrGNames;
	uint64_t xorKey = ReadValue(gnames + 0x10040);
	
	uintptr_t chunkPtrAddr = ReadValue(gnames + 0x40 + block * 8);

	if (block == 0)
		chunkPtrAddr ^= xorKey;

	uintptr_t entryAddr = chunkPtrAddr + 2 * offset;
	uint16_t header = ReadValue(entryAddr);
	uintptr_t strPtr = entryAddr + 2;

	uint32_t strLen = header >> 6;
	bool isWide = header & 1;

	if (strLen == 0 || strLen > 200)
		return "None";

	std::string result;

	if (isWide) {
		std::u16string u16buf(strLen, u'\0');
		driver->read(strPtr, u16buf.data(), strLen * 2);
		result = std::string(u16buf.begin(), u16buf.end());
	}
	else {
		result.resize(strLen);
		driver->read(strPtr, result.data(), strLen);
	}

	return result;
}

uint64_t GetGNameAddress(uintptr_t LibBase) {
	const uintptr_t Address = ReadValue(ReadValue(LibBase + 0xA62fee8)+8)+0x690;
	const uintptr_t indicesBase = Address + 0x3F0; // 偏移表
	const uintptr_t byteArrayAddr = Address + 0x370;// 字节数据
	uint32_t indices[8] = { 0 };
	for (int i = 0; i < 8; i++) {
		if (!driver->read(indicesBase + i * 4, &indices[i], sizeof(uint32_t))) {

		}
	}
	uint8_t byteArray[256] = { 0 };
	if (!driver->read(byteArrayAddr, byteArray, sizeof(byteArray))) {

	}
	uint64_t gnamePtr = 0;
	for (int i = 0; i < 8; i++) {
		if (indices[i] >= sizeof(byteArray)) {

		}
		gnamePtr |= (uint64_t)byteArray[indices[i]] << (i * 8);
	}
	//printf("GName Pointer: 0x%llX\n", gnamePtr);
	uint64_t realGNameAddr = 0;
	if (!driver->read(gnamePtr, &realGNameAddr, sizeof(realGNameAddr))) {
	}
	//printf("Real GName Address: 0x%llX\n", realGNameAddr);
	return realGNameAddr;
}

void getString(char * buf, uintptr_t namepy)
{
    unsigned short buf16[16] = { 0 };
    driver->read(namepy, buf16, 28);
    unsigned short *pTempUTF16 = buf16;
    char *pTempUTF8 = buf;
    char *pUTF8End = pTempUTF8 + 32;
    while (pTempUTF16 < buf16 + (28 / sizeof(unsigned short))) {
    
        if (*pTempUTF16 <= 0x007F & pTempUTF8 + 1 < pUTF8End)
        {
            *pTempUTF8++ = (char) * pTempUTF16;
        }
        else if (*pTempUTF16 >= 0x0080 & *pTempUTF16 <= 0x07FF & pTempUTF8 + 2 < pUTF8End)
        {
            *pTempUTF8++ = (*pTempUTF16 >> 6) | 0xC0;
            *pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
        }
        else if (*pTempUTF16 >= 0x0800 & *pTempUTF16 <= 0xFFFF & pTempUTF8 + 3 < pUTF8End)
        {
            *pTempUTF8++ = (*pTempUTF16 >> 12) | 0xE0;
            *pTempUTF8++ = ((*pTempUTF16 >> 6) & 0x3F) | 0x80;
            *pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
        }
        else
        {
            break;
        }
        pTempUTF16++;
    }
}

const std::map<uint32_t, WeaponType> WeaponTypeMap = {
    {10101, WeaponType::LOW_LEVEL},  // 标配（初始基础武器，伤害低）
    {10901, WeaponType::LOW_LEVEL},  // 匕首（近战武器，攻击范围有限）
    
    // 狙击枪（SNIPER）- 高远程威胁
    {10501, WeaponType::SNIPER},     // 飞将
    {10504, WeaponType::SNIPER},     // 莽侠
    {10502, WeaponType::SNIPER},     // 冥狗
    
    // 爆发枪（BURST）- 高爆发威胁
    {10103, WeaponType::BURST},      // 狂怒
    {10602, WeaponType::BURST},      // 奥丁
    {10404, WeaponType::BURST},      // 狂徒
    
    // 普通枪（NORMAL）- 中等威胁（排除低级后，剩余常规武器）
    {10102, WeaponType::NORMAL},     // 短炮
    {10104, WeaponType::NORMAL},     // 鬼魅
    {10105, WeaponType::NORMAL},     // 正义
    {10201, WeaponType::NORMAL},     // 蜂刺
    {10202, WeaponType::NORMAL},     // 骇灵
    {10301, WeaponType::NORMAL},     // 雄鹿
    {10302, WeaponType::NORMAL},     // 判官
    {10401, WeaponType::NORMAL},     // 撩犬
    {10402, WeaponType::NORMAL},     // 戍卫
    {10403, WeaponType::NORMAL},     // 幻影
    {10601, WeaponType::NORMAL}      // 战神
};

WeaponType getWeaponType(uint32_t WeaponID) {
    auto iter = WeaponTypeMap.find(WeaponID);
    if (iter != WeaponTypeMap.end()) {
        return iter->second;
    } else {
        return WeaponType::UNKNOWN;
    }
}

bool IsMale(int roleID) {
	static const std::unordered_set<int> maleIDs = {
		3003001, // 不死鸟
		3005001, // 猎枭
		3008003, // 零
		3009003, // 铁壁
		3012001, // 烈狱
		3014001, // 夜露
	};
	return maleIDs.count(roleID) > 0;
}

float CalcDistance(D2D p1, D2D p2) {
    return sqrtf(powf(p2.x - p1.x, 2) + powf(p2.y - p1.y, 2));
}