

#include "Misc.hpp"

namespace WalkerProcessor {
    
    namespace HooksPathing {

        struct Pathing {

            static void mySpellCast(RE::ActorMagicCaster* this_)
            {

                if (this_->currentSpell)
                {
                    if (this_->currentSpell->formID == MiscThings::get_shifted_id_for_mod(2, 0x1aa86e))
                    {
                        if (this_->state == RE::MagicCaster::State::kCasting)
                        {
                            auto player = RE::PlayerCharacter::GetSingleton();

                            auto item = (RE::TESBoundObject*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1af975));

                            if (item)
                            {
                                std::vector<std::pair<RE::ObjectRefHandle, float>> cookies_to_control{};

                                auto camera = RE::PlayerCamera::GetSingleton();

                                if (camera)
                                {
                                    auto camera_dirY = camera->cameraRoot->world.rotate.GetVectorY(); //lookat
                                    auto camera_dirX = camera->cameraRoot->world.rotate.GetVectorX(); //right

                                    RE::NiPoint3 pos_to_place = player->GetPosition();

                                    pos_to_place.z += 120.0f;

                                    pos_to_place += camera_dirY * 50.0f;


                                    for (int i = 0; i < 10; i++)
                                    {
                                        RE::NiPoint3 shift = MiscThings::rotate_around_axis(camera_dirX * 40.0f, camera_dirY, -2.0f*RE::NI_PI / 9.0f * i);

                                        auto new_item = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(item, pos_to_place + shift, { (float)std::rand() / RAND_MAX * 10.0f, (float)std::rand() / RAND_MAX * 10.0f, (float)std::rand() / RAND_MAX * 10.0f }, player->GetParentCell(), player->GetWorldspace(), nullptr, nullptr, player->GetHandle(), false, true);

                                        cookies_to_control.push_back({ new_item, -2.0f * RE::NI_PI / 9.0f * i });
                                    }
                                }

                                MiscThings::add_cookies_to_controll(cookies_to_control, true);

                            }
                            bool stop_here = false;
                        }
                    }

                    if (this_->currentSpell->formID == MiscThings::get_shifted_id_for_mod(2, 0x1b4a7d))
                    {
                        if (this_->state == RE::MagicCaster::State::kCasting || this_->state == RE::MagicCaster::State::kUnk09)
                        {
                            MiscThings::explode_cookies_nearby();
                        }
                        else
                            bool stop_here = false;
                    }


                    if (this_->currentSpell->formID == MiscThings::get_shifted_id_for_mod(2, 0x1af977)) //cookie2
                    {
                        if (this_->state == RE::MagicCaster::State::kCasting)
                        {
                            auto player = RE::PlayerCharacter::GetSingleton();

                            auto item = (RE::TESBoundObject*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1af975));

                            if (item)
                            {
                                auto camera = RE::PlayerCamera::GetSingleton();

                                if (camera)
                                {
                                    auto camera_dirY = camera->cameraRoot->world.rotate.GetVectorY(); //lookat
                                    auto camera_dirX = camera->cameraRoot->world.rotate.GetVectorX(); //right

                                    RE::NiPoint3 pos_to_place = player->GetPosition();

                                    pos_to_place.z += 140.0f;

                                    pos_to_place += camera_dirY * 50.0f;

                                    for (int i = 0; i < 10; i++)
                                    {
                                        RE::NiPoint3 shift = MiscThings::rotate_around_axis(camera_dirX * (float)std::rand() / RAND_MAX * 30.0f, camera_dirY, (float)std::rand() / RAND_MAX * 2.0f * RE::NI_PI);

                                        auto new_item = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(item, pos_to_place + shift, { (float)std::rand() / RAND_MAX * 10.0f, (float)std::rand() / RAND_MAX * 10.0f, (float)std::rand() / RAND_MAX * 10.0f }, player->GetParentCell(), player->GetWorldspace(), nullptr, nullptr, player->GetHandle(), false, true);

                                    }
                                }


                            }
                            bool stop_here = false;
                        }
                    }

                    if (this_->currentSpell->formID == MiscThings::get_shifted_id_for_mod(2, 0x1bec80)) //cookie3
                    {
                        if (this_->state == RE::MagicCaster::State::kCasting)
                        {
                            auto player = RE::PlayerCharacter::GetSingleton();
                            auto item = (RE::TESBoundObject*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1af975));

                            RE::NiPoint3 pos_to_place = player->GetPosition();

                            RE::TESObjectREFR* target_to_place = player;
                  
                            auto camera = RE::PlayerCamera::GetSingleton();
                            auto camera_dirY = camera->cameraRoot->world.rotate.GetVectorY();

                            auto filter = MiscThings::get_player_projectile_filter();
                            float raycast_distance = MiscThings::GetRaycastDistance(camera->cameraRoot->world.translate, camera_dirY, 500000.0f, player, filter);

                            pos_to_place += camera_dirY * raycast_distance;



                            if (item)
                            {
                                pos_to_place.z += 3000.0f;
                                pos_to_place.x += 1500.0f;

                                auto new_item = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(item, pos_to_place, { (float)std::rand() / RAND_MAX * 10.0f, (float)std::rand() / RAND_MAX * 10.0f, (float)std::rand() / RAND_MAX * 10.0f }, target_to_place->GetParentCell(), target_to_place->GetWorldspace(), nullptr, nullptr, target_to_place->GetHandle(), false, true);
                                new_item.get().get()->SetScale(200.0f);
                                MiscThings::remember_big_cookie(new_item);
                            }
                        }
                    }
                    
                    
                }
                    
                return originalSpellCast(this_);
                
            }

            static inline REL::Relocation<decltype(mySpellCast)> originalSpellCast;

            static inline void Install() { 
                originalSpellCast = REL::Relocation<uintptr_t>(RE::VTABLE_ActorMagicCaster[0]).write_vfunc(0x7, mySpellCast);
            }
        };
    }

    void install_hook()
    {
        HooksPathing::Pathing::Install();
    }


}
