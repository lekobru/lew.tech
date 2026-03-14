#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <array>

#include "Extended_Interface.hpp"  // ← основной файл чита, подключаем его

// ────────────────────────────────────────────────
// Базовые типы
// ────────────────────────────────────────────────

struct Vector
{
    float x, y, z;
    Vector() = default;
    Vector(float ix = 0.f, float iy = 0.f, float iz = 0.f) : x(ix), y(iy), z(iz) {}

    Vector operator+(const Vector& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vector operator-(const Vector& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vector operator*(float fl) const { return { x * fl, y * fl, z * fl }; }
};

struct QAngle
{
    float x, y, z;
};

struct matrix3x4_t
{
    float m[3][4];
};

// ────────────────────────────────────────────────
// Интерфейсы Source Engine
// ────────────────────────────────────────────────

class IBaseClientDLL;
class IEngineClient;
class IEngineTrace;
class IVModelInfo;
class IVModelRender;
class ISurface;
class IPanel;
class ICvar;
class IInput;
class IGameEventManager2;
class IMaterialSystem;

// ────────────────────────────────────────────────
// Глобальные указатели на интерфейсы
// ────────────────────────────────────────────────

namespace Interfaces
{
    inline IBaseClientDLL* Client = nullptr;
    inline IEngineClient* Engine = nullptr;
    inline IEngineTrace* Trace = nullptr;
    inline IVModelInfo* ModelInfo = nullptr;
    inline IVModelRender* ModelRender = nullptr;
    inline ISurface* Surface = nullptr;
    inline IPanel* Panel = nullptr;
    inline ICvar* Cvar = nullptr;
    inline IInput* Input = nullptr;
    inline IGameEventManager2* GameEvents = nullptr;
    inline IMaterialSystem* MaterialSys = nullptr;
}

// ────────────────────────────────────────────────
// Получение интерфейсов (вызывается один раз при инжекте)
// ────────────────────────────────────────────────

template<typename T>
T* GetInterface(const char* module_name, const char* interface_name)
{
    HMODULE module = GetModuleHandleA(module_name);
    if (!module) return nullptr;

    auto CreateInterface = reinterpret_cast<uintptr_t>(GetProcAddress(module, "CreateInterface"));
    if (!CreateInterface) return nullptr;

    // Простой обход CreateInterface (без полной версии)
    for (int i = 0; i < 100; ++i)
    {
        char name[128];
        sprintf_s(name, "%s%03d", interface_name, i);

        using CreateFn = void* (__cdecl*)(const char*, int*);
        auto fn = reinterpret_cast<CreateFn>(CreateInterface);
        int ret = 0;
        T* iface = reinterpret_cast<T*>(fn(name, &ret));

        if (iface && ret == 0)
            return iface;
    }

    return nullptr;
}

inline void InitializeInterfaces()
{
    Interfaces::Client = GetInterface<IBaseClientDLL>("client.dll", "VClient");
    Interfaces::Engine = GetInterface<IEngineClient>("engine.dll", "VEngineClient");
    Interfaces::Trace = GetInterface<IEngineTrace>("engine.dll", "EngineTraceClient");
    Interfaces::ModelInfo = GetInterface<IVModelInfo>("engine.dll", "VModelInfoClient");
    Interfaces::ModelRender = GetInterface<IVModelRender>("engine.dll", "VEngineModel");
    Interfaces::Surface = GetInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface");
    Interfaces::Panel = GetInterface<IPanel>("vgui2.dll", "VGUI_Panel");
    Interfaces::Cvar = GetInterface<ICvar>("vstdlib.dll", "VEngineCvar");
    Interfaces::Input = *reinterpret_cast<IInput**>(GetProcAddress(GetModuleHandleA("client.dll"), "g_pInput")); // часто так
    Interfaces::GameEvents = GetInterface<IGameEventManager2>("engine.dll", "GAMEEVENTSMANAGER");
    Interfaces::MaterialSys = GetInterface<IMaterialSystem>("materialsystem.dll", "VMaterialSystem");
}

// ────────────────────────────────────────────────
// Классы сущностей (минимальный набор)
// ────────────────────────────────────────────────

class C_BaseEntity
{
public:
    uintptr_t GetThis() { return reinterpret_cast<uintptr_t>(this); }

    int EntIndex()
    {
        return *reinterpret_cast<int*>(GetThis() + 0x64); // обычно m_iEntIndex
    }

    bool IsDormant()
    {
        return *reinterpret_cast<bool*>(GetThis() + 0xE9); // типичный offset dormant
    }

    Vector GetAbsOrigin()
    {
        return *reinterpret_cast<Vector*>(GetThis() + 0x134); // m_vecOrigin
    }

    Vector GetVelocity()
    {
        return *reinterpret_cast<Vector*>(GetThis() + 0x140); // m_vecVelocity
    }

    float GetSimulationTime()
    {
        return *reinterpret_cast<float*>(GetThis() + 0x15C); // m_flSimulationTime
    }

    bool SetupBones(matrix3x4_t* bone_matrix_out, int max_bones, int bone_mask, float cur_time);
};

class C_BasePlayer : public C_BaseEntity
{
public:
    int GetHealth()
    {
        return *reinterpret_cast<int*>(GetThis() + 0x90); // m_iHealth
    }

    int GetTeam()
    {
        return *reinterpret_cast<int*>(GetThis() + 0xF0); // m_iTeamNum
    }

    bool IsAlive()
    {
        int hp = GetHealth();
        return hp > 0 && hp <= 100;
    }
};

// ────────────────────────────────────────────────
// Удобные хелперы
// ────────────────────────────────────────────────
