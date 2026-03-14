#include "Autowall.hpp"
// √лобальный объект
CAutowall g_Autowall;

// ѕолучаем weapon info (как в твоЄм Item_Post_Frame или где-то уже есть)
CSWeaponInfo* GetWeaponInfo(C_BaseCombatWeapon* weapon)
{
    if (!weapon) return nullptr;
    int id = weapon->GetWeaponID();
    return Interfaces::WeaponSystem->GetWeaponInfo(id);  // или твой способ получени€
}

// ќсновна€ функци€ Ч можно ли попасть в точку с учЄтом пробити€
bool CAutowall::CanHit(const Vector& end_pos, C_BasePlayer* target, float* out_damage)
{

    if (!Interfaces::Engine->IsInGame() || !LocalPlayer || !target || !target->IsAlive())
        return false;

    C_BaseCombatWeapon* weapon = LocalPlayer->GetActiveWeapon();
    if (!weapon) return false;

    CSWeaponInfo* weapon_data = GetWeaponInfo(weapon);
    if (!weapon_data) return false;

    Vector eye_pos = LocalPlayer->GetEyePosition();
    Vector dir = (end_pos - eye_pos).Normalized();

    FireBulletData data;
    data.src = eye_pos;
    data.direction = dir;
    data.penetrate_count = 4;               // макс проникновений в CSS
    data.current_damage = (float)weapon_data->m_iDamage;
    data.weapon = weapon;
    data.trace_length = 0.f;
    data.trace_length_remaining = weapon_data->m_flRange;

    // ѕервый trace до цели
    Ray_t ray;
    ray.Init(data.src, end_pos);
    CTraceFilter filter;
    filter.pSkip = LocalPlayer;
    Interfaces::EngineTrace->TraceRay(ray, MASK_SHOT_HULL, &filter, &data.enter_trace);

    // ≈сли попали в цель без стен Ч просто считаем урон
    if (data.enter_trace.m_pEnt == target)
    {
        if (out_damage) *out_damage = data.current_damage;
        return true;
    }

    // »наче Ч пытаемс€ пробить
    if (!HandleBulletPenetration(weapon_data, data))
        return false;

    // ѕосле всех пенетраций провер€ем, дошли ли до цели
    if (data.current_damage >= 1.f)
    {
        if (out_damage) *out_damage = data.current_damage;
        return true;
    }

    return false;
}

// —ама€ важна€ Ч обработка пенетрации
bool CAutowall::HandleBulletPenetration(CSWeaponInfo* wpn_data, FireBulletData& data)
{
    float modifier, damage_modifier;
    C_BasePlayer* pPlayer = nullptr;

    while (data.penetrate_count > 0 && data.current_damage >= 1.f)
    {
        data.trace_length_remaining -= data.enter_trace.fraction * data.trace_length_remaining;

        Vector end = data.src + data.direction * (data.trace_length_remaining + 0.1f);

        if (!TraceToExit(data.enter_trace, data.exit_trace, data.enter_trace.endpos, data.direction, wpn_data->m_flRangeModifier))
            return false;

        modifier = GetDamageModifier(data.exit_trace.surface.flags);

        data.current_damage *= pow(wpn_data->m_flRangeModifier, data.trace_length_remaining * 0.0025f);

        if (data.enter_trace.hitgroup != 0)
        {
            pPlayer = (C_BasePlayer*)data.enter_trace.m_pEnt;
            if (!pPlayer || !pPlayer->IsPlayer() || pPlayer->GetTeam() == LocalPlayer->GetTeam())
                return false;

            ScaleDamage(pPlayer, data.enter_trace.hitgroup, wpn_data, data.current_damage);

            if (out_damage) *out_damage = data.current_damage;  // optional
            return true;
        }

        if (!data.exit_trace.DidHit() || data.exit_trace.surface.flags & SURF_SKY)
            return false;

        data.src = data.exit_trace.endpos;
        data.penetrate_count--;
    }

    return false;
}

// TraceToExit Ч ищем выход из стены
bool CAutowall::TraceToExit(trace_t& enter_trace, trace_t& exit_trace, Vector start, Vector dir, float max_distance)
{
    float distance = 0.f;
    int first_contents = 0;

    while (distance < max_distance)
    {
        distance += 4.f;
        Vector end = start + dir * distance;

        Ray_t ray;
        ray.Init(start, end);

        CTraceFilter filter;
        filter.pSkip = LocalPlayer;

        Interfaces::EngineTrace->TraceRay(ray, MASK_SHOT_HULL, &filter, &exit_trace);

        if (!first_contents)
            first_contents = exit_trace.contents;

        if (exit_trace.contents & CONTENTS_SOLID && !(exit_trace.contents & first_contents))
            continue;

        if (exit_trace.fraction == 1.f)
            return false;

        if (exit_trace.DidHit() && exit_trace.m_pEnt)
        {
            if (enter_trace.DidHitNonWorldEntity() && exit_trace.DidHitNonWorldEntity() &&
                enter_trace.m_pEnt == exit_trace.m_pEnt)
                return true;
        }

        if (exit_trace.surface.flags & SURF_NODRAW || exit_trace.surface.flags & SURF_SKY)
            return false;
    }

    return false;
}

// ћодификатор урона по материалу (упрощЄнно)
float CAutowall::GetDamageModifier(int surface_material)
{
    switch (surface_material)
    {
    case CHAR_TEX_METAL:
    case CHAR_TEX_GRATE:        return 0.99f;
    case CHAR_TEX_CONCRETE:     return 0.70f;
    case CHAR_TEX_WOOD:         return 0.80f;
    case CHAR_TEX_FLESH:        return 0.90f;
    default:                    return 1.0f;
    }
}

// «аглушка Ч масштабирование урона по hitgroup (head 4x и т.д.)
void ScaleDamage(C_BasePlayer* player, int hitgroup, CSWeaponInfo* weapon_data, float& damage)
{
    bool has_helmet = player->HasHelmet();
    int armor = player->GetArmorValue();

    switch (hitgroup)
    {
    case HITGROUP_HEAD:
        damage *= has_helmet ? 2.0f : 4.0f;
        break;
    case HITGROUP_CHEST:
    case HITGROUP_STOMACH:
        damage *= 1.0f;
        break;
    case HITGROUP_LEFTLEG:
    case HITGROUP_RIGHTLEG:
        damage *= 0.75f;
        break;
    default:
        break;
    }

    if (armor > 0)
    {
        if (hitgroup == HITGROUP_HEAD && has_helmet)
        {
            float ratio = weapon_data->m_flArmorRatio * 0.5f;
            damage *= ratio;
        }
        else
        {
            damage *= weapon_data->m_flArmorRatio;
        }
    }
}