#ifndef __ESP_H
#define __ESP_H

#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include "config.h"
#include "global.h"
#include "util.h"
#include "renderer.h"


class ESP
{	
    public:	
		bool cached = false;
		void Draw(Config& cfg, Renderer& renderer)
		{
			if (!cfg.m_EnableESP)
			{
				return;
			}

			if (Global::m_PersistentLevel == nullptr || Global::m_LocalPlayer == nullptr)
			{
				return;
			}

			if (Global::m_LocalPlayer == nullptr || Global::m_LocalPlayer->PlayerController == nullptr)
			{
				return;
			}

			auto playerController = Global::m_LocalPlayer->PlayerController;
			if (playerController->AcknowledgedPawn == nullptr)
			{
				return;
			}

			auto localPos = playerController->AcknowledgedPawn->RootComponent->Location;

			auto itemsDrawn = 0u;

			auto actors = Global::m_PersistentLevel->AActors;
			for (int i = 0; i < actors.Num(); i++)
			{
				if (!actors.IsValidIndex(i))
				{
					continue;
				}

				auto actor = Global::m_PersistentLevel->AActors[i];
				if (actor == nullptr || actor->RootComponent == nullptr)
				{
					continue;
				}

				if (actor->IsA(SDK::AFortPawn::StaticClass()))
				{
					auto pawn = static_cast<SDK::AFortPawn*>(actor);
					if (pawn->GetName().find("PlayerPawn_Athena_C") == std::string::npos)
					{
						continue;
					}

					if (Util::IsTeammate(actor) || Util::IsLocalPlayer(actor))
					{
						continue;
					}

					if (pawn->PlayerState == nullptr || !pawn->PlayerState->IsA(SDK::AFortPlayerStateAthena::StaticClass()))
					{
						continue;
					}

					auto state = static_cast<SDK::AFortPlayerStateAthena*>(pawn->PlayerState);
					if (!state->PlayerName.IsValid() || pawn->RootComponent == nullptr)
					{
						continue;
					}

					SDK::FVector playerLoc;
					SDK::FVector headLoc;
					SDK::FVector footLoc;

					Util::Engine::GetBoneLocation(pawn->Mesh, &playerLoc, 66);
					Util::Engine::GetBoneLocation(pawn->Mesh, &headLoc, 66);
					Util::Engine::GetBoneLocation(pawn->Mesh, &footLoc, 0);
					headLoc.Z += 15;

					SDK::FVector2D screenPos;
					SDK::FVector2D iPos;
					SDK::FVector2D hPos;
					if (!Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, playerLoc, &screenPos) || !Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, footLoc, &iPos) || !Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, headLoc, &hPos))
					{
						continue;
					}

					auto name = state->PlayerName.c_str();
					auto distance = Util::GetDistance(localPos, pawn->RootComponent->Location);
					auto curWeapon = pawn->CurrentWeapon;
					auto health = static_cast<int>(state->CurrentHealth);
					auto shield = static_cast<int>(state->CurrentShield);
					float height = abs(hPos.Y - iPos.Y);
					float width = height * 0.65f;

					float hpheight = (height / 100) * health;
					float sheight = (height / 100) * shield;
					float bwidth = width * 0.05f;
					if (bwidth < 4.f)
						bwidth = 4.f;

					std::wstringstream ss;
					std::wstringstream info;
					Color BoxColor = cfg.m_EnemyTextColor;

					if (curWeapon && curWeapon->WeaponData)
					{
						auto itemDef = static_cast<SDK::UFortWorldItemDefinition*>(curWeapon->WeaponData);

						switch (itemDef->Tier.GetValue())
						{
						case SDK::EFortItemTier::I: // white
							break;
						case SDK::EFortItemTier::II: // green
							BoxColor = Color{ 0.0f, 0.95f, 0.0f, 0.95f };
							break;
						case SDK::EFortItemTier::III: // blue
							BoxColor = Color{ 0.4f, 0.65f, 1.0f, 0.95f };
							break;
						case SDK::EFortItemTier::IV: // purple
							BoxColor = Color{ 0.7f, 0.25f, 0.85f, 0.95f };
							break;
						case SDK::EFortItemTier::V: // orange
							BoxColor = Color{ 0.85f, 0.65f, 0.0f, 0.95f };
							break;
						case SDK::EFortItemTier::VI: // gold
						case SDK::EFortItemTier::VII:
							BoxColor = Color{ 0.95f, 0.85f, 0.45f, 0.95f };
							break;
						case SDK::EFortItemTier::VIII:
						case SDK::EFortItemTier::IX:
						case SDK::EFortItemTier::X:
							BoxColor = Color{ 1.0f, 0.0f, 1.0f, 0.95f };
							break;
						}					

						if (itemDef->ItemType == SDK::EFortItemType::WeaponRanged || itemDef->ItemType == SDK::EFortItemType::WeaponMelee || itemDef->ItemType == SDK::EFortItemType::WeaponHarvest)
							ss << name << L" [" << Util::DistanceToString(distance) << L"] " << itemDef->DisplayName.Get();
						else
							ss << name << L" [" << Util::DistanceToString(distance) << L"]";

						if(distance > 10000.f)
							info << health << L" HP | " << shield << L" S";

						else if (itemDef->ItemType == SDK::EFortItemType::WeaponRanged)
							info << curWeapon->AmmoCount << L" Ammo";
					}

