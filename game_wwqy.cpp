#include "include/data/game_wwqy.h"
#include <iostream>
#include "include/Android_touch/TouchHelperA.h"
#include <algorithm> 

void game_wwqy::getModuleBase() {
    this->addr = ::GetModuleBaseAddr("libUE4.so");
}

D3D pos_head[500];
void game_wwqy::load_init() {
    getModuleBase();
    
    LocalPlayer_Offset = ReadValue(this->addr+0xA93C4D0);    
    printf("addr%p\n",this->addr);
    ViewMatrix = ReadValue(ReadValue(ReadValue(this->addr + 0xA958ba8) + 0x10) + 0x8) + 0x690;    
    printf("矩阵%p\n",ViewMatrix);   
    Gworld = ReadValue(ReadValue(LocalPlayer_Offset + 0x70) + 0x70);
}

uint64_t tempP=0;
void game_wwqy::update() {
    Gworld = ReadValue(ReadValue(LocalPlayer_Offset + 0x70) + 0x70);
    if (Gworld == 0) {
        std::cout<<"error 33";
        return;
    }
    Gname = ReadValue(ReadValue(this->addr + 0xA62fee8)+8)+0x690;
    EntityArr = ReadValue(Gworld + 0x198) + 0x2c8;
    eneity_count = ReadDword(EntityArr + 8);
    if(eneity_count==1){
    EntityArr = ReadValue(Gworld + 0x30) + 0x98;
    eneity_count = ReadDword(EntityArr + 8);    
    EntityArr = ReadValue(EntityArr);
    tempP=EntityArr;
    }else{
    EntityArr = ReadValue(EntityArr);
    tempP=0;
    }
    driver->read(this->ViewMatrix, this->matrix, 16 * sizeof(float)); 
    PlayerController = ReadValue(LocalPlayer_Offset+0x30);
    PlayerCameraManager = ReadValue(PlayerController + 0x348) + 0xF50;
    driver->read(PlayerCameraManager,&viewInfo.Location,sizeof(viewInfo.Location));
    driver->read(PlayerCameraManager + 0x18,&viewInfo.Rotation,sizeof(viewInfo.Rotation));
    driver->read(PlayerCameraManager + 0x24,&viewInfo.FOV,sizeof(viewInfo.FOV)) ;
    
    uint64_t me = ReadValue(PlayerController + 0x2e0);
    uint32_t me_Teamid = ReadDword(ReadValue(me+0x2d0)+0x480);
    
    D3D me_pos = {};    
    
    for (int i=0;i<eneity_count;i++) {
        isInit[i] = false;
        driver->read(ReadValue(me + 0x320) + 0x1f0, &me_pos, sizeof(me_pos));
        object = ReadValue(EntityArr + 8*i);        
        if (object <= 0x10000000 || object % 4 != 0 || object >= 0x10000000000)
                continue;
                
        if(object==me) continue;     
        
        teamID[i] = ReadDword(ReadValue(object+0x2d0)+0x480);                             
        if(tempP==0){

        object = ReadValue(object+0x310);
        teamID[i] = ReadDword(ReadValue(object+0x2d0)+0x480);
        if (teamID[i] == me_Teamid) continue;
        }else {
        if(teamID[i]!=0) continue;
        }

        uint64_t HealthManger = ReadValue(ReadValue(ReadValue(object+0x6b8)+0x158));
        hp[i] = ReadFloat(HealthManger+0xd0);             
        if(hp[i]<=0 || ReadDword(ReadValue(object+0x2d0)+0x4b0)==1) continue;   
        armor[i] = ReadFloat(HealthManger + 0x1c8);
        
        getString(text[i].text,ReadValue(ReadValue(object+0x2d0)+0x388));    
        priority[i] = getWeaponType(ReadDword(object+0xec0));
        if (ReadFloat(ReadValue(object + 0x198) + 0x134)!=0&&ReadFloat(ReadValue(object + 0x198) + 0x134+8)!=-90000){
            driver->read(ReadValue(object + 0x198) + 0x134, &pos[i], sizeof(this->pos[i]));           
            isMiss[i] = false;
        }else {
            isMiss[i] = true;
        }             
        dis[i] = getDis(pos[i],me_pos) *0.01f;        
        if(dis[i] == 0) continue;
        if(dis[i] < 25&&isMiss[i]){
            continue;  
        }
        uint64_t mesh = ReadValue(object + 0x310);
        uint64_t human = mesh + 0x1e0;
        uint64_t bone = ReadValue(mesh + 0x570);
        FTransform meshtrans;
        driver->read(human, &meshtrans, 4*11);
        FMatrix c2wMatrix = TransformToMatrix(meshtrans);
        D2D TempBone_Pos[15] = {};
                  
        double cameraz = matrix[3] * this->pos[i].x+matrix[7] * this->pos[i].y+matrix[11] * this->pos[i].z + matrix[15];
        r_xx[i] = px+(matrix[0] * this->pos[i].x+matrix[4] * this->pos[i].y+matrix[8] * this->pos[i].z + matrix[12]) / cameraz * px;
        r_yy[i] = py - (matrix[1] * this->pos[i].x+matrix[5] * this->pos[i].y + matrix[9] * (this->pos[i].z - 5)+matrix[13]) / cameraz * py;
        r_ww[i] = py - (matrix[1] * this->pos[i].x+matrix[5] * this->pos[i].y + matrix[9] * (this->pos[i].z+225)+matrix[13]) / cameraz * py;
        bone_count = 15;                          
        if((r_yy[i] - r_ww[i])/2<0) {
            bone_count=0;
        }          
        Bone_Pos[i].bone_count = bone_count;
        uint32_t HeroID = ReadDword(ReadValue(object+0x2d0)+0x494); 
        for (int k = 0; k < bone_count; k++) {
            const uint32_t* boneTemp = IsMale(HeroID) ? bone_list.boneTemp_Male : bone_list.boneTemp_Female;             
            FTransform temp_trans;
            driver->read(bone + (boneTemp[k] * 48), &temp_trans, 4 * 11);
            FMatrix boneMatrix = TransformToMatrix(temp_trans);
            D3D Pos = MarixToVector(MatrixMulti(boneMatrix,c2wMatrix));
            if (k == 0){
                pos_head[i] = Pos;
                Pos.z += 7;
            }
            D2D ScreenPos = WorldToScreen(Pos, viewInfo, px*2, py*2);
            TempBone_Pos[k].x = ScreenPos.x;
            TempBone_Pos[k].y = ScreenPos.y;
        }                     
        for (int j = 0; j < bone_count; j++) {
            Bone_Pos[i].Bone_Pos[j] = TempBone_Pos[j];
        }  
        isInit[i] = true;                  
    }
}

