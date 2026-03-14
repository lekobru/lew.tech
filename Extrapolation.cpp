#pragma once

#include "Extended_Interface.hpp"           // или где у вас C_BasePlayer, C_BaseEntity и т.д.
#include "interfaces.hpp"    // IEngineClient, IBaseClientDLL и т.п.
#include <Interfaces_Offsets.cpp>

// можно вынести в глобальные переменные чита или в cfg
namespace Extrapolation
{
    inline bool Enabled = true;
    inline float Extrapolation_Time = 0.2f;     // сколько секунд вперёд предсказывать (0.1–0.4 обычно)
    inline bool Draw_Bone_Connections = true;   // рисовать ли линии между старой и новой позицией костей

    // Структура для хранения предсказанных данных одного игрока
    struct ExtrapolatedData_t
    {
        Vector origin;
        Vector velocity;
        Vector mins, maxs;
        matrix3x4_t bone_matrix[128];  // если хотите точную экстраполяцию по костям
        float simtime;
        bool valid = false;
    };

    inline ExtrapolatedData_t g_ExtrapData[65];

    // Основная функция экстраполяции
    void ProcessPlayer(C_BasePlayer* pPlayer, ExtrapolatedData_t& out)
    {
        if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant() || !pPlayer->IsEnemy())
            return;

        const float curtime = Interfaces::EngineClient()->GetLastTimeStamp();  // или g_GlobalVars->curtime
        const float latency = Interfaces::EngineClient()->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
        const float interp = Interfaces::Cvar()->FindVar("cl_interp")->GetFloat();
        const float total_latency = latency + interp;

        // Текущая симуляция времени игрока
        float player_simtime = pPlayer->m_flSimulationTime();

        // Если игрок не обновлялся давно — пропускаем
        if (player_simtime <= 0.f || player_simtime >= curtime)
            return;

        // Сколько тиков прошло с последнего обновления
        float delta_time = curtime - player_simtime;

        // Берём velocity из предпоследнего тика (если есть доступ)
        // В CSS v34 часто velocity хранится в C_BasePlayer
        Vector velocity = pPlayer->m_vecVelocity();

        // Простая линейная экстраполяция
        Vector predicted_origin = pPlayer->GetAbsOrigin() + velocity * Extrapolation_Time;

        // Более точная — учитываем сетевой лаг
        // predicted_origin = pPlayer->GetAbsOrigin() + velocity * (Extrapolation_Time + total_latency);

        out.origin = predicted_origin;
        out.velocity = velocity;
        out.mins = pPlayer->m_vecMins();
        out.maxs = pPlayer->m_vecMaxs();
        out.simtime = player_simtime;
        out.valid = true;

        // Если хотите экстраполировать кости (лучше для headshot-aimbot / skeleton ESP)
        // Обычно делают так:
        matrix3x4_t* bone_cache = pPlayer->GetBoneCache();
        if (bone_cache)
        {
            // Копируем текущую матрицу
            memcpy(out.bone_matrix, bone_cache, sizeof(matrix3x4_t) * 128);

            // Сдвигаем каждую кость по velocity (очень грубо, но работает)
            for (int i = 0; i < 128; ++i)
            {
                out.bone_matrix[i][0][3] += velocity.x * Extrapolation_Time;
                out.bone_matrix[i][1][3] += velocity.y * Extrapolation_Time;
                out.bone_matrix[i][2][3] += velocity.z * Extrapolation_Time;
            }
        }
    }

    // Вызывать каждый кадр (в PaintTraverse / DrawModelExecute / CreateMove)
    void Run()
    {
        if (!Enabled)
            return;

        for (int i = 1; i <= 64; ++i)
        {
            C_BasePlayer* p = GetPlayer(i);
            if (!p) continue;

            ProcessPlayer(p, g_ExtrapData[i]);
        }
    }

    // Пример использования в вашем ESP / skeleton
    void DrawPlayerESP(C_BasePlayer* pPlayer)
    {
        int idx = pPlayer->EntIndex();
        if (!g_ExtrapData[idx].valid)
            return; // или рисуем обычную позицию

        Vector screen;
        if (!WorldToScreen(g_ExtrapData[idx].origin, screen))
            return;

        // Рисуем кружок / бокс / голову в предсказанной позиции
        DrawCircle(screen.x, screen.y, 8, 32, Color(255, 0, 0, 220));

        // Можно нарисовать линию от текущей позиции к предсказанной
        Vector curr_screen;
        if (WorldToScreen(pPlayer->GetAbsOrigin(), curr_screen))
        {
            DrawLine(curr_screen.x, curr_screen.y, screen.x, screen.y, Color(255, 255, 0, 180));
        }

        // Если есть кости — рисуем skeleton по g_ExtrapData[idx].bone_matrix
        // ...
    }
}