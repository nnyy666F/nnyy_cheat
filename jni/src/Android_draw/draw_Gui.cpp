
#include "draw.h"
#include <sstream>
#include <iomanip>
#include <iostream>
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
#include <vector>
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
std::unique_ptr<AndroidImgui> graphics;
ImFont *zh_font = NULL;
ImFont *icon_font_0 = NULL;
ImFont *icon_font_1 = NULL;
ImFont *icon_font_2 = NULL;
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
struct PackagePID
{
	std::string pkgName;
	int pid;
};

void LoadConfig(bool isSync)
{
	if (config == "已加载" && !isSync)
		return;

	FILE *file = fopen("config.txt", "r");
	if (!file)
	{
		std::cerr << "配置文件不存在，使用默认值" << std::endl;
		config = "已加载";
		return;
	}

	char buffer[256] = {0};
	if (fgets(buffer, sizeof(buffer), file))
	{
		char *token = strtok(buffer, ",");
		if (token && isSync)
		{
			game.FriX =
				std::clamp(static_cast<float>(atof(token)), 0.0f,
						   static_cast<float>(game.px * 2));
		}

		token = strtok(nullptr, ",");
		if (token && isSync)
		{
			game.FriY =
				std::clamp(static_cast<float>(atof(token)), 0.0f,
						   static_cast<float>(game.py * 2));
		}

		token = strtok(nullptr, ",");
		if (token)
			game.FriSize = std::clamp(static_cast<float>(atof(token)), 0.0f, 100.0f);

		token = strtok(nullptr, ",");
		if (token)
			game.face = std::clamp(atoi(token), 0, 1);

		token = strtok(nullptr, ",");
		if (token)
			game.aimbot_status = std::clamp(atoi(token), 0, 2);

		token = strtok(nullptr, ",");
		if (token)
		{
			game.aimbot_range =
				std::clamp(static_cast<float>(atof(token)), 0.0f,
						   15.0f);
		}

		token = strtok(nullptr, ",");
		if (token)
			game.aimbot_press_range = std::clamp(static_cast<float>(atof(token)), 0.0f, 50.0f);

		token = strtok(nullptr, ",");
		if (token)
			game.isShow_aimbot_range = std::clamp(atoi(token), 0, 1);

		token = strtok(nullptr, ",");
		if (token)
			game.ishasP = std::clamp(atoi(token), 0, 1);

		token = strtok(nullptr, ",");
		if (token)
			game.aimbot_pull_time = std::clamp(strtof(token, nullptr), 0.0f, 1.0f);
			
		token = strtok(nullptr, ",");
		if (token)
			permeate_record = std::clamp(atoi(token), 0, 1);	
	}

	fclose(file);
	config = "已加载";
}

bool SaveConfig()
{
	FILE *file = fopen("config.txt", "w");
	if (!file)
	{
		std::cerr << "Failed to open config.txt for writing!" << std::endl;
		return false;
	}
	fprintf(file, "%.2f,%.2f,%.2f,%d,%d,%f,%f,%d,%d,%f,%d",
			game.FriX,
			game.FriY,
			game.FriSize,
			game.face,
			game.aimbot_status,
			game.aimbot_range, game.aimbot_press_range, game.isShow_aimbot_range, game.ishasP, game.aimbot_pull_time,permeate_record);

	fclose(file);
	return true;
}

bool M_Android_LoadFont(float SizePixels)
{
	ImGuiIO &io = ImGui::GetIO();
	static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 3.0;
	icons_config.OversampleV = 3.0;
	icons_config.SizePixels = SizePixels;
	::icon_font_0 =
		io.Fonts->AddFontFromMemoryCompressedTTF((const void *)&font_awesome_brands_compressed_data,
												 sizeof(font_awesome_brands_compressed_data), 0.0f,
												 &icons_config, icons_ranges);
	::icon_font_1 =
		io.Fonts->AddFontFromMemoryCompressedTTF((const void *)&font_awesome_regular_compressed_data,
												 sizeof(font_awesome_regular_compressed_data), 0.0f,
												 &icons_config, icons_ranges);
	::icon_font_2 =
		io.Fonts->AddFontFromMemoryCompressedTTF((const void *)&font_awesome_solid_compressed_data,
												 sizeof(font_awesome_solid_compressed_data), 0.0f,
												 &icons_config, icons_ranges);

	io.Fonts->AddFontDefault();
	return zh_font != nullptr;
}

