#ifndef NATIVESURFACE_DRAW_H
#define NATIVESURFACE_DRAW_H

#include <stdio.h>
#include <stdlib.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "native_surface/ANativeWindowCreator.h"

#include "AndroidImgui.h"
#include "TouchHelperA.h"
#include "my_imgui.h"
#include "include/data/game_wwqy.h"
#include "include/My_Utils/utils.h"

extern ANativeWindow *window;
extern android::ANativeWindowCreator::DisplayInfo displayInfo;
extern ImGuiWindow *g_window;
extern int abs_ScreenX, abs_ScreenY;
extern std::unique_ptr<AndroidImgui> graphics;
extern TextureInfo Aekun_image;
extern int native_window_screen_x, native_window_screen_y;

extern BaseTexData *kk_image;
extern ImFont* zh_font;
extern ImFont* icon_font_0;
extern ImFont* icon_font_1;
extern ImFont* icon_font_2;
struct Last_ImRect {
    float Pos_x;
    float Pos_y;
    float Size_x;
    float Size_y;
};
extern struct Last_ImRect LastCoordinate;
extern bool permeate_record;
extern bool permeate_record_ini;
extern void screen_config();

extern game_wwqy game;
extern void drawBegin();
extern void Layout_tick_UI(bool *main_thread_flag);
extern void init_My_drawdata();

extern void read_data();
extern utils gameUtils;


#endif //NATIVESURFACE_DRAW_H
