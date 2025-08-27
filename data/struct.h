#pragma once
#include <cmath>
struct name {
    char text[15];
};

struct boneList {
    uint32_t boneTemp_Male[15] = { 52, 51, 2, 8, 9, 10, 30, 31, 32, 67, 68, 69, 71, 72, 73 };
    uint32_t boneTemp_Female[15] = { 8, 6, 2, 21, 22, 23, 44, 45, 46, 68, 69, 70, 72, 73, 74 };
};

struct D2D {
    float x;
    float y;
    D2D() : x(0), y(0) {}
    D2D(float _x, float _y) : x(_x), y(_y) {}
    D2D operator+(const D2D& rhs) const {
        return D2D(x + rhs.x, y + rhs.y);
    }
    D2D operator-(const D2D& rhs) const {
        return D2D(x - rhs.x, y - rhs.y);
    }
    D2D operator*(float rhs) const {
        return D2D(x * rhs, y * rhs);
    }
};
struct FRotator {
    float Pitch;
    float Yaw;
    float Roll;   
};

struct D3D {
    float x;
    float y;
    float z;

    D3D() : x(0), y(0), z(0) {}
    D3D(float x, float y, float z) : x(x), y(y), z(z) {}

    D3D operator-(const D3D& other) const {
        return D3D(x - other.x, y - other.y, z - other.z);
    }
};

struct Quat
{
    float x;
    float y;
    float z;
    float w;
};

struct FTransform
{
    Quat Rotation;
    D3D Translation;
    D3D Scale3D;
};

struct Vec3 {
    float x,y,w ,h;
};

struct FMatrix
{
    float M[4][4];
};

struct MinimalViewInfo {
    D3D Location;
    FRotator Rotation;
    float FOV;
};  

inline FMatrix TransformToMatrix(const FTransform& transform)
{
    FMatrix matrix;

    float x = transform.Rotation.x;
    float y = transform.Rotation.y;
    float z = transform.Rotation.z;
    float w = transform.Rotation.w;

    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, yy = y * y2, zz = z * z2;
    float xy = x * y2, xz = x * z2, yz = y * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    float sx = transform.Scale3D.x;
    float sy = transform.Scale3D.y;
    float sz = transform.Scale3D.z;

    matrix.M[0][0] = (1.0f - (yy + zz)) * sx;
    matrix.M[0][1] = (xy + wz) * sx;
    matrix.M[0][2] = (xz - wy) * sx;
    matrix.M[0][3] = 0.0f;

    matrix.M[1][0] = (xy - wz) * sy;
    matrix.M[1][1] = (1.0f - (xx + zz)) * sy;
    matrix.M[1][2] = (yz + wx) * sy;
    matrix.M[1][3] = 0.0f;

    matrix.M[2][0] = (xz + wy) * sz;
    matrix.M[2][1] = (yz - wx) * sz;
    matrix.M[2][2] = (1.0f - (xx + yy)) * sz;
    matrix.M[2][3] = 0.0f;

    matrix.M[3][0] = transform.Translation.x;
    matrix.M[3][1] = transform.Translation.y;
    matrix.M[3][2] = transform.Translation.z;
    matrix.M[3][3] = 1.0f;

    return matrix;
}

inline FMatrix RotatorToMatrix(FRotator rotation) {
    float radPitch = rotation.Pitch * M_PI / 180.0f;
    float radYaw = rotation.Yaw * M_PI / 180.0f;
    float radRoll = rotation.Roll * M_PI / 180.0f;
    
    float SP = sinf(radPitch);
    float CP = cosf(radPitch);
    float SY = sinf(radYaw);
    float CY = cosf(radYaw);
    float SR = sinf(radRoll);
    float CR = cosf(radRoll);
    
    FMatrix matrix;
    
    matrix.M[0][0] = (CP * CY);
    matrix.M[0][1] = (CP * SY);
    matrix.M[0][2] = (SP);
    matrix.M[0][3] = 0;
    
    matrix.M[1][0] = (SR * SP * CY - CR * SY);
    matrix.M[1][1] = (SR * SP * SY + CR * CY);
    matrix.M[1][2] = (-SR * CP);
    matrix.M[1][3] = 0;
    
    matrix.M[2][0] = (-(CR * SP * CY + SR * SY));
    matrix.M[2][1] = (CY * SR - CR * SP * SY);
    matrix.M[2][2] = (CR * CP);
    matrix.M[2][3] = 0;
    
    matrix.M[3][0] = 0;
    matrix.M[3][1] = 0;
    matrix.M[3][2] = 0;
    matrix.M[3][3] = 1;
    
    return matrix;
}

inline D2D WorldToScreen(D3D worldLocation,MinimalViewInfo viewInfo,int width, int height) {

    FMatrix tempMatrix = RotatorToMatrix(viewInfo.Rotation);
    D3D vAxisX(tempMatrix.M[0][0], tempMatrix.M[0][1], tempMatrix.M[0][2]);
    D3D vAxisY(tempMatrix.M[1][0], tempMatrix.M[1][1], tempMatrix.M[1][2]);
    D3D vAxisZ(tempMatrix.M[2][0], tempMatrix.M[2][1], tempMatrix.M[2][2]);
    D3D Delta = { worldLocation.x - viewInfo.Location.x, worldLocation.y - viewInfo.Location.y, worldLocation.z - viewInfo.Location.z };
    D3D vTransformed = {
        Delta.x * vAxisY.x + Delta.y * vAxisY.y + Delta.z * vAxisY.z,
        Delta.x * vAxisZ.x + Delta.y * vAxisZ.y + Delta.z * vAxisZ.z,
        Delta.x * vAxisX.x + Delta.y * vAxisX.y + Delta.z * vAxisX.z
    };

    // 读取FOV

    float fov = viewInfo.FOV;
    float screenCenterX = (width / 2.0f);
    float screenCenterY = (height / 2.0f);

    if (vTransformed.z < 1) {
        vTransformed.z = 1;
    }


    return D2D(
        (screenCenterX + vTransformed.x * (screenCenterX / tanf(fov * ((float)M_PI / 360.0f))) / vTransformed.z),
        (screenCenterY - vTransformed.y * (screenCenterX / tanf(fov * ((float)M_PI / 360.0f))) / vTransformed.z)
    );

}

inline float getDis(D3D obj,D3D me) {
    float dx = obj.x - me.x;
    float dy = obj.y - me.y;
    float dz = obj.z - me.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

inline FMatrix MatrixMulti(FMatrix m1, FMatrix m2)
{
    FMatrix matrix = FMatrix();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                matrix.M[i][j] += m1.M[i][k] * m2.M[k][j];
            }
        }
    }
    return matrix;
}

inline D3D MarixToVector(FMatrix matrix)
{
    return D3D(matrix.M[3][0], matrix.M[3][1], matrix.M[3][2]);
}

struct BonePos {
    D2D Bone_Pos[15];
    int bone_count;
};

struct TouchC {
    float fwx;
    float fwy;
    float fwdx;
};