bool IsDown = false;
int tempIndex=-1;
float lastTouchX = -1.0f;
float lastTouchY = -1.0f;
const float SWIPE_THRESHOLD = 5.0f;
bool isSwipe=false;
void game_wwqy::Touch()
{
    const int 屏幕高 = this->py * 2;
    const int 屏幕宽 = this->px * 2;
    D2D s_touchPos = { static_cast<float>(1.2f * px), static_cast<float>(py) }; 
    static bool isInTargetArea = false;
    static std::chrono::high_resolution_clock::time_point pressStartTime;
    static std::chrono::high_resolution_clock::time_point startTime;

    while(true){
        touch_status = 0;
        static D3D last_world[500];
        static bool has_last_world[500];
        
        auto touches = Touch::GetAllFingerPositions();
        bool currentInTarget = false;        
        isSwipe=false;

        for (const auto& touch : touches) {

            bool triggerTarget = false;
            if (1||this->face == 0) {
                if (touch.x > (ctrl.fwx - ctrl.fwdx) && touch.x < (ctrl.fwx + ctrl.fwdx) && 
                    touch.y > (ctrl.fwy - ctrl.fwdx) && touch.y < (ctrl.fwy + ctrl.fwdx)) {
                    triggerTarget = true;
                }
            } else {
                if (touch.x > (ctrl.fwy - ctrl.fwdx) && touch.x < (ctrl.fwy + ctrl.fwdx) && 
                    (屏幕宽 - touch.y) > (ctrl.fwx - ctrl.fwdx) && (屏幕宽 - touch.y) < (ctrl.fwx + ctrl.fwx)) {
                    triggerTarget = true;
                }
            }            
            
            if(touch.x>px&&touch.y>py){
                float deltaX = 0.0f;
                float deltaY = 0.0f;
                if (lastTouchX != -1.0f && lastTouchY != -1.0f) {        
                    deltaX = touch.x - lastTouchX;
                    deltaY = touch.y - lastTouchY;
                    if (fabs(deltaX) > SWIPE_THRESHOLD || fabs(deltaY) > SWIPE_THRESHOLD) {
                        isSwipe = true;       

                    }         
                }
                lastTouchX = touch.x;
                lastTouchY = touch.y;
            }

        
            if (triggerTarget) {
                currentInTarget = true;
                if (!isInTargetArea) {
                    pressStartTime = std::chrono::high_resolution_clock::now();
                    isInTargetArea = true;
                }
                auto now = std::chrono::high_resolution_clock::now();
                float duration = std::chrono::duration_cast<std::chrono::duration<float>>(now - pressStartTime).count();
                if (duration < 0.1f) {
                    continue;
                }
                touch_status++;
                if (1||this->face == 0) {
                    if (touch.x > py && touch.y > px) {
                        touch_status++;     
                    }            
                } else {        
                    if (touch.x < py && touch.y < px) {
                        touch_status++;     
                    }            
                }
            }
        }
        
        if (!currentInTarget) {
            isInTargetArea = false;
        }
        float min_distance = FLT_MAX;
        D2D obj_headPos = {0.f, 0.f};
        bool has_target = false;
        for (int i = 0; i < eneity_count; i++) {
            if (!isInit[i] || Bone_Pos[i].bone_count <= 0 || isMiss[i]) {
                continue;
            }
            if (touch_status != 0 && !isSwipe && tempIndex != -1 && i != tempIndex) {
                if(hp[tempIndex]>0){
                continue;
                }else{
                tempIndex=-1;
                startTime = std::chrono::high_resolution_clock::now();
                }
            }
                        
            D3D headWorld = pos_head[i];
            D3D predictWorld = headWorld;
            
            if (has_last_world[i]) {
                float vx = headWorld.x - last_world[i].x;
                float vy = headWorld.y - last_world[i].y;
                float vz = headWorld.z - last_world[i].z;
                vx *= 0.6f; vy *= 0.6f; vz *= 0.6f;
                float dt = 0.016f;
                predictWorld.x = headWorld.x + vx / dt * 0.2f;
                predictWorld.y = headWorld.y + vy / dt * 0.2f;
                predictWorld.z = headWorld.z + vz / dt * 0.2f;
            }

            last_world[i] = headWorld;
            has_last_world[i] = true;
            //printf("%f\n",rect_w[i]);

            D2D predictScreen = WorldToScreen(predictWorld, viewInfo, px*2, py*2);
            D2D worldScreen = WorldToScreen(headWorld, viewInfo, px*2, py*2);
            if (worldScreen.x < 1.0f || worldScreen.x > 屏幕宽-1.0f || 
                worldScreen.y < 1.0f || worldScreen.y > 屏幕高-1.0f) continue;

            float dx_center = worldScreen.x - px;
            float dy_center = worldScreen.y - py;
            //if (dx_center*dx_center + dy_center*dy_center > aimbot_range*aimbot_range) continue;
            if (fabs(dx_center) > aimbot_range * rect_w[i] || fabs(dy_center) > py) {
                continue;
            }
            
            float dx = worldScreen.x - px;
            float dy = worldScreen.y - py;
            if (dx*dx + dy*dy < min_distance) {
                min_distance = dx*dx + dy*dy;
                obj_headPos = worldScreen;
                auto now = std::chrono::high_resolution_clock::now();
                float duration = std::chrono::duration_cast<std::chrono::duration<float>>(now - startTime).count();
                /*float targetDist = sqrt(dx*dx + dy*dy);
                float aimCircleRadius = aimbot_range;
                bool isIn75PercentCircle = (targetDist / aimCircleRadius) <= 0.75f; 
                if (aimCircleRadius <= 0.001f) isIn75PercentCircle = true;  */
                bool isIn75PercentCircle = (rect_w[i] <= 0.0f) || (fabs(dx_center) < (aimbot_range * 0.25f) * rect_w[i]);
                if (duration > aimbot_pull_time || isIn75PercentCircle) {
                    has_target = true;
                    rectWidth=rect_w[i];
                    tempIndex = i;
                }
            }
            
            if(tempIndex!=-1&&i==tempIndex){
                i=0;
            }
        }
       
        const D2D anchorPos = { static_cast<float>(1.2f * px), static_cast<float>(py) };
        if (!IsDown) s_touchPos = anchorPos;
        if (touch_status==1&&aimbot_status==0||touch_status==2&&aimbot_status==2||touch_status==1&&aimbot_status==1) {
            if(!has_target) continue;
            
            if (!IsDown) {
                Touch::Down(s_touchPos.y, s_touchPos.x);
                IsDown = true;
            }

            float err_x = obj_headPos.x - px;
            float err_y = (py - obj_headPos.y) - aimbot_press_range;

            float step_x = err_x / touch_s;
            float step_y = err_y / touch_s;
            const float maxStep = 35.0f;
            step_x = std::clamp(step_x, -maxStep, maxStep);
            step_y = std::clamp(step_y, -maxStep, maxStep);
            const float minStep = 1.9f;
            if (std::fabs(step_x) < minStep && std::fabs(err_x) > 5.0f) step_x = (err_x>0?minStep:-minStep);
            if (std::fabs(step_y) < minStep && std::fabs(err_y) > 5.0f) step_y = (err_y>0?minStep:-minStep);

            if (s_touchPos.x >= 屏幕宽-100 || s_touchPos.x <= 100 || 
                s_touchPos.y >= 屏幕高-100 || s_touchPos.y <= 100) {
                s_touchPos = anchorPos;
                Touch::Up();
                Touch::Down(s_touchPos.y, s_touchPos.x);
            }
            if(this->face == 1) {
            s_touchPos.x = std::clamp(s_touchPos.x - step_x, 1.0f, static_cast<float>(屏幕宽-1));
            s_touchPos.y = std::clamp(s_touchPos.y - step_y, 1.0f, static_cast<float>(屏幕高-1));
            Touch::Move(s_touchPos.y, s_touchPos.x);
            }else {
            s_touchPos.x = std::clamp(s_touchPos.x + step_x, 1.0f, static_cast<float>(屏幕宽-1));
            s_touchPos.y = std::clamp(s_touchPos.y + step_y, 1.0f, static_cast<float>(屏幕高-1));
            Touch::Move(s_touchPos.y, s_touchPos.x);
            }

        } else {
            if (IsDown) {
                Touch::Up();
                IsDown = false;
                s_touchPos = anchorPos;
                tempIndex=-1;
            }
        }

        usleep(1000/60*1000);
    }
}

void game_wwqy::setPxPy(int px, int py) {
    this->px = px > py ? px : py;
    this->py = px > py ? py : px;
}