					else if (pawn->bIsDBNO)
					{
						ss << name << L" [" << Util::DistanceToString(distance) << L"]";
						info << health << L" HP | " << L"Is Dying";
					}

					else
					{
						ss << name << L" [" << Util::DistanceToString(distance) << L"]";
						info << health << L" HP";
					}					

					//  Draw Skeleton here so we always set back to solid mode
					Color skel_color{ 1.f, 1.f ,1.f ,0.95f };

					SDK::FVector Skel_Head, Skel_Chest, Skel_Shoulder_L, Skel_Shoulder_R, Skel_Elbow_L, Skel_Elbow_R, Skel_Hand_L, Skel_Hand_R, Skel_Pelvis, Skel_Leg_L, Skel_Leg_R, Skel_Knee_L, Skel_Knee_R, Skel_Foot_L, Skel_Foot_R;

					SDK::FVector2D Skel_Head2D, Skel_Chest2D, Skel_Shoulder_L2D, Skel_Shoulder_R2D, Skel_Elbow_L2D, Skel_Elbow_R2D, Skel_Hand_L2D, Skel_Hand_R2D, Skel_Pelvis2D, Skel_Leg_L2D, Skel_Leg_R2D, Skel_Knee_L2D, Skel_Knee_R2D, Skel_Foot_L2D, Skel_Foot_R2D;

					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Head, eBone::BONE_HEAD);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Chest, eBone::BONE_CHEST);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Shoulder_L, eBone::BONE_L_SHOULDER_1);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Shoulder_R, eBone::BONE_R_SHOULDER);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Elbow_L, eBone::BONE_L_ELBOW);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Elbow_R, eBone::BONE_R_ELBOW);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Hand_L, eBone::BONE_L_HAND_ROOT_1);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Hand_R, eBone::BONE_R_HAND_ROOT_1);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Pelvis, eBone::BONE_PELVIS_1);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Leg_L, eBone::BONE_L_LEG_ROOT);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Leg_R, eBone::BONE_R_LEG_ROOT);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Knee_L, eBone::BONE_L_KNEE);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Knee_R, eBone::BONE_R_KNEE);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Foot_L, eBone::BONE_L_FOOT_ROOT);
					Util::Engine::GetBoneLocation(pawn->Mesh, &Skel_Foot_R, eBone::BONE_R_FOOT_ROOT);

					if (Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Head, &Skel_Head2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Chest, &Skel_Chest2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Shoulder_L, &Skel_Shoulder_L2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Shoulder_R, &Skel_Shoulder_R2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Elbow_L, &Skel_Elbow_L2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Elbow_R, &Skel_Elbow_R2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Hand_L, &Skel_Hand_L2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Hand_R, &Skel_Hand_R2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Pelvis, &Skel_Pelvis2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Leg_L, &Skel_Leg_L2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Leg_R, &Skel_Leg_R2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Knee_L, &Skel_Knee_L2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Knee_R, &Skel_Knee_R2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Foot_L, &Skel_Foot_L2D) &&
						Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, Skel_Foot_R, &Skel_Foot_R2D))
					{
						renderer.drawLine(Vec2(Skel_Head2D.X, Skel_Head2D.Y), Vec2(Skel_Chest2D.X, Skel_Chest2D.Y), skel_color);

						renderer.drawLine(Vec2(Skel_Chest2D.X, Skel_Chest2D.Y), Vec2(Skel_Shoulder_L2D.X, Skel_Shoulder_L2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Shoulder_L2D.X, Skel_Shoulder_L2D.Y), Vec2(Skel_Elbow_L2D.X, Skel_Elbow_L2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Elbow_L2D.X, Skel_Elbow_L2D.Y), Vec2(Skel_Hand_L2D.X, Skel_Hand_L2D.Y), skel_color);

						renderer.drawLine(Vec2(Skel_Chest2D.X, Skel_Chest2D.Y), Vec2(Skel_Shoulder_R2D.X, Skel_Shoulder_R2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Shoulder_R2D.X, Skel_Shoulder_R2D.Y), Vec2(Skel_Elbow_R2D.X, Skel_Elbow_R2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Elbow_R2D.X, Skel_Elbow_R2D.Y), Vec2(Skel_Hand_R2D.X, Skel_Hand_R2D.Y), skel_color);

						renderer.drawLine(Vec2(Skel_Chest2D.X, Skel_Chest2D.Y), Vec2(Skel_Pelvis2D.X, Skel_Pelvis2D.Y), skel_color);

						renderer.drawLine(Vec2(Skel_Pelvis2D.X, Skel_Pelvis2D.Y), Vec2(Skel_Leg_L2D.X, Skel_Leg_L2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Leg_L2D.X, Skel_Leg_L2D.Y), Vec2(Skel_Knee_L2D.X, Skel_Knee_L2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Knee_L2D.X, Skel_Knee_L2D.Y), Vec2(Skel_Foot_L2D.X, Skel_Foot_L2D.Y), skel_color);

						renderer.drawLine(Vec2(Skel_Pelvis2D.X, Skel_Pelvis2D.Y), Vec2(Skel_Leg_R2D.X, Skel_Leg_R2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Leg_R2D.X, Skel_Leg_R2D.Y), Vec2(Skel_Knee_R2D.X, Skel_Knee_R2D.Y), skel_color);
						renderer.drawLine(Vec2(Skel_Knee_R2D.X, Skel_Knee_R2D.Y), Vec2(Skel_Foot_R2D.X, Skel_Foot_R2D.Y), skel_color);
					}

					auto size = renderer.getTextExtent(ss.str(), cfg.m_TextSize, cfg.m_DefaultFont);
					auto isize = renderer.getTextExtent(info.str(), cfg.m_TextSize, cfg.m_DefaultFont);

					renderer.drawText(Vec2(screenPos.X - size.x * 0.5f, screenPos.Y - size.y - 16.0f), ss.str(), BoxColor, 0, cfg.m_TextSize, cfg.m_DefaultFont);
					renderer.drawText(Vec2(iPos.X - isize.x * 0.5f, iPos.Y - isize.y + 16.0f), info.str(), cfg.m_InfoTextColor, 0, cfg.m_TextSize, cfg.m_DefaultFont);
					
					renderer.drawOutlinedRect(Vec4(hPos.X - (width / 2), hPos.Y, width, height), 1.f, BoxColor, Color{ 0.f , 0.f, 0.f, 0.2f });
					if (!pawn->bIsDBNO)
					{
						if(health > 0 && distance < 10000.f)
						renderer.drawOutlinedRect(Vec4(hPos.X - width * 0.8f, hPos.Y + height, bwidth, -1*hpheight), 1.f, Color{ 0.f , 0.f, 0.f, 0.7f }, Color{ 0.f, 0.8f, 0.f, 0.95f });
						if(shield > 0 && distance < 10000.f)
						renderer.drawOutlinedRect(Vec4(hPos.X + width * 0.8f, hPos.Y + height, bwidth, -1*sheight), 1.f, Color{ 0.f , 0.f, 0.f, 0.7f }, Color{ 0.f, 0.f, 0.8f, 0.95f });
					}
					itemsDrawn++;
				}
				else if (actor->IsA(SDK::ABuildingTrap::StaticClass()))
				{
					if (itemsDrawn >= cfg.m_MaxESPLabelsCount)
					{
						continue;
					}

					SDK::FVector2D screenPos;

					if (!Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, actor->RootComponent->Location, &screenPos))
					{
						continue;
					}

					auto trap = static_cast<SDK::ABuildingTrap*>(actor);

					auto distance = Util::GetDistance(localPos, actor->RootComponent->Location);
					if (distance > cfg.m_MaxESPRange)
					{
						continue;
					}

					if (!trap->TrapData)
					{
						continue;
					}

					auto trapdata = trap->TrapData;

					std::wstringstream ss;
					ss << trapdata->DisplayName.Get() << L" [" << Util::DistanceToString(distance) << L"]"; // Todo: Find name of Trap

					auto size = renderer.getTextExtent(ss.str(), cfg.m_TextSize, cfg.m_DefaultFont);
					renderer.drawText(Vec2(screenPos.X - size.x, screenPos.Y - size.y), ss.str(), Color{ 1.f, 0.f, 0.f, 0.95f }, 0, cfg.m_TextSize, cfg.m_DefaultFont);
					itemsDrawn++;
				}
				else if (actor->IsA(SDK::AB_Pickups_C::StaticClass()))
				{
					if (itemsDrawn >= cfg.m_MaxESPLabelsCount || GetAsyncKeyState(cfg.m_IgnoreHotkey) & 0x8000 || GetAsyncKeyState(cfg.m_AimbotHotkey) & 0x8000)
					{
						continue;
					}

					SDK::FVector2D screenPos;
					if (!Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, actor->RootComponent->Location, &screenPos))
					{
						continue;
					}

					auto pickup = static_cast<SDK::AB_Pickups_C*>(actor);

					if (!pickup->ItemDefinition)
					{
						continue;
					}

					auto itemDef = pickup->ItemDefinition;

					Color color{ 0.8f, 0.8f, 0.8f, 0.95f };

					if (itemDef->ItemType == SDK::EFortItemType::Ammo)
					{
						color = { 0.8f, 0.8f, 0.8f, 0.35f };
					}

					switch (itemDef->Tier.GetValue())
					{
					case SDK::EFortItemTier::I: // white
						break;
					case SDK::EFortItemTier::II: // green
						color = Color{ 0.0f, 0.95f, 0.0f, 0.95f };
						break;
					case SDK::EFortItemTier::III: // blue
						color = Color{ 0.4f, 0.65f, 1.0f, 0.95f };
						break;
					case SDK::EFortItemTier::IV: // purple
						color = Color{ 0.7f, 0.25f, 0.85f, 0.95f };
						break;
					case SDK::EFortItemTier::V: // orange
						color = Color{ 0.85f, 0.65f, 0.0f, 0.95f };
						break;
					case SDK::EFortItemTier::VI: // gold
					case SDK::EFortItemTier::VII:
						color = Color{ 0.95f, 0.85f, 0.45f, 0.95f };
						break;
					case SDK::EFortItemTier::VIII:
					case SDK::EFortItemTier::IX:
					case SDK::EFortItemTier::X:
						color = Color{ 1.0f, 0.0f, 1.0f, 0.95f };
						break;
					}

					if (!itemDef->DisplayName.Get())
					{
						continue;
					}

					std::wstring name = itemDef->DisplayName.Get();
					std::wstring::size_type found = name.find(L"Potion", found);
					if (found != std::wstring::npos)
					{
						color = Color{ 0.55f, 0.95f, 0.80f, 0.95f };
					}
					else if (name == L"Bandage" || name == L"Med Kit")
					{
						color = Color{ 0.9f, 0.55f, 0.55f, 0.95f };
					}
					else if (name == L"Shield Potion")
					{
						color = Color{ 0.35f, 0.55f, 0.85f, 0.95f };
					}

					if (pickup->RootComponent == nullptr)
					{
						continue;
					}

					auto distance = Util::GetDistance(localPos, pickup->RootComponent->Location);
					if (distance > cfg.m_MaxESPRange)
					{
						continue;
					}

					std::wstringstream ss;
					ss << name << L" [" << Util::DistanceToString(distance) << L"]";

					auto size = renderer.getTextExtent(ss.str(), cfg.m_TextSize, cfg.m_DefaultFont);
					renderer.drawText(Vec2(screenPos.X - size.x, screenPos.Y - size.y), ss.str(), color, 0, cfg.m_TextSize, cfg.m_DefaultFont);
					itemsDrawn++;
				}
				else if (actor->GetName().find("AthenaSupplyDrop_02_C") != std::string::npos)
				{
					if (itemsDrawn >= cfg.m_MaxESPLabelsCount || GetAsyncKeyState(cfg.m_IgnoreHotkey) & 0x8000 || GetAsyncKeyState(cfg.m_AimbotHotkey) & 0x8000)
					{
						continue;
					}

					SDK::FVector2D screenPos;
					if (!Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, actor->RootComponent->Location, &screenPos))
					{
						continue;
					}

					if (actor->RootComponent == nullptr)
					{
						continue;
					}

					auto distance = Util::GetDistance(localPos, actor->RootComponent->Location);
					if (distance > cfg.m_MaxESPRange)
					{
						continue;
					}

					std::wstringstream ss;
					ss << L"Supply Drop [" << Util::DistanceToString(distance) << L"]";

					auto size = renderer.getTextExtent(ss.str(), cfg.m_TextSize, cfg.m_DefaultFont);
					renderer.drawText(Vec2(screenPos.X - size.x, screenPos.Y - size.y), ss.str(), Color{ 0.4f, 0.9f, 0.4f, 0.75f }, 0, cfg.m_TextSize, cfg.m_DefaultFont);
					itemsDrawn++;
				}
			}

			if(!cached)
			{
				ChestLoc.clear();
				AmmoLoc.clear();
				cached = true;
				auto objects = Global::m_PersistentLevel->GObjects->ObjObjects;
				for (int i = 0; i < objects.Num(); i++)
				{
					auto object = objects.GetByIndex(i);

					if (object == nullptr)
					{
						continue;
					}

					if (!(object->IsA(SDK::ABuildingContainer::StaticClass())))
					{
						continue;
					}

					SDK::ABuildingContainer* BContainer = (SDK::ABuildingContainer*)object;

					if (BContainer == nullptr || BContainer->RootComponent == nullptr || BContainer->LootRepeatSoundCue == nullptr)
						continue;

					if (BContainer->LootRepeatSoundCue->GetName() != "Tiered_Chest_LootRepeat_Cue" && BContainer->LootRepeatSoundCue->GetName() != "Toolbox_SearchEnd_Cue" || BContainer->bAlreadySearched)
						continue;

					SDK::FVector BLoc = BContainer->RootComponent->K2_GetComponentLocation();

					if (BContainer->LootRepeatSoundCue->GetName() == "Tiered_Chest_LootRepeat_Cue")
					{
						ChestLoc.push_back(BLoc);
					}
					else if (BContainer->LootRepeatSoundCue->GetName() == "Toolbox_SearchEnd_Cue")
					{
						AmmoLoc.push_back(BLoc);
					}
				}
			}

			if (GetAsyncKeyState(cfg.m_IgnoreHotkey) & 0x8000 || GetAsyncKeyState(cfg.m_AimbotHotkey) & 0x8000)
			{
				return;
			}

			if(cached)
			{
				for (int i = 0; i < ChestLoc.size(); i++)
				{
					if (itemsDrawn >= cfg.m_MaxESPLabelsCount)
					{
						continue;
					}

					SDK::FVector BLoc = ChestLoc[i];

					if (BLoc.X == 0.f && BLoc.Y == 0.f && BLoc.Z == 0.f)
						continue;

					auto distance = Util::GetDistance(localPos, BLoc);

					if (distance > 30000.f)
						continue;

					SDK::FVector2D screenPos;

					if (!Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, BLoc, &screenPos))
					{
						continue;
					}

					std::wstringstream ss;

					ss << L"Chest" << L" [" << Util::DistanceToString(distance) << L"]";

					auto size = renderer.getTextExtent(ss.str(), cfg.m_TextSize, cfg.m_DefaultFont);
					renderer.drawText(Vec2(screenPos.X - size.x * 0.5f, screenPos.Y - size.y - 16.0f), ss.str(), Color{ 1.f, 1.f, 1.f, 0.95f }, 0, cfg.m_TextSize, cfg.m_DefaultFont);
				}

				for (int i = 0; i < AmmoLoc.size(); i++)
				{
					if (itemsDrawn >= cfg.m_MaxESPLabelsCount)
					{
						continue;
					}

					SDK::FVector BLoc = AmmoLoc[i];

					if (BLoc.X == 0.f && BLoc.Y == 0.f && BLoc.Z == 0.f)
						continue;

					auto distance = Util::GetDistance(localPos, BLoc);

					if (distance > 30000.f)
						continue;

					SDK::FVector2D screenPos;
					if (!Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, BLoc, &screenPos))
					{
						continue;
					}

					std::wstringstream ss;

					ss << L"AmmoBox" << L" [" << Util::DistanceToString(distance) << L"]";

					auto size = renderer.getTextExtent(ss.str(), cfg.m_TextSize, cfg.m_DefaultFont);
					renderer.drawText(Vec2(screenPos.X - size.x * 0.5f, screenPos.Y - size.y - 16.0f), ss.str(), Color{ 0.8f, 0.8f, 0.8f, 0.35f }, 0, cfg.m_TextSize, cfg.m_DefaultFont);
				}
			}
		}

    private:
		std::vector<SDK::FVector> ChestLoc;
		std::vector<SDK::FVector> AmmoLoc;
};

#endif