void init_My_drawdata()
{
	ImGui::My_Android_LoadSystemFont(25.0f);
	M_Android_LoadFont(25.0f);
	// ::kk_image = graphics->LoadTextureFromMemory((void
	// *)picture_ZhenAiKun_PNG_H, sizeof(picture_ZhenAiKun_PNG_H));
}

void screen_config()
{
	::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin()
{
	if (::permeate_record_ini)
	{
		if (::g_window != nullptr)
		{
			LastCoordinate.Pos_x = ::g_window->Pos.x;
			LastCoordinate.Pos_y = ::g_window->Pos.y;
			LastCoordinate.Size_x = ::g_window->Size.x;
			LastCoordinate.Size_y = ::g_window->Size.y;
		}
		else
		{
			LastCoordinate = {100, 125, 400, 600};
		}
		if (graphics != nullptr)
		{
			graphics->Shutdown();
		}
		else
		{
			std::cerr << "对象为空" << std::endl;
		}
		if (::window != nullptr)
		{
			android::ANativeWindowCreator::Destroy(::window);
		}
		::window = android::ANativeWindowCreator::Create("test",
														 native_window_screen_x,
														 native_window_screen_y,
														 permeate_record);
		if (::window == nullptr)
		{
			std::cerr << "窗口创建失败" << std::endl;
			permeate_record_ini = false;
			return;
		}
		if (graphics != nullptr)
		{
			if (!graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y))
			{
				std::cerr << "渲染初始化衰败" << std::endl;
				android::ANativeWindowCreator::Destroy(::window);
				permeate_record_ini = false;
				return;
			}
		}

		::init_My_drawdata();

		switch (style_idx)
		{
		case 0:
			ImGui::StyleColorsLight();
			break;
		case 1:
			ImGui::StyleColorsDark();
			break;
		case 2:
			ImGui::StyleColorsClassic();
			break;
		}

		permeate_record_ini = false;
	}
	screen_config();
	if (::orientation != displayInfo.orientation)
	{
		::orientation = displayInfo.orientation;
		Touch::setOrientation(displayInfo.orientation);
		if (g_window != NULL)
		{
			g_window->Pos.x = 100;
			g_window->Pos.y = 125;
		}
	}
}

struct GameFrameData
{
	int eneity_count;		  // 敌人数量
	float r_xx[100];		  // 敌人X坐标
	float r_yy[100];		  // 敌人Y坐标
	float r_ww[100];		  // 敌人宽度
	bool isMiss[100];		  // 是否未命中
	float hp[100];			  // 敌人血量
	float armor[100];		  // 敌人护甲
	bool isInit[100];		  // 是否初始化
	name text[100];			  // 敌人名称
	float dis[100];			  // 敌人距离
	int teamID[100];		  // 敌人队伍ID
	WeaponType priority[100]; // 武器类型
	BonePos Bone_Pos[100];	  // 骨骼位置
	D3D pos[100];			  // 敌人世界坐标（用于数据变化检测）
};

// 2. 双缓冲核心变量（读写分离+线程同步）
static GameFrameData g_buffers[2] = {0};						// 两个缓冲区（0：写缓冲，1：读缓冲）
static int g_write_idx = 0;										// 写缓冲区索引（0/1切换）
static int g_read_idx = 1;										// 读缓冲区索引（0/1切换）
static bool g_is_new_data = false;								// 新数据就绪标记
static pthread_mutex_t g_buf_mutex = PTHREAD_MUTEX_INITIALIZER; // 缓冲锁
static pthread_cond_t g_buf_cond = PTHREAD_COND_INITIALIZER;	// 唤醒条件变量

