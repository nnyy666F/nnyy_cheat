#pragma once

#include <linux/input.h>
#include <vector>
#include <functional>
#include "VectorStruct.h"
#define maxE 5
#define maxF 10
#define UNGRAB 0
#define GRAB 1

namespace Touch {
    enum class TouchAction {
        NONE,   // 无操作
        DOWN,   // 按下操作
        MOVE    // 移动操作
    };
    struct touchObj {
        int id = 0;
        Vector2 pos = {0, 0};
        bool isDown = false;      
        TouchAction action = TouchAction::NONE; 
    };
    
    struct Device {
        int fd = 0;
        struct input_absinfo absX; 
        struct input_absinfo absY;
        float S2TX = 0.0f;
        float S2TY = 0.0f;
        touchObj Finger[maxF] = {};
    };

    bool Init(const Vector2 &s, bool p_readOnly);

    void Close();

    void Down(float x, float y);

    void Move(float x, float y);

    void Up();

    void Move(touchObj *touch, float x, float y);
    
    std::vector<Vector2> GetAllFingerPositions();

    void Upload();
    
    void SetTouchObstacle(Vector2 *Pos, Vector2 *Size, int Count);

    void SetCallBack(const std::function<void(std::vector<Device> *)> &cb);

    Vector2 Touch2Screen(const Vector2 &coord);

    Vector2 GetScale();

    void setOrientation(int orientation);

    void setOtherTouch(bool p_otherTouch);
}
