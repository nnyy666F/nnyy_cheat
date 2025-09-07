#pragma once

#include "base.h"
#include <cstdint>
#include "../My_Utils/kernel.h"
#include <pthread.h>
#include "struct.h"  

//uint64_t
class game_wwqy : public base {
public:
    void getModuleBase() override;
    void load_init();
    int eneity_count;
    int health[100];
    uint64_t Gname;
    uint64_t LocalPlayer_Offset;
    
    uint64_t ViewMatrix;
    uint64_t Gworld;
    uint64_t EntityArr;
    uint64_t object;
    uint64_t PlayerController;
    uint64_t PlayerCameraManager;
    
    uint32_t bone_count = 15;
    
    int teamID[100]={};
    
    D3D pos[100]={};
    D2D point[100]={};
    float dis[100]={};
    float fov=0;
    float r_xx[100]={};
    float r_yy[100]={};
    float r_ww[100]={};
    
    float hp[100]={};
    float armor[100]={};
    
    float matrix[16];    
    MinimalViewInfo viewInfo;
    
    void update();
    void Touch();
    void setPxPy(int px,int py) override;
    Vec3 view;
    
    bool isMiss[100]={};
    bool isInit[100]={};
    bool isOverlap[100]={};
    
    WeaponType priority[100]={};
    
    name text[100];       
        
    BonePos Bone_Pos[100];
    boneList bone_list;
    
    float FriSize;
    float FriX;
    float FriY;
    
    TouchC ctrl;
    int face=-1;
    int touch_status=0;
    int aimbot_status=0;
    float aimbot_range=0.0f;
    float aimbot_press_range=0.0f;
    bool isShow_aimbot_range=false;  
    float aimbot_pull_time=0.0f;
    float touch_s=16.0f;
    
    bool ishasP=false;   
    float rect_w[100] = {0.0f};
    float rectWidth=-1;
    
    int imgui_fps=120;
};