#pragma once

#include "Extended_Interface.hpp"  // у тебя уже должен быть интерфейс engine, client etc.

#include "MinSDK.hpp"
#include "Extended_Interface.hpp"  // оставь, если там Interfaces::

struct FireBulletData
{
    Vector          src;            // start position (eye pos)
    trace_t         enter_trace;    // первый trace
    trace_t         exit_trace;     // trace выхода из поверхности
    Vector          direction;      // направление пули
    int             penetrate_count;
    float           damage;         // оставшийся урон
    float           trace_length;
    float           trace_length_remaining;
    float           current_damage;
    C_BaseCombatWeapon* weapon;
};

class CAutowall
{
public:
    static bool CanHit(const Vector& end_pos, C_BasePlayer* target, float* out_damage = nullptr);
    static bool HandleBulletPenetration(CSWeaponInfo* weapon_data, FireBulletData& data);
    static bool TraceToExit(trace_t& enter_trace, trace_t& exit_trace, Vector start, Vector dir, float max_distance);
    static float GetDamageModifier(int surface_material);
    static bool IsArmored(int hitgroup);
};

extern CAutowall g_Autowall;