void *read_data_thread(void *arg)
{
	read_data_status.store(1, std::memory_order_release);

	pthread_mutex_lock(&game_data_mutex);
	read_data();
	bool init_ok = (game.addr != 0 && game.Gworld != 0);
	if (!init_ok)
	{
		read_data_status.store(-1, std::memory_order_release);
		pthread_mutex_unlock(&game_data_mutex);
		is_thread_running.store(false, std::memory_order_release);
		puts("基址获取失败了.......");
		return nullptr;
	}
	read_data_status.store(2, std::memory_order_release);
	pthread_mutex_unlock(&game_data_mutex);
	is_thread_running.store(false, std::memory_order_release);

	while (!stop_update_thread.load(std::memory_order_acquire))
	{
		pthread_mutex_lock(&game_data_mutex);
		game.update();
			pthread_mutex_lock(&g_buf_mutex);		
			GameFrameData &write_buf = g_buffers[g_write_idx];

			write_buf.eneity_count = game.eneity_count;
			memcpy(write_buf.r_xx, game.r_xx, sizeof(game.r_xx));
			memcpy(write_buf.r_yy, game.r_yy, sizeof(game.r_yy));
			memcpy(write_buf.r_ww, game.r_ww, sizeof(game.r_ww));
			memcpy(write_buf.isMiss, game.isMiss, sizeof(game.isMiss));
			memcpy(write_buf.hp, game.hp, sizeof(game.hp));
			memcpy(write_buf.armor, game.armor, sizeof(game.armor));
			memcpy(write_buf.isInit, game.isInit, sizeof(game.isInit));
			memcpy(write_buf.text, game.text, sizeof(game.text));
			memcpy(write_buf.dis, game.dis, sizeof(game.dis));
			memcpy(write_buf.teamID, game.teamID, sizeof(game.teamID));
			memcpy(write_buf.priority, game.priority, sizeof(game.priority));
			memcpy(write_buf.Bone_Pos, game.Bone_Pos, sizeof(game.Bone_Pos));
			memcpy(write_buf.pos, game.pos, sizeof(game.pos));
			g_write_idx = 1 - g_write_idx;
			g_is_new_data = true;
			pthread_cond_signal(&g_buf_cond);

			pthread_mutex_unlock(&g_buf_mutex);	
		
		pthread_mutex_unlock(&game_data_mutex);
		usleep((100 * 10000) / game.imgui_fps);
	}

	pthread_mutex_destroy(&g_buf_mutex);
	pthread_cond_destroy(&g_buf_cond);
	printf("更新线程已停止\n");
	return nullptr;
}

void *start_touch(void *arg)
{
	game.Touch();
	return nullptr;
}

void get_buffered_game_data(GameFrameData &out_frame)
{
	pthread_mutex_lock(&g_buf_mutex);
	while (!g_is_new_data && !stop_update_thread.load(std::memory_order_acquire))
	{
		pthread_cond_wait(&g_buf_cond, &g_buf_mutex);
	}
	if (stop_update_thread.load(std::memory_order_acquire))
	{
		pthread_mutex_unlock(&g_buf_mutex);
		memset(&out_frame, 0, sizeof(out_frame));
		return;
	}
	out_frame = g_buffers[g_read_idx];
	// 切换读索引（与写索引同步）
	g_read_idx = 1 - g_read_idx;
	g_is_new_data = false; // 重置新数据标记

	pthread_mutex_unlock(&g_buf_mutex);
}

