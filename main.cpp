#include "draw.h"
#include "timer.h"
#include "include/My_Utils/utils.h"
#include "AndroidImgui.h"
#include "GraphicsManager.h"
#include <iostream>
#define version "2.0.1"
timer FPS限制;
int main(int argc, char *argv[]) {
    /*utils gameUtils;
    gameUtils.getGame();*/
    puts("by鲶鱼&&落泪，支持rt&qx驱动，公益绘制不要被骗\nGithub:https://github.com/nnyy666F/nnyy_cheat");
    
    ::graphics = GraphicsManager::getGraphicsInterface(GraphicsManager::VULKAN);
    ::screen_config(); 

    ::native_window_screen_x = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::native_window_screen_y = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::abs_ScreenX = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::abs_ScreenY = (::displayInfo.height < ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);

    ::window = android::ANativeWindowCreator::Create("test", native_window_screen_x, native_window_screen_y, permeate_record);
    graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y);
    ::init_My_drawdata();
    Touch::Init({(float)::abs_ScreenX, (float)::abs_ScreenY}, false);
    Touch::setOrientation(displayInfo.orientation);
    
    ImGui::StyleColorsDark();
    
    FPS限制.SetFps(game.imgui_fps);
    FPS限制.AotuFPS_init();
    FPS限制.setAffinity(); 
    
    if(!gameUtils.isjoin("1014691786")){
        puts("QQ群验证失败，请加入");
        return 0;
    }
    
    static bool flag = true;    
    while (flag) {
        drawBegin();
        graphics->NewFrame();
        
        Layout_tick_UI(&flag);

        FPS限制.SetFps(game.imgui_fps);
        FPS限制.AotuFPS();
        graphics->EndFrame();        
    }
    graphics->Shutdown();
    android::ANativeWindowCreator::Destroy(::window);
    return 0;
}
