#include "draw.h"
#include "include/My_Utils/utils.h"
#include "AndroidImgui.h"
#include "GraphicsManager.h"
#include <iostream>

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
    
    Touch::Init({(float)::abs_ScreenX, (float)::abs_ScreenY}, false);
    Touch::setOrientation(displayInfo.orientation);
    ::init_My_drawdata();
    
    ImGui::StyleColorsDark();
    
    static bool flag = true;    
    while (flag) {
        drawBegin();
        if (permeate_record == false) {
            android::ANativeWindowCreator::ProcessMirrorDisplay();
        }
        
        graphics->NewFrame();
        
        Layout_tick_UI(&flag);

        graphics->EndFrame();
    }
    graphics->Shutdown();
    android::ANativeWindowCreator::Destroy(::window);
    return 0;
}