void Layout_tick_UI(bool *main_thread_flag)
{
	WindowCount = 0;
	static bool show_draw_Line = false;
	static bool show_another_window = false;

	static std::vector<PackagePID> pkgPidList;
	static bool hasDetected = false; // 是否已执行过检测
	static bool initSuccess = false; // 是否已成功初始化river
	static int selectedPkgIdx = -1;
	static const int kDefaultPkgCount = sizeof(gameUtils.packageNames) / sizeof(gameUtils.packageNames[0]); // 默认包名数量

	// 初始化互斥锁（仅第一次调用时执行）
	static bool mutex_inited = false;
	if (!mutex_inited)
	{
		pthread_mutex_init(&game_data_mutex, nullptr);
		mutex_inited = true;
	}

	LoadConfig(false);
	{
		static float f = 0.0f;
		static int counter = 0;
		static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		std::string title_buf(64, '\0');
		auto tempFPS = static_cast<int>(ImGui::GetIO().Framerate);
		std::snprintf(&title_buf[0], title_buf.size(), "鲶鱼辅助--%dFPS ###Menu", tempFPS);
		ImGui::Begin(title_buf.c_str());

		if (::permeate_record_ini)
		{
			ImGui::SetWindowPos(
				{LastCoordinate.Pos_x, LastCoordinate.Pos_y});
			ImGui::SetWindowSize(
				{LastCoordinate.Size_x, LastCoordinate.Size_y});
			permeate_record_ini = false;
		}
		ImGui::Text("渲染接口 : %s, gui版本 : %s", graphics->RenderName, IMGUI_VERSION);
		if (ImGui::Combo("##主题", &style_idx, "白色主题\0蓝色主题\0紫色主题\0"))
		{
			switch (style_idx)
			{
			case 0:
				ImGui::StyleColorsLight();
				break;
			case 1:
				ImGui::StyleColorsDark();
				break;
			case 2:
				ImGui::StyleColorsClassic();
				break;
			}
		}
		if (ImGui::
				Checkbox("不允许录屏? \xef\x8d\xa8 \xef\x91\x9f \xef\x9a\xad",
						 &::permeate_record))
		{
			::permeate_record_ini = true;
		}
		ImGui::Checkbox("我有共存包名?", &game.ishasP);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "应用平均 %.3f ms/frame (%.1f FPS)",
						   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		game.imgui_fps = ImGui::GetIO().Framerate < 60 ? 60 : ImGui::GetIO().Framerate;
		if (ImGui::Button("退出辅助"))
		{
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

		ImGui::Separator();
		ImGui::Text("【游戏进程管理】");
		ImGui::Spacing();

		// 检测按钮：点击触发PID扫描（仅未检测/未初始化时显示）
		if (!hasDetected || !initSuccess)
		{
			if (ImGui::Button("开始检测游戏进程"))
			{
				::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
				int screenWidth = displayInfo.width;
				int screenHeight = displayInfo.height;

				pkgPidList.clear(); // 清空历史数据
				selectedPkgIdx = -1;
				initSuccess = false;
				read_data_status.store(0, std::memory_order_release);

				if (!game.ishasP)
				{
					for (int i = 0; i < kDefaultPkgCount; i++)
					{
						PackagePID item;
						item.pkgName = gameUtils.packageNames[i];
						char pkgBuf[256] = {0};
						strncpy(pkgBuf, item.pkgName.c_str(), sizeof(pkgBuf) - 1);
						item.pid = getPID(pkgBuf);
						pkgPidList.push_back(item);
					}
				}
				else
				{
					FILE *pkg_file = fopen("/data/page.txt", "r");
					if (pkg_file)
					{
						char buffer[1024] = {0};
						if (fgets(buffer, sizeof(buffer) - 1, pkg_file))
						{
							buffer[strcspn(buffer, "\n\r")] = '\0';
							char *pkg = strtok(buffer, ",");
							while (pkg)
							{
								char *trim_pkg = pkg;
								while (*trim_pkg == ' ' || *trim_pkg == '\t')
									trim_pkg++;
								if (*trim_pkg == '\0')
								{
									pkg = strtok(nullptr, ",");
									continue;
								}
								char pkgBuf[256] = {0};
								strncpy(pkgBuf, trim_pkg, sizeof(pkgBuf) - 1);
								PackagePID item;
								item.pkgName = trim_pkg;
								item.pid = getPID(pkgBuf);
								pkgPidList.push_back(item);
								pkg = strtok(nullptr, ",");
							}
						}
						else
						{
							printf("错误：/data/page.txt为空，未读取到包名\n");
						}
						fclose(pkg_file);
					}
					else
					{
						printf("错误：无法打开/data/page.txt（可能权限不足或文件不存在）\n");
					}
				}

				std::vector<PackagePID> validPkgPidList;
				for (auto &item : pkgPidList)
				{
					if (item.pid > 0)
					{
						validPkgPidList.push_back(item);
					}
				}
				pkgPidList.swap(validPkgPidList);

				hasDetected = true;
				game.setPxPy(screenWidth / 2, screenHeight / 2);
				LoadConfig(true);
			}
			ImGui::Spacing();
		}

		if (hasDetected)
		{
			int validCount = pkgPidList.size();
			if (validCount == 0)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
								   "❌ 未检测到任何运行的游戏进程");
				if (ImGui::Button("重新检测"))
				{
					hasDetected = false; // 重置检测状态
					pkgPidList.clear();
					selectedPkgIdx = -1;
				}
			}
			// 情况2：仅1个有效进程（自动选中）
			else if (validCount == 1)
			{
				selectedPkgIdx = 0;
				auto &selectedItem = pkgPidList[selectedPkgIdx];
				ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
								   "✅ 检测到1个游戏进程：");
				ImGui::Text("包名：%s", selectedItem.pkgName.c_str());
				ImGui::Text("PID：%d", selectedItem.pid);
				ImGui::Spacing();

				if (!initSuccess && ImGui::Button("初始化进程连接"))
				{
					river_init(selectedItem.pid);
					initSuccess = true;
				}
				if (initSuccess)
				{
					ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f),
									   "✅ 进程连接成功！");
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
								   "✅ 检测到%d个游戏进程，请选择：", validCount);
				ImGui::Spacing();

				// 遍历显示所有有效进程，点击按钮选中
				for (int i = 0; i < validCount; i++)
				{
					auto &item = pkgPidList[i];
					char btnText[256];
					snprintf(btnText, sizeof(btnText), "%d. 包名：%s（PID：%d）", i + 1,
							 item.pkgName.c_str(), item.pid);
					if (ImGui::Button(btnText))
					{
						selectedPkgIdx = i;
					}
					ImGui::Spacing();
				}
				if (selectedPkgIdx != -1)
				{
					auto &selectedItem = pkgPidList[selectedPkgIdx];
					ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f),
									   "已选择：包名=%s，PID=%d",
									   selectedItem.pkgName.c_str(), selectedItem.pid);

					if (!initSuccess && ImGui::Button("确认初始化进程连接"))
					{
						river_init(selectedItem.pid);
						initSuccess = true;
						// base::cott();
					}

					if (initSuccess)
					{
						ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.8f, 1.0f),
										   "✅ 进程连接成功！");
					}
				}
			}
			if (initSuccess)
			{
				ImGui::Text("【开火键区域设置】");
				ImGui::SliderFloat("区域大小", &game.FriSize, 0, 100);
				ImGui::SliderFloat("区域X坐标", &game.FriX, 0, displayInfo.width);
				ImGui::SliderFloat("区域Y坐标", &game.FriY, 0, displayInfo.height);
				ImGui::Spacing();

				auto FriSizeScale = 1.3 + (game.FriSize * 1.3 - 50) * 0.01;
				auto fwx = displayInfo.width - 125 * FriSizeScale - game.FriX;
				auto fwy = 125 * FriSizeScale + game.FriY;
				game.ctrl.fwx = fwx;
				game.ctrl.fwy = fwy;
				game.ctrl.fwdx = 125 * FriSizeScale;

				ImDrawList *draw_list = ImGui::GetBackgroundDrawList();

				draw_list->AddCircleFilled(ImVec2(fwx, fwy), game.ctrl.fwdx,
										   IM_COL32(255, 255, 255, 64));

				ImVec2 text_pos = ImVec2(fwx - 35 / 2.0f, fwy - 35 / 2.0f);
				draw_list->AddText(zh_font, 25.0f, text_pos, IM_COL32(0, 0, 0, 255),
								   "\t\t开火键区域\n(请手动调节至开火键上！)");
				ImGui::Spacing();
				ImGui::Text("【自瞄与压枪设置】");
				ImGui::SliderFloat("身位大小", &game.aimbot_range, 0.0f, 15.0f);
				ImGui::SliderFloat("压枪力度", &game.aimbot_press_range, 0.0f, 50.0f);
				ImGui::SliderFloat("拉枪延时(切换)", &game.aimbot_pull_time, 0.0f, 1.0f);
				ImGui::Checkbox("显示自瞄范围", &game.isShow_aimbot_range);
				ImGui::Spacing();

				if (game.isShow_aimbot_range)
				{
					ImGui::GetForegroundDrawList()->AddCircle(ImVec2(game.px, game.py),
															  game.aimbot_range * game.rectWidth, ImColor(0, 255, 0, 150), 0,
															  1.6f);
					if (game.aimbot_pull_time > 0.0f)
					{
						ImGui::GetForegroundDrawList()->AddCircle(ImVec2(game.px, game.py),
																  0.25 * game.aimbot_range * game.rectWidth, ImColor(0, 255, 255, 150), 0,
																  1.6f);
					}
				}
				int current_status = read_data_status.load(std::memory_order_acquire);
				bool thread_running = is_thread_running.load(std::memory_order_acquire);

				if ((current_status == 0 || current_status == -1) && !thread_running)
				{
					if (ImGui::Button("初始化游戏数据（启动线程）"))
					{
						pthread_t tid;
						int ret = pthread_create(&tid, nullptr, read_data_thread, nullptr);
						if (ret == 0)
						{
							pthread_detach(tid);
							is_thread_running.store(true, std::memory_order_release);
						}
						else
						{
							read_data_status.store(-1, std::memory_order_release);
							ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
											   "❌ 数据线程创建失败！错误码：%d",
											   ret);
						}

						// 启动触摸控制线程
						pthread_t tid2;
						int ret2 = pthread_create(&tid2, nullptr, start_touch, nullptr);
						if (ret2 == 0)
						{
							pthread_detach(tid2);
						}
					}
				}
				// 数据初始化中
				else if (current_status == 1 && thread_running)
				{
					ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f),
									   "⏳ 游戏数据初始化中...（请勿重复点击）");
				}
				else if (current_status == 2)
				{
					ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
									   "✅ 游戏数据初始化完成！");
					ImGui::Spacing();
					GameFrameData frame_data;
					get_buffered_game_data(frame_data);

					for (int i = 0; i < frame_data.eneity_count; i++)
					{
						if (!frame_data.isInit[i])
							continue;

						game.view =
							{
								frame_data.r_xx[i] - (frame_data.r_yy[i] - frame_data.r_ww[i]) / 24,
								frame_data.r_yy[i],
								(frame_data.r_yy[i] - frame_data.r_ww[i]) / 2,
								(frame_data.r_yy[i] - frame_data.r_ww[i])};

						auto x_start = game.view.x + game.view.w / 2;
						auto x_start2 = game.view.x + game.view.h / 4;
						auto x_start3 = game.view.y + game.view.h / 3;
						auto x_start4 = game.view.x + game.view.w;
						auto y_start2 = game.view.x - game.view.h / 4;
						auto y_start = game.view.y - game.view.h + game.view.h / 2;

						ImDrawList *draw_list = ImGui::GetBackgroundDrawList();
						if ((frame_data.r_yy[i] - frame_data.r_ww[i]) / 2 < 0)
						{
							if (frame_data.dis[i] <= 25 && !frame_data.isMiss[i])
							{
								draw_list->AddLine(ImVec2(game.px, 100),
												   ImVec2(-game.view.x + displayInfo.width,
														  game.view.y - game.view.w + displayInfo.height),
												   IM_COL32(255, 0, 0, 255), 2);
							}
							continue;
						}

						game.rect_w[i] = x_start2 - y_start2;

						ImVec2 rect_min = ImVec2(x_start2, x_start3);
						ImVec2 rect_max = ImVec2(y_start2, y_start);

						auto color = IM_COL32(0, 255, 0, 255);
						switch (frame_data.priority[i])
						{
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

						draw_list->AddRect(rect_min,
										   rect_max,
										   frame_data.isMiss[i] ? IM_COL32(255, 255, 0, 255) : color,
										   0.0f, ImDrawFlags_RoundCornersAll, 1.0f);

						if (!frame_data.isMiss[i])
						{
							draw_list->AddLine(ImVec2(game.px, 100),
											   ImVec2(game.view.x, y_start - 40),
											   IM_COL32(255, 255, 255, 200), 2);
						}

						if (frame_data.hp[i] < 100.0f)
						{
							auto rect_center = 4 * (y_start2 - x_start2) / 2;

							float hpjd =
								game.view.y - (frame_data.hp[i] < 10 ? 10 : frame_data.hp[i]) / 100 * game.view.w * 2 +
								game.view.h / 2;
							draw_list->AddLine(ImVec2(x_start4 + 4 + rect_center / 6, hpjd),
											   ImVec2(x_start4 + 4 + rect_center / 6,
													  game.view.y - 100 / 100 * game.view.w * 2 + game.view.h / 2),
											   IM_COL32(255, 255, 255, 255), 2);
							draw_list->AddLine(ImVec2(x_start4 + 4 + rect_center / 6, x_start3),
											   ImVec2(x_start4 + 4 + rect_center / 6, hpjd),
											   IM_COL32(220, 20, 61, 200), 2);

							float hjjd =
								game.view.y - (frame_data.armor[i] < 10 ? 10 : frame_data.armor[i]) / 50 * game.view.w * 2 +
								game.view.h / 2;
							draw_list->AddLine(ImVec2(x_start4 + 4 + rect_center, hjjd),
											   ImVec2(x_start4 + 4 + rect_center,
													  game.view.y - 50 / 50 * game.view.w * 2 + game.view.h / 2),
											   IM_COL32(255, 255, 255, 200), 2);
							draw_list->AddLine(ImVec2(x_start4 + 4 + rect_center, x_start3),
											   ImVec2(x_start4 + 4 + rect_center, hjjd),
											   IM_COL32(0, 255, 0, 200), 2);
						}

						draw_list->AddText(zh_font, 30.0f, ImVec2(game.view.x - 40, y_start - 45),
										   IM_COL32(255, 255, 0, 255), frame_data.text[i].text);

						std::string t = "[" + std::to_string(int(frame_data.dis[i])) + " M]";
						draw_list->AddText(zh_font, 20.0f, ImVec2(game.view.x + 20, y_start - 20),
										   IM_COL32(255, 255, 255, 255), t.c_str());

						t = "[" + std::to_string(frame_data.teamID[i]) + " ]";
						draw_list->AddText(zh_font, 20.0f, ImVec2(game.view.x - 35, y_start - 18),
										   IM_COL32(0, 255, 0, 255), t.c_str());

						// 绘制骨骼
						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[0].x, frame_data.Bone_Pos[i].Bone_Pos[0].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[1].x, frame_data.Bone_Pos[i].Bone_Pos[1].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 头 → 胸

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[1].x, frame_data.Bone_Pos[i].Bone_Pos[1].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[2].x, frame_data.Bone_Pos[i].Bone_Pos[2].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 胸 → 骨盆

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[1].x, frame_data.Bone_Pos[i].Bone_Pos[1].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[3].x, frame_data.Bone_Pos[i].Bone_Pos[3].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 胸 → 左肩

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[3].x, frame_data.Bone_Pos[i].Bone_Pos[3].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[4].x, frame_data.Bone_Pos[i].Bone_Pos[4].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 左肩 → 左肘

						D2D leftElbow = {frame_data.Bone_Pos[i].Bone_Pos[4].x, frame_data.Bone_Pos[i].Bone_Pos[4].y};
						D2D originalLeftHand = {frame_data.Bone_Pos[i].Bone_Pos[5].x, frame_data.Bone_Pos[i].Bone_Pos[5].y};
						float leftArmLength = CalcDistance(leftElbow, originalLeftHand);

						const float NORMAL_ARM_LENGTH = 10.0f;
						ImVec2 leftHandTarget = ImVec2(originalLeftHand.x, originalLeftHand.y);

						if (leftArmLength > NORMAL_ARM_LENGTH)
						{
							ImVec2 dir;
							dir.x = originalLeftHand.x - leftElbow.x;
							dir.y = originalLeftHand.y - leftElbow.y;

							dir.x = dir.x / leftArmLength;
							dir.y = dir.y / leftArmLength;

							leftHandTarget.x = leftElbow.x + dir.x * NORMAL_ARM_LENGTH;
							leftHandTarget.y = leftElbow.y + dir.y * NORMAL_ARM_LENGTH;
						}
						draw_list->AddLine(ImVec2(leftElbow.x, leftElbow.y), leftHandTarget,
										   IM_COL32(255, 0, 0, 255), 1.0f);

						D2D rightElbow = {frame_data.Bone_Pos[i].Bone_Pos[7].x, frame_data.Bone_Pos[i].Bone_Pos[7].y};
						D2D originalRightHand = {frame_data.Bone_Pos[i].Bone_Pos[8].x, frame_data.Bone_Pos[i].Bone_Pos[8].y};
						float rightArmLength = CalcDistance(rightElbow, originalRightHand);
						ImVec2 rightHandTarget = ImVec2(originalRightHand.x, originalRightHand.y);

						if (rightArmLength > NORMAL_ARM_LENGTH)
						{
							ImVec2 dir;
							dir.x = originalRightHand.x - rightElbow.x;
							dir.y = originalRightHand.y - rightElbow.y;
							dir.x = dir.x / rightArmLength;
							dir.y = dir.y / rightArmLength;
							rightHandTarget.x = rightElbow.x + dir.x * NORMAL_ARM_LENGTH;
							rightHandTarget.y = rightElbow.y + dir.y * NORMAL_ARM_LENGTH;
						}
						draw_list->AddLine(ImVec2(rightElbow.x, rightElbow.y), rightHandTarget,
										   IM_COL32(255, 0, 0, 255), 1.0f);

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[2].x, frame_data.Bone_Pos[i].Bone_Pos[2].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[9].x, frame_data.Bone_Pos[i].Bone_Pos[9].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 骨盆 → 左臀

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[9].x, frame_data.Bone_Pos[i].Bone_Pos[9].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[10].x, frame_data.Bone_Pos[i].Bone_Pos[10].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 左臀 → 左膝

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[10].x, frame_data.Bone_Pos[i].Bone_Pos[10].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[11].x, frame_data.Bone_Pos[i].Bone_Pos[11].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 左膝 → 左脚

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[2].x, frame_data.Bone_Pos[i].Bone_Pos[2].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[12].x, frame_data.Bone_Pos[i].Bone_Pos[12].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 骨盆 → 右臀

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[12].x, frame_data.Bone_Pos[i].Bone_Pos[12].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[13].x, frame_data.Bone_Pos[i].Bone_Pos[13].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 右臀 → 右膝

						draw_list->AddLine(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[13].x, frame_data.Bone_Pos[i].Bone_Pos[13].y),
										   ImVec2(frame_data.Bone_Pos[i].Bone_Pos[14].x, frame_data.Bone_Pos[i].Bone_Pos[14].y),
										   IM_COL32(255, 0, 0, 255), 1.0f); // 右膝 → 右脚

						draw_list->AddCircle(ImVec2(frame_data.Bone_Pos[i].Bone_Pos[0].x, frame_data.Bone_Pos[i].Bone_Pos[0].y),
											 7.0f, ImColor(255, 0, 0, 255), 0, 4.0f);
					}
				}
				// 初始化失败
				else if (current_status == -1)
				{
					ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
									   "❌ 数据初始化失败！请检查驱动或进程状态");
					if (!thread_running && ImGui::Button("重新初始化数据"))
					{
						read_data_status.store(0, std::memory_order_release); // 重置状态允许重试
					}
				}
			}
		}

		WindowPos[WindowCount] = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		WindowSize[WindowCount] = Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
		WindowCount++;
		ImGui::End();
	}

	Touch::SetTouchObstacle(WindowPos, WindowSize, WindowCount);
}

void read_data()
{
	game.load_init();
}