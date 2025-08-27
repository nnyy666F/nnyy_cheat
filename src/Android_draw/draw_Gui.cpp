#include "draw.h"
#include <iostream>
#include "include/My_Utils/utils.h"
#include "include/My_Utils/kernel.h"

#include "My_font/zh_Font.h"
#include "My_font/fontawesome-brands.h"
#include "My_font/fontawesome-regular.h"
#include "My_font/fontawesome-solid.h"
#include "My_font/gui_icon.h"

#include "picture_ZhenAiKun_png.h"

#include "include/data/base.h"
#include "include/data/game_wwqy.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <cstring>

bool permeate_record = false;
bool permeate_record_ini = false;
struct Last_ImRect LastCoordinate = {0, 0, 0, 0};
int style_idx = 1;

static float orientation = -1;
ANativeWindow *window; 
android::ANativeWindowCreator::DisplayInfo displayInfo;
ImGuiWindow *g_window;
int abs_ScreenX, abs_ScreenY;

int native_window_screen_x, native_window_screen_y;

BaseTexData *kk_image;
std::chrono::high_resolution_clock::time_point start_time;
std::unique_ptr<AndroidImgui>  graphics;
ImFont* zh_font = NULL;
ImFont* icon_font_0 = NULL;
ImFont* icon_font_1 = NULL;
ImFont* icon_font_2 = NULL;
Vector2 WindowPos[64];
Vector2 WindowSize[64];
int WindowCount;
utils gameUtils;
game_wwqy game;
std::atomic<bool> is_thread_running(false);
std::atomic<int> read_data_status(0);
pthread_mutex_t game_data_mutex;
std::atomic<bool> stop_update_thread(false);
std::string config;

void LoadConfig() {
    static bool has_loaded = false;
    if (has_loaded || config == "已加载") return;

    FILE* file = fopen("config.txt", "r");
    if (!file) {
        std::cerr << "配置文件不存在，使用默认值" << std::endl;
        config = "已加载";
        has_loaded = true;
        return;
    }

    char buffer[256] = {0};
    if (fgets(buffer, sizeof(buffer), file)) {
        char* token = strtok(buffer, ",");
        if (token) game.FriX = atof(token);
        
        token = strtok(nullptr, ",");
        if (token) game.FriY = atof(token);
        
        token = strtok(nullptr, ",");
        if (token) game.FriSize = atof(token);
        
        token = strtok(nullptr, ",");
        if (token) game.face = atoi(token);
        
        token = strtok(nullptr, ",");
        if (token) game.aimbot_status = atoi(token);
        
        token = strtok(nullptr, ",");
        if (token) game.aimbot_range = atof(token);
        
        token = strtok(nullptr, ",");
        if (token) game.aimbot_press_range = atof(token);
        
        token = strtok(nullptr, ",");
        if (token) game.isShow_aimbot_range = atoi(token);
    }

    fclose(file);
    config = "已加载";
    has_loaded = true;
}

bool SaveConfig() {
    FILE* file = fopen("config.txt", "w");
    if (!file) {
        std::cerr << "Failed to open config.txt for writing!" << std::endl;
        return false;
    }
        fprintf(file, "%.2f,%.2f,%.2f,%d,%d,%f,%f,%d", 
            game.FriX, 
            game.FriY, 
            game.FriSize, 
            game.face,
            game.aimbot_status,
            game.aimbot_range,
            game.aimbot_press_range,
            game.isShow_aimbot_range);

    fclose(file);
    return true;
}

bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO &io = ImGui::GetIO();
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 3.0;
    icons_config.OversampleV = 3.0;		
    icons_config.SizePixels = SizePixels;
	::icon_font_0 = io.Fonts->AddFontFromMemoryCompressedTTF((const void *)&font_awesome_brands_compressed_data, sizeof(font_awesome_brands_compressed_data), 0.0f, &icons_config, icons_ranges);
	::icon_font_1 = io.Fonts->AddFontFromMemoryCompressedTTF((const void *)&font_awesome_regular_compressed_data, sizeof(font_awesome_regular_compressed_data), 0.0f, &icons_config, icons_ranges);
	::icon_font_2 = io.Fonts->AddFontFromMemoryCompressedTTF((const void *)&font_awesome_solid_compressed_data, sizeof(font_awesome_solid_compressed_data), 0.0f, &icons_config, icons_ranges);

    io.Fonts->AddFontDefault();
    return zh_font != nullptr;
}

