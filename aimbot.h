#ifndef __AIMBOT_H
#define __AIMBOT_H

#include <chrono>

#include <windows.h>

#include "SDK.hpp"
#include "config.h"
#include "util.h"

#include <time.h>  

class Aimbot
{
    public:
    float m_LastAimDistance = 0.0f;

    void Run(Config& cfg, SDK::APlayerController* playerController)
    {
		if (!cfg.m_EnableAimbot)
		{
			return;
		}

        auto now = m_Timer.now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastTime);
        m_LastTime = now;

        if (playerController == nullptr || playerController->AcknowledgedPawn == nullptr)
        {
            return;
        }

		if (GetAsyncKeyState(cfg.m_AimbotHotkey) & 0x8000 && !(GetAsyncKeyState(cfg.m_IgnoreHotkey) & 0x8000)) // Ignore Aimbot when Mouse5 pressed
		{
            if (m_TargetPlayer == nullptr)
            {
                m_TargetPlayer = Util::GetClosestVisiblePlayer(cfg, playerController);
            }
        }
        else
        {
            m_TargetPlayer = nullptr;
        }

        if (m_TargetPlayer != nullptr)
        {
            auto pawn = static_cast<SDK::AFortPawn*>(m_TargetPlayer);
            if (pawn->bIsDBNO)
            {
                m_TargetPlayer = nullptr;
                return;
            }

            SDK::FVector playerLoc;
            Util::Engine::GetBoneLocation(static_cast<SDK::ACharacter*>(m_TargetPlayer)->Mesh, &playerLoc, eBone::BONE_CHEST);

            auto dist = Util::GetDistance(playerController->AcknowledgedPawn->RootComponent->Location, playerLoc);
            m_LastAimDistance = dist;

            if (dist < cfg.m_MinAimbotHeadshotDistance)
            {
                Util::Engine::GetBoneLocation(static_cast<SDK::ACharacter*>(m_TargetPlayer)->Mesh, &playerLoc, eBone::BONE_HEAD);

				// Uncomment lower part if you want aimspot randomization
				// Untested, probably switches like mad
				// Also the randomization seed should be changed after enemy change or sth else
				/*
				switch (rand() % 4 + 1)
				{
				case 1:
					Util::Engine::GetBoneLocation(static_cast<SDK::ACharacter*>(m_TargetPlayer)->Mesh, &playerLoc, eBone::BONE_HEAD);
				case 2:
					Util::Engine::GetBoneLocation(static_cast<SDK::ACharacter*>(m_TargetPlayer)->Mesh, &playerLoc, eBone::BONE_CHEST_TOP_1);
				case 3:
					Util::Engine::GetBoneLocation(static_cast<SDK::ACharacter*>(m_TargetPlayer)->Mesh, &playerLoc, eBone::BONE_CHEST);
				case 4:
					Util::Engine::GetBoneLocation(static_cast<SDK::ACharacter*>(m_TargetPlayer)->Mesh, &playerLoc, eBone::BONE_CHEST_TOP_2);
				}*/
            }

            LookAt(cfg, playerController, playerLoc, deltaTime.count() / 1000.0f);
        }
    }

    void LookAt(Config& cfg, SDK::APlayerController* m_Player, SDK::FVector position, float deltaTime)
    {
        using namespace Util;

		int screenSizeX, screenSizeY;
		m_Player->GetViewportSize(&screenSizeX, &screenSizeY);
		SDK::FVector2D centerScreen{ (float)screenSizeX / 2, (float)screenSizeY / 2 };
		SDK::FVector2D screenPos;
		if (Engine::WorldToScreen(m_Player, position, &screenPos))
		{
			auto enemyDir = screenPos - centerScreen;		
			//m_Agressiveness += (Len(enemyDir) / cfg.m_AimbotFieldOfViewPixels) * deltaTime;
			//m_AimVelocity = m_AimVelocity + enemyDir * cfg.m_AimbotSmoothing * m_Agressiveness * deltaTime;
			//Util::MoveMouse(m_AimVelocity.X, m_AimVelocity.Y);
			mouse_event(MOUSEEVENTF_MOVE, floor(enemyDir.X / 2.7f), floor(enemyDir.Y / 2.7f), NULL, NULL);
		}
		m_AimVelocity = m_AimVelocity * cfg.m_AimVelocityDamping * deltaTime;
		m_Agressiveness *= 0.95f;
		if (m_Agressiveness < 1.0f)
		{
			m_Agressiveness = 1.0f;
		}
    }

    SDK::AActor* GetTargetPlayer() const
    {
        return m_TargetPlayer;
    }

    private:
    SDK::FVector2D m_AimVelocity{ 0.0f, 0.0f };
    float m_Agressiveness = 1.0f;

    SDK::AActor* m_TargetPlayer = nullptr;
    std::chrono::high_resolution_clock m_Timer;
    std::chrono::high_resolution_clock::time_point m_LastTime;
};

#endif