void init_My_drawdata() {
    ImGui::My_Android_LoadSystemFont(25.0f);
    M_Android_LoadFont(25.0f);
   // ::kk_image = graphics->LoadTextureFromMemory((void *)picture_ZhenAiKun_PNG_H, sizeof(picture_ZhenAiKun_PNG_H));
}

void screen_config() {
    ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin() {
    if (::permeate_record_ini) {
        LastCoordinate.Pos_x = ::g_window->Pos.x;
        LastCoordinate.Pos_y = ::g_window->Pos.y;
        LastCoordinate.Size_x = ::g_window->Size.x;
        LastCoordinate.Size_y = ::g_window->Size.y;

        graphics->Shutdown();
        android::ANativeWindowCreator::Destroy(::window);
        ::window = android::ANativeWindowCreator::Create("test", native_window_screen_x, native_window_screen_y, permeate_record);
        graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y);
        ::init_My_drawdata();
        
        switch (style_idx) {
                case 0: ImGui::StyleColorsLight(); break;
                case 1: ImGui::StyleColorsDark(); break;
                case 2: ImGui::StyleColorsClassic(); break;
            }
    } 

    screen_config();
    if (::orientation != displayInfo.orientation) {
        ::orientation = displayInfo.orientation;
        Touch::setOrientation(displayInfo.orientation);
        if (g_window != NULL) {
            g_window->Pos.x = 100;
            g_window->Pos.y = 125;        
        }        
    }
}

void* read_data_thread(void* arg) {
    read_data_status.store(1, std::memory_order_release);

    pthread_mutex_lock(&game_data_mutex);
    read_data();
    bool init_ok = (game.addr != 0 && game.Gworld != 0);
    if (!init_ok) {
        read_data_status.store(-1, std::memory_order_release);
        pthread_mutex_unlock(&game_data_mutex);
        is_thread_running.store(false, std::memory_order_release);
        puts("基址获取失败了.......");
        return nullptr;
    }
    read_data_status.store(2, std::memory_order_release);
    pthread_mutex_unlock(&game_data_mutex);
    is_thread_running.store(false, std::memory_order_release);
    
    while (!stop_update_thread.load(std::memory_order_acquire)) {
        pthread_mutex_lock(&game_data_mutex);
        game.update();
        pthread_mutex_unlock(&game_data_mutex);        
        usleep((100 * 1000) / 6); // 60帧
    }
    printf("更新线程已停止\n");
    return nullptr;
}

void* start_touch(void* arg) {
    game.Touch();
    return nullptr;
}

void get_safe_game_data(int& out_enemy_cnt,float out_rxx[100],float out_ryy[100],float out_rww[100],bool out_isMiss[100],float out_hp[100],float out_armor[100],bool out_isInit[100],name out_name[100],float out_dis[100],int out_teamID[100],WeaponType out_weaType[100],BonePos out_bone[100]) {
    pthread_mutex_lock(&game_data_mutex);
    out_enemy_cnt = game.eneity_count;
    memcpy(out_rxx, game.r_xx, sizeof(game.r_xx));
    memcpy(out_ryy, game.r_yy, sizeof(game.r_yy));
    memcpy(out_rww, game.r_ww, sizeof(game.r_ww));
    memcpy(out_isMiss, game.isMiss, sizeof(game.isMiss));
    memcpy(out_hp, game.hp, sizeof(game.hp));
    memcpy(out_armor, game.armor, sizeof(game.armor));
    memcpy(out_isInit, game.isInit, sizeof(game.isInit));
    memcpy(out_name, game.text, sizeof(game.text));
    memcpy(out_dis, game.dis, sizeof(game.dis));
    memcpy(out_teamID, game.teamID, sizeof(game.teamID));
    memcpy(out_weaType, game.priority, sizeof(game.priority));
    memcpy(out_bone, game.Bone_Pos, sizeof(game.Bone_Pos));
    pthread_mutex_unlock(&game_data_mutex);
}

void Layout_tick_UI(bool *main_thread_flag) {
    WindowCount = 0;
    static bool show_draw_Line = false;
    static bool show_another_window = false;

    static const int kPackageCount = sizeof(gameUtils.packageNames) / sizeof(gameUtils.packageNames[0]);
    static int pidArray[kPackageCount] = {-1}; // 存储每个包名的PID
    static int validCount = 0;                 // 有效PID数量（≠0）
    static bool hasDetected = false;           // 是否已执行过检测
    static bool initSuccess = false;           // 是否已成功初始化river
    static int selectedPID = -1;               // 用户选择的PID    

    // 初始化互斥锁（仅第一次调用时执行）
    static bool mutex_inited = false;
    if (!mutex_inited) {
        pthread_mutex_init(&game_data_mutex, nullptr);
        mutex_inited = true;
    }
    
      LoadConfig(); 
    { 
        static float f = 0.0f;
        static int counter = 0;
        static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        ImGui::Begin("鲶鱼辅助");        
        ImGui::Text("渲染接口 : %s, gui版本 : %s", graphics->RenderName, IMGUI_VERSION);
        if (ImGui::Combo("##主题", &style_idx, "白色主题\0蓝色主题\0紫色主题\0")) {
            switch (style_idx) {
                case 0: ImGui::StyleColorsLight(); break;
                case 1: ImGui::StyleColorsDark(); break;
                case 2: ImGui::StyleColorsClassic(); break;
            }
        }
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "应用平均 %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("退出辅助")) {
            SaveConfig();
            *main_thread_flag = false;
        }

        ImGui::Spacing();
        ImGui::Text("【屏幕朝向控制】");
        ImGui::RadioButton("右侧朝向", &game.face, 0);        
        ImGui::SameLine();
        ImGui::RadioButton("左侧朝向", &game.face, 1);
        
        ImGui::Spacing();
        ImGui::Text("【自瞄调节】");
        ImGui::RadioButton("开火", &game.aimbot_status, 0);
        ImGui::SameLine();
        ImGui::RadioButton("开镜", &game.aimbot_status, 1);
        ImGui::SameLine();
        ImGui::RadioButton("组合", &game.aimbot_status, 2);
        //ImGui::SliderFloat("自瞄速度",&game.touch_s,0,25);
        
        ImGui::Separator();
        ImGui::Text("【游戏进程管理】");
        ImGui::Spacing();

        // 检测按钮：点击触发PID扫描（仅未检测/未初始化时显示）
        if (!hasDetected || !initSuccess) {
            if (ImGui::Button("开始检测游戏进程")) {
            ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
                int screenWidth = displayInfo.width;
                int screenHeight = displayInfo.height;
                
                validCount = 0;
                selectedPID = -1;
                initSuccess = false;
                read_data_status.store(0, std::memory_order_release); // 重置数据初始化状态

                for (int i = 0; i < kPackageCount; i++) {
                    pidArray[i] = getPID(gameUtils.packageNames[i]); 
                    if (pidArray[i] > 0) {
                        validCount++;
                    }
                }
                hasDetected = true; // 标记为已检测
                game.setPxPy(screenWidth/2,screenHeight/2);
            }
            ImGui::Spacing();
        }

        if (hasDetected) {
            if (validCount == 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "❌ 未检测到任何运行的游戏");
                if (ImGui::Button("重新检测")) {
                    hasDetected = false; // 重置检测状态，允许重新检测
                }
            }
            // 情况2：仅1个有效进程（自动选择）
            else if (validCount == 1) {
                // 找到唯一有效PID
                for (int i = 0; i < kPackageCount; i++) {
                    if (pidArray[i] > 0) {
                        selectedPID = pidArray[i];
                        break;
                    }
                }
                // 显示结果 + 初始化按钮
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "✅ 检测到1个游戏进程：");
                int pkg_idx = -1;
                for (int i = 0; i < kPackageCount; i++) {
                    if (pidArray[i] == selectedPID) {
                        pkg_idx = i;
                        break;
                    }
                }
                if (pkg_idx != -1) {
                    ImGui::Text("包名：%s", gameUtils.packageNames[pkg_idx]);
                } else {
                    ImGui::Text("包名：未知");
                }
                ImGui::Text("PID：%d", selectedPID);
                ImGui::Spacing();

                if (!initSuccess && ImGui::Button("初始化进程连接")) {
                    river_init(selectedPID); // 调用初始化函数
                    initSuccess = true;
                }
                if(initSuccess) {
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f), "✅ 进程连接成功！");
                }
            }
            else {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "✅ 检测到%d个游戏进程，请选择：", validCount);
                ImGui::Spacing();
                int tempCount = 0;
                for (int i = 0; i < kPackageCount; i++) {
                    if (pidArray[i] > 0) {
                        tempCount++;
                        char btnText[128];
                        snprintf(btnText, sizeof(btnText), "%d. %s（PID：%d）", tempCount, gameUtils.packageNames[i], pidArray[i]);
                        if (ImGui::Button(btnText)) {
                            selectedPID = pidArray[i];
                        }
                        ImGui::Spacing();
                    }
                }
                if (selectedPID != -1) {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "已选择：PID = %d", selectedPID);
                    if (!initSuccess && ImGui::Button("确认初始化进程连接")) {
                        river_init(selectedPID); 
                        initSuccess = true;   
                        base::cott();                 
                    }
                    
                    if(initSuccess) {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f), "✅ 进程连接成功！");           			
                    }
                }
            }
            
            if (initSuccess) {
                ImGui::Text("开火键区域设置");		
            			ImGui::SliderFloat("区域大小",&game.FriSize,0,100);				
            			ImGui::SliderFloat("区域X",&game.FriX,0,displayInfo.width);
            			ImGui::SliderFloat("区域Y",&game.FriY,0,displayInfo.height);		
            			
            			auto FriSize=1.3+(game.FriSize*1.3-50)*0.01;                   	    
            			auto fwx=displayInfo.width-125*FriSize-game.FriX;
            			auto fwy=125*FriSize+game.FriY;
            			game.ctrl.fwx = fwx;
            			game.ctrl.fwy = fwy;
            			game.ctrl.fwdx = 125*FriSize;
            			            			           			
                        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
                        draw_list->AddCircleFilled(
                            ImVec2(fwx, fwy),  
                            game.ctrl.fwdx,
                            IM_COL32(255, 255, 255, 64)
                        );
                        
                        ImVec2 text_pos = ImVec2(
                            fwx - 35 / 2.0f,
                            fwy - 35 / 2.0f
                        );
                    
                        draw_list->AddText(
                            zh_font,
                            25.0f,
                            text_pos,
                            IM_COL32(
                            0,0,0,255
                            ),
                            "\t\t开火键区域\n(请手动调节至开火键上！)"                                                        
                        );
                        
                ImGui::SliderFloat("自瞄范围", &game.aimbot_range, 50.0f, game.py);       
                ImGui::SliderFloat("压枪力度", &game.aimbot_press_range, 0.0f, 50.0f);   
                ImGui::Checkbox("显示范围",&game.isShow_aimbot_range);
                if(game.isShow_aimbot_range){
                ImGui::GetForegroundDrawList()->AddCircle( ImVec2(game.px, game.py), game.aimbot_range,ImColor(0,255,0,150),0,1.6f);
                }
 
                        
                int current_status = read_data_status.load(std::memory_order_acquire);
                bool thread_running = is_thread_running.load(std::memory_order_acquire);

                // 仅未开始/失败且线程未运行时可点击
                if ((current_status == 0 || current_status == -1) && !thread_running) {
                    if (ImGui::Button("初始化游戏数据（启动线程）")) {
                        pthread_t tid;
                        int ret = pthread_create(&tid, nullptr, read_data_thread, nullptr);
                        if (ret == 0) {
                            pthread_detach(tid);
                            is_thread_running.store(true, std::memory_order_release);
                        } else {
                            read_data_status.store(-1, std::memory_order_release);
                            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "❌ 线程创建失败！错误码：%d", ret);
                        }
                        pthread_t tid2;
                        int ret2 = pthread_create(&tid, nullptr, start_touch, nullptr);
                        if (ret2 == 0) {
                            pthread_detach(tid2);
                        }
                    }
                }                
                else if (current_status == 1 && thread_running) {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "⏳ 数据初始化中...（请勿重复点击）");
                }
                else if (current_status == 2) {
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "✅ 游戏数据初始化完成！");
                    // 线程安全读取数据
                    int show_enemy_cnt;
                    float show_matrix[16];
                    float show_rxx[100]={};
                    float show_ryy[100]={};
                    float show_rww[100]={};
                    bool show_isMiss[100]={};
                    float show_hp[100]={};
                    float show_armor[100]={};
                    bool show_isInit[100]={};
                    name show_name[100]={};
                    float show_dis[100]={};
                    int show_teamID[100]={};
                    WeaponType show_weaType[100]={};
                    BonePos show_Bone_Pos[100];
                    get_safe_game_data(show_enemy_cnt,show_rxx,show_ryy,show_rww,show_isMiss,show_hp,show_armor,show_isInit,show_name,show_dis,show_teamID,show_weaType,show_Bone_Pos);

                                    
                   for(int i=0;i<show_enemy_cnt;i++) {
                        if(!show_isInit[i]) continue;
                        game.view = {show_rxx[i] - (show_ryy[i] - show_rww[i])/24, show_ryy[i], (show_ryy[i] - show_rww[i])/2, (show_ryy[i] - show_rww[i])};
                        auto x_start=game.view.x+game.view.w/2;
                        auto x_start2=game.view.x+game.view.h/4;
                        auto x_start3=game.view.y+game.view.h/3;
                        auto x_start4=game.view.x+game.view.w;
                        auto y_start2=game.view.x-game.view.h/4;
                        auto y_start=game.view.y-game.view.h+game.view.h/2;
                                                                                       
                        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
                        if ((show_ryy[i] - show_rww[i])/2<0) {
                        if(show_dis[i]<=25 && !show_isMiss[i]){
                            draw_list->AddLine(ImVec2(game.px,100), ImVec2(-game.view.x+displayInfo.width, game.view.y-game.view.w+displayInfo.height), IM_COL32(255,0,0, 255), 2);
                        }
                         continue;                        
                       }
                       
                        ImVec2 rect_min = ImVec2(x_start2, x_start3);
                        ImVec2 rect_max = ImVec2(y_start2, y_start);
                        
                        auto color = IM_COL32(0, 255, 0, 255);
                        switch(show_weaType[i]){
                            case WeaponType::LOW_LEVEL:
                                color = IM_COL32(200, 200, 200, 255);
                                break;
                            case WeaponType::BURST:
                                color = IM_COL32(255, 50, 50, 255);
                                break;
                            case WeaponType::SNIPER:
                                color = IM_COL32(0, 255, 255, 255);
                                break;                                
                            case WeaponType::UNKNOWN:
                                color = IM_COL32(100, 100, 100, 255);
                                break;
                        }
                        
                        draw_list->AddRect(
                            rect_min,
                            rect_max,
                            show_isMiss[i]?IM_COL32(255, 255, 0, 255):color,
                            0.0f,
                            ImDrawFlags_RoundCornersAll,
                            1.0f
                        );                       
                       if(!show_isMiss[i]){
                        draw_list->AddLine(ImVec2(game.px, 100),ImVec2(game.view.x, y_start - 40),IM_COL32(255,255,255, 200),2);
                       }                      
                        if (show_hp[i]<100.0f) {
                        auto rect_center =4 * (y_start2-(x_start2))/2;
                        
                        float hpjd=game.view.y-(show_hp[i]<10?10:show_hp[i])/100*game.view.w*2+game.view.h/2;
                        draw_list->AddLine(ImVec2(x_start4+4+rect_center/6, hpjd),ImVec2(x_start4+4+rect_center/6, game.view.y-(100<10?10:100)/100*game.view.w*2+game.view.h/2),IM_COL32(255,255,255, 255),2);
                        draw_list->AddLine(ImVec2(x_start4+4+rect_center/6, x_start3),ImVec2(x_start4+4+rect_center/6, hpjd),IM_COL32(220,20,61, 200),2);
                        }
                        
                        draw_list->AddText(zh_font,30.0f,ImVec2(game.view.x - 40, y_start - 45),IM_COL32(255, 255, 0, 255),show_name[i].text);
                        
                        std::string t = "[" + std::to_string(int(show_dis[i])) + " M]";
                        draw_list->AddText(zh_font,20.0f,ImVec2(game.view.x+20, y_start - 20),IM_COL32(255, 255, 255, 255),t.c_str());
                        
                        t = "[" + std::to_string(show_teamID[i]) + " ]";
                        draw_list->AddText(zh_font,20.0f,ImVec2(game.view.x-35, y_start - 18),IM_COL32(0, 255, 0, 255),t.c_str());             
                        
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[0].x, show_Bone_Pos[i].Bone_Pos[0].y), ImVec2(show_Bone_Pos[i].Bone_Pos[1].x, show_Bone_Pos[i].Bone_Pos[1].y), IM_COL32(255,0,0, 255), 1.0f); // 头 → 胸
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[1].x, show_Bone_Pos[i].Bone_Pos[1].y), ImVec2(show_Bone_Pos[i].Bone_Pos[2].x, show_Bone_Pos[i].Bone_Pos[2].y), IM_COL32(255,0,0, 255), 1.0f); // 胸 → 骨盆
                        
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[1].x, show_Bone_Pos[i].Bone_Pos[1].y), ImVec2(show_Bone_Pos[i].Bone_Pos[3].x, show_Bone_Pos[i].Bone_Pos[3].y), IM_COL32(255,0,0, 255), 1.0f); // 胸 → 左肩
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[3].x, show_Bone_Pos[i].Bone_Pos[3].y), ImVec2(show_Bone_Pos[i].Bone_Pos[4].x, show_Bone_Pos[i].Bone_Pos[4].y), IM_COL32(255,0,0, 255), 1.0f); // 左肩 → 左肘
                        
                        // jni/src/Android_draw/draw_Gui.cpp
// 左臂绘制逻辑（修复类型转换）
                        D2D leftElbow = {show_Bone_Pos[i].Bone_Pos[4].x, show_Bone_Pos[i].Bone_Pos[4].y};  // 自定义D2D类型
                        D2D originalLeftHand = {show_Bone_Pos[i].Bone_Pos[5].x, show_Bone_Pos[i].Bone_Pos[5].y};  // 自定义D2D类型
                        float leftArmLength = CalcDistance(leftElbow, originalLeftHand);  // CalcDistance已支持D2D，无需改
                        
                        // 修复：D2D 转 ImVec2（手动用x/y构造ImVec2，不能直接赋值）
                        ImVec2 leftHandTarget = ImVec2(originalLeftHand.x, originalLeftHand.y);  // 原错误行：需手动传x/y
                        
                        if (leftArmLength > 10.0f) {
                            // 方向向量计算（D2D类型，不影响）
                            ImVec2 dir;
                            dir.x = originalLeftHand.x - leftElbow.x;
                            dir.y = originalLeftHand.y - leftElbow.y;
                        
                            // 归一化（ImVec2分量计算，正常）
                            dir.x = dir.x / leftArmLength;
                            dir.y = dir.y / leftArmLength;
                        
                            // 修复：D2D（leftElbow）转 ImVec2，再计算目标位置
                            leftHandTarget.x = leftElbow.x + dir.x * 10.0f;  // leftElbow.x 是D2D的x，直接用
                            leftHandTarget.y = leftElbow.y + dir.y * 10.0f;  // leftElbow.y 是D2D的y，直接用
                        }
                        
                        // 修复：D2D（leftElbow）转 ImVec2，传给AddLine（AddLine需要ImVec2参数）
                        draw_list->AddLine(ImVec2(leftElbow.x, leftElbow.y), leftHandTarget, IM_COL32(255,0,0, 255), 1.0f);
                        
                        
                        // 右臂绘制逻辑（同样修复类型转换，关键处已标注）
                        D2D rightElbow = {show_Bone_Pos[i].Bone_Pos[7].x, show_Bone_Pos[i].Bone_Pos[7].y};
                        D2D originalRightHand = {show_Bone_Pos[i].Bone_Pos[8].x, show_Bone_Pos[i].Bone_Pos[8].y};
                        float rightArmLength = CalcDistance(rightElbow, originalRightHand);
                        
                        // 修复：D2D 转 ImVec2
                        ImVec2 rightHandTarget = ImVec2(originalRightHand.x, originalRightHand.y);
                        
                        if (rightArmLength > 10.0f) {
                            ImVec2 dir;
                            dir.x = originalRightHand.x - rightElbow.x;
                            dir.y = originalRightHand.y - rightElbow.y;
                        
                            dir.x = dir.x / rightArmLength;
                            dir.y = dir.y / rightArmLength;
                        
                            // 修复：用D2D的x/y计算ImVec2目标位置
                            rightHandTarget.x = rightElbow.x + dir.x * 10.0f;
                            rightHandTarget.y = rightElbow.y + dir.y * 10.0f;
                        }
                        
                        // 修复：D2D 转 ImVec2，传给AddLine
                        draw_list->AddLine(ImVec2(rightElbow.x, rightElbow.y), rightHandTarget, IM_COL32(255,0,0, 255), 1.0f);
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[2].x, show_Bone_Pos[i].Bone_Pos[2].y), ImVec2(show_Bone_Pos[i].Bone_Pos[9].x, show_Bone_Pos[i].Bone_Pos[9].y), IM_COL32(255,0,0, 255), 1.0f);  // 骨盆 → 左臀
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[9].x, show_Bone_Pos[i].Bone_Pos[9].y), ImVec2(show_Bone_Pos[i].Bone_Pos[10].x, show_Bone_Pos[i].Bone_Pos[10].y), IM_COL32(255,0,0, 255), 1.0f); // 左臀 → 左膝
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[10].x, show_Bone_Pos[i].Bone_Pos[10].y), ImVec2(show_Bone_Pos[i].Bone_Pos[11].x, show_Bone_Pos[i].Bone_Pos[11].y), IM_COL32(255,0,0, 255), 1.0f); // 左膝 → 左脚

                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[2].x, show_Bone_Pos[i].Bone_Pos[2].y), ImVec2(show_Bone_Pos[i].Bone_Pos[12].x, show_Bone_Pos[i].Bone_Pos[12].y), IM_COL32(255,0,0, 255), 1.0f);  // 骨盆 → 右臀
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[12].x, show_Bone_Pos[i].Bone_Pos[12].y), ImVec2(show_Bone_Pos[i].Bone_Pos[13].x, show_Bone_Pos[i].Bone_Pos[13].y), IM_COL32(255,0,0, 255), 1.0f); // 右臀 → 右膝
                        draw_list->AddLine(ImVec2(show_Bone_Pos[i].Bone_Pos[13].x, show_Bone_Pos[i].Bone_Pos[13].y), ImVec2(show_Bone_Pos[i].Bone_Pos[14].x, show_Bone_Pos[i].Bone_Pos[14].y), IM_COL32(255,0,0, 255), 1.0f); // 右膝 → 右脚
                        
                        draw_list->AddCircle(ImVec2(show_Bone_Pos[i].Bone_Pos[0].x, show_Bone_Pos[i].Bone_Pos[0].y), 7.0f, ImColor(255,0,0, 255), 0, 4.0f);
                   }         
                }
                // 初始化失败
                else if (current_status == -1) {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "❌ 数据初始化失败！请检查驱动或进程状态");
                    if (!thread_running && ImGui::Button("重新初始化数据")) {
                        read_data_status.store(0, std::memory_order_release); // 重置状态允许重试
                    }
                }
            }
        }
        
        WindowPos[WindowCount] = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);       
        WindowSize[WindowCount] = Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);       
        WindowCount ++;        
        ImGui::End(); 

    }

    Touch::SetTouchObstacle(WindowPos, WindowSize, WindowCount);
}

void read_data() {
    game.load_init();
}