
#include "main.hpp"
#include "Misc.hpp"
#include <ctime>


namespace MiscThings {



    struct RayCastResult 
    {
        float distance = -1.0f;
        RE::COL_LAYER layer = RE::COL_LAYER::kUnidentified;
        RE::hkVector4 normalOut = RE::hkVector4(0, 0, 0, 0);
        bool didHit = false;

        // Do a null check before using this
        RE::TESObjectREFR* hitObjectRef = nullptr;

        RE::FormType GetHitObjectFormType_Safe() const {
            if (!hitObjectRef) return RE::FormType::None;
            return hitObjectRef->GetObjectReference()->GetFormType();
        }

        RayCastResult() = default;

        RayCastResult(float d, RE::COL_LAYER l, const RE::hkVector4& n, bool h, RE::TESObjectREFR* r)
            : distance(d), layer(l), normalOut(n), didHit(h), hitObjectRef(r) {
        }
    };


    uint32_t my_filter1 = 0b00000000000010010000000000011110; //player filter //kCharController = 30,
    uint32_t my_filter2 = 0b00000000000010010000000000001110; //filter without invisible zones, that doesnt work on bjorlam //kTrap = 14,
    uint32_t my_filter5 = 0b00000000000010010000000000000110;//kProjectile = 6, (may be the best?)


    uint32_t get_player_projectile_filter()
    {
        return my_filter5;
    }

    void update_player_collision_groups()
    {
        auto player = RE::PlayerCharacter::GetSingleton();
        RE::CFilter cFilter_info{};
        player->GetCollisionFilterInfo(cFilter_info);

        auto new_group = cFilter_info.filter & 0xFFFF0000;

        //remove old group
        my_filter1 = my_filter1 & 0x0000FFFF;
        my_filter2 = my_filter2 & 0x0000FFFF;
        my_filter5 = my_filter5 & 0x0000FFFF;

        //add new group
        my_filter1 += new_group;
        my_filter2 += new_group;
        my_filter5 += new_group;
    }





    RayCastResult RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist,
        RE::Actor* actor, RE::TESObjectREFR* target) {
        RayCastResult result{};
        result.distance = maxDist;

        if (!actor) {
            return result;
        }
        const auto& cell = actor->GetParentCell();
        if (!cell) {
            return result;
        }
        const auto& bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld) {
            return result;
        }

        RE::bhkPickData pickData;
        const auto& havokWorldScale = RE::bhkWorld::GetWorldScale();

        // Set ray start and end points (scaled to Havok world)
        pickData.rayInput.from = rayStart * havokWorldScale;
        rayDir.Unitize();
        pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;



        bool linked = false;

        if (target && target->IsActor())
        {
            auto target_actor = (RE::Actor*)target;
            RE::CFilter cFilter_info{};
            target_actor->GetCollisionFilterInfo(cFilter_info);

            if (cFilter_info.filter & (1 << 15))
                linked = true;
        }

        auto player = RE::PlayerCharacter::GetSingleton();

        if (linked)
            pickData.rayInput.filterInfo = (RE::CFilter)my_filter1; //idk why but for sitting actors filter2 doesnt work
        else
            pickData.rayInput.filterInfo = (RE::CFilter)my_filter2;


        // Perform the raycast
        if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
            result.didHit = true;
            result.didHit = true;
            result.distance = maxDist * pickData.rayOutput.hitFraction;
            result.normalOut = pickData.rayOutput.normal;

            result.layer = pickData.rayOutput.rootCollidable->GetCollisionLayer();

            result.hitObjectRef = RE::TESHavokUtilities::FindCollidableRef(*pickData.rayOutput.rootCollidable);
        }


        return result;
    }


    RayCastResult RayCast_debug(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist,
        RE::Actor* actor, RE::TESObjectREFR* target, uint32_t filter) {
        RayCastResult result{};
        result.distance = maxDist;

        if (!actor) {
            return result;
        }
        const auto& cell = actor->GetParentCell();
        if (!cell) {
            return result;
        }
        const auto& bhkWorld = cell->GetbhkWorld();
        if (!bhkWorld) {
            return result;
        }

        RE::bhkPickData pickData;
        const auto& havokWorldScale = RE::bhkWorld::GetWorldScale();

        // Set ray start and end points (scaled to Havok world)
        pickData.rayInput.from = rayStart * havokWorldScale;
        rayDir.Unitize();
        pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;



        bool linked = false;

        if (target && target->IsActor())
        {
            auto target_actor = (RE::Actor*)target;
            RE::CFilter cFilter_info{};
            target_actor->GetCollisionFilterInfo(cFilter_info);

            if (cFilter_info.filter & (1 << 15))
                linked = true;
        }

        auto player = RE::PlayerCharacter::GetSingleton();

        pickData.rayInput.filterInfo = (RE::CFilter)filter;

        // static_cast<RE::CFilter>(cFilter.filter | static_cast<uint32_t>(layerMask));

        // Perform the raycast
        if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
            result.didHit = true;
            result.didHit = true;
            result.distance = maxDist * pickData.rayOutput.hitFraction;
            result.normalOut = pickData.rayOutput.normal;

            result.layer = pickData.rayOutput.rootCollidable->GetCollisionLayer();

            result.hitObjectRef = RE::TESHavokUtilities::FindCollidableRef(*pickData.rayOutput.rootCollidable);
        }


        return result;
    }


    float GetRaycastDistance(RE::NiPoint3 from, RE::NiPoint3 aimVector, float distance, RE::TESObjectREFR* target, uint32_t filter)
    {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto player_actor = (RE::Actor*)player->AsReference();

        MiscThings::RayCastResult raycast_result{};

        if (filter)
            raycast_result = RayCast_debug(from, aimVector, distance, player_actor, target, filter);
        else
            raycast_result = RayCast(from, aimVector, distance, player_actor, target);


        return raycast_result.distance;
    }


















    RE::TESForm* get_hand_contents(bool right)
    {
        RE::TESForm* hand_contents = nullptr;

        auto player = RE::PlayerCharacter::GetSingleton();

        if (!player || !player->currentProcess)
            return nullptr;

        auto actor_process = player->currentProcess;
        auto equipped_list = actor_process->equippedObjects;

        auto equipped_right = equipped_list[1];
        auto equipped_left = equipped_list[0];

        if (right)
        {
            hand_contents = equipped_right;
        }
        else
        {
            hand_contents = equipped_left;
        }

        return hand_contents;
    }


    int get_shifted_id_for_mod(int mod, uint32_t base_id)
    {
        switch (mod)
        {
        case (1): //ghosu
        {
            auto mod = RE::TESDataHandler::GetSingleton()->LookupModByName("Ghosu - Project Flintlock.esp");
            if (mod)
            {
                return (((uint32_t)mod->GetCompileIndex()) << (6*4)) + base_id;
            }
            break;
        }

        case (2): //cookies
        {
            auto mod = RE::TESDataHandler::GetSingleton()->LookupModByName("cookie_spells.esp");
            if (mod)
            {
                return (((uint32_t)mod->GetCompileIndex()) << (6 * 4)) + base_id;
            }
            break;
        }
        }

        return 0;
    }


    int random_int_from_range(int min, int max)
    {
        if (min > max)
            return 0;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(min, max);
        int random_num = distr(gen);

        return random_num;
    }


    RE::NiPoint3 rotate_around_axis(RE::NiPoint3 v, RE::NiPoint3 axis, float radians)
    {
        float pi = RE::NI_PI;

        float cosTheta = std::cos(radians);
        float sinTheta = std::sin(radians);

        // Vector rotation formula components
        RE::NiPoint3 term1 = v * cosTheta;
        RE::NiPoint3 term2 = axis.Cross(v) * sinTheta;
        RE::NiPoint3 term3 = axis * axis.Dot(v) * (1.0 - cosTheta);

        return term1 + term2 + term3;
    }




    std::vector<std::pair<RE::ObjectRefHandle, float>> cookies_to_control{};

    RE::ObjectRefHandle big_cookie{};
    float cookie_timer3 = 0.0f;

    void remember_big_cookie(RE::ObjectRefHandle cookie)
    {
        big_cookie = cookie;
        cookie_timer3 = 0.0f;
    }

    void add_cookies_to_controll(std::vector<std::pair<RE::ObjectRefHandle, float>> cookies, bool stick_mode)
    {
        cookies_to_control = cookies;
    }

    float cookie_timer = 0.0f;
    float cookie_rotate_timer = 0.0f;

    float rotation_koef = 1.0f;
    bool rotation_dir = false;

    float cookie_timer2 = 0.0f;

    

    RE::ObjectRefHandle target_to_throw{};

    void reset_cookie_control()
    {
        cookie_rotate_timer = 0.0f;
        cookie_timer = 0.0f;

        rotation_koef = (float)std::rand() / RAND_MAX * 5.0f;
        rotation_dir = MiscThings::random_int_from_range(0, 1);

        target_to_throw.reset();
        cookies_to_control.clear();
        
    }


    std::vector<RE::ObjectRefHandle> cookies_to_explode{};


    void reset_cookie_control2()
    {
        cookies_to_explode.clear();
        cookie_timer2 = 0.0f;
    }

    void reset_cookie_control3()
    {
        big_cookie.reset();
        cookie_timer3 = 0.0f;
    }

    void explode_cookies_nearby()
    {
        auto player = RE::PlayerCharacter::GetSingleton();

        if (player)
        {
            RE::TES::GetSingleton()->ForEachReferenceInRange(player, 3000.0f,
                [&](RE::TESObjectREFR* a_ref) {

                    if (a_ref)
                    {
                        auto base_obj = a_ref->GetBaseObject();
                        if (base_obj && base_obj->formID == MiscThings::get_shifted_id_for_mod(2, 0x1af975))
                        {
                            //a_ref->PlaceObjectAtMe(explosion, false);
                            if (!a_ref->IsDisabled() && a_ref->GetScale() < 100.0f)
                                cookies_to_explode.push_back(a_ref->GetHandle());
                        }
                    }
                    return RE::BSContainer::ForEachResult::kContinue;
                });
        }
    }


    bool reset_player_hp = false;
    float player_hp_old = 0.0f;


    void do_reset_player_hp(float hp)
    {
        player_hp_old = hp;
        reset_player_hp = true;
    }

    void cookie_controller(float dtime)
    {
        auto camera = RE::PlayerCamera::GetSingleton();
        auto player = RE::PlayerCharacter::GetSingleton();
        auto player_pos = player->GetPosition();

        static auto explosion = (RE::BGSExplosion*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1b4a7c));

        if (reset_player_hp)
        {
            if (player_hp_old > 0.0f)
            {
                player->RestoreActorValue(RE::ActorValue::kHealth, 1000.0f);
                player->SetBaseActorValue(RE::ActorValue::kHealth, player_hp_old);

            }

            reset_player_hp = false;
            player_hp_old = 0.0f;
        }


        if (explosion)
        {
            if (std::size(cookies_to_explode) > 0)
            {
                if (cookie_timer2 <= 0.0f)
                {
                    cookie_timer2 = (float)std::rand() / RAND_MAX * 0.3f + 0.1f;

                    auto last_cookie = cookies_to_explode.at(std::size(cookies_to_explode) - 1);
                    if (last_cookie && last_cookie.get() && last_cookie.get().get())
                    {
                        last_cookie.get().get()->PlaceObjectAtMe(explosion, false);
                        cookies_to_explode.pop_back();
                        last_cookie.get().get()->SetDelete(true);
                        last_cookie.get().get()->Disable();
                    }
                    else
                    {
                        cookies_to_explode.pop_back();
                    }
                }
                else
                    cookie_timer2 -= dtime;
            }
            else
                reset_cookie_control2();
        }



        if (std::size(cookies_to_control) > 0)
        {
            if (cookie_timer >= 0.5f)
            {
                {
                    if (cookie_timer - 0.5f > 0.2f)
                    {
                        cookie_timer = 0.5f;

                        if (camera->cameraRoot && cookies_to_control.at(std::size(cookies_to_control) - 1).first)
                        {
                            auto cookie_ref = cookies_to_control.at(std::size(cookies_to_control) - 1).first.get().get();
                            if (cookie_ref)
                            {
                                auto cookie_3d = cookie_ref->Get3D();
                                if (cookie_3d && cookie_3d->collisionObject)
                                {
                                    auto body = cookie_3d->GetCollisionObject()->GetRigidBody();


                                    auto camera_dir = camera->cameraRoot->world.rotate.GetVectorY();

                                    RE::NiPoint3 throw_dir = camera_dir;

                                    throw_dir.Unitize();

                                    if (body)
                                    {
                                        throw_dir *= 1000.0f;
                                        body->SetLinearImpulse({ throw_dir.x, throw_dir.y, throw_dir.z, 1.0f });
                                        cookies_to_control.pop_back();
                                    }
                                    else
                                    {
                                        cookies_to_control.pop_back();
                                    }

                                }
                                else
                                {
                                    cookies_to_control.pop_back();
                                }
                            }
                            else
                            {
                                cookies_to_control.pop_back();
                            }
                        }
                        else
                        {
                            cookies_to_control.clear();
                            return;
                        }
                    }
                    else
                        cookie_timer += dtime;
                }
            }
            else
                cookie_timer += dtime;
        }
        else
        {
            reset_cookie_control();
        }
            


        auto camera_dirY = camera->cameraRoot->world.rotate.GetVectorY(); //lookat
        auto camera_dirX = camera->cameraRoot->world.rotate.GetVectorX(); //right

        RE::NiPoint3 pos_to_place = player->GetPosition();
        pos_to_place.z += 120.0f;
        pos_to_place += camera_dirY * 50.0f;

        for (auto& cookie_handle : cookies_to_control)
        {
            if (cookie_handle.first)
            {
                auto cookie_ref = cookie_handle.first.get().get();
                if (cookie_ref)
                {
                    auto cookie_3d = cookie_ref->Get3D();
                    if (cookie_3d && cookie_3d->collisionObject)
                    {
                        auto body = cookie_3d->GetCollisionObject()->GetRigidBody();

                        if (body)
                        {
                            {
                                //float, preparing
                                float angle = cookie_handle.second;
                                float dir = 1.0f;

                                if (rotation_dir)
                                    dir = -1.0f;

                                angle += RE::NI_PI / 2.0f * cookie_rotate_timer * rotation_koef * dir;


                                RE::NiPoint3 shift = MiscThings::rotate_around_axis(camera_dirX * 40.0f, camera_dirY, angle);



                                RE::NiPoint3 desired_pos = pos_to_place + shift;
                                auto current_pos = cookie_ref->GetPosition();
                                auto delta_pos = desired_pos - current_pos;

                                body->SetLinearVelocity({ delta_pos.x, delta_pos.y, delta_pos.z, 1.0f });
                            }

                        }
                        else
                        {
                            reset_cookie_control();
                        }
                    }
                }
            }
        }


        if (big_cookie && big_cookie.get() && big_cookie.get().get())
        {
            if (cookie_timer3 < 10.0f)
            {
                cookie_timer3 += dtime;
                auto big_cookie_ref = big_cookie.get().get();
                static auto small_cookie = (RE::TESBoundObject*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1af975));
                static auto explosion = (RE::BGSExplosion*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1bec84));

                auto cookie_3d = big_cookie_ref->Get3D();
                if (small_cookie && explosion && cookie_3d && cookie_3d->collisionObject)
                {
                    auto body = cookie_3d->GetCollisionObject()->GetRigidBody();

                    if (body)
                    {
                        auto camera_dirY = camera->cameraRoot->world.rotate.GetVectorY(); //lookat
                        


                        auto filter = get_player_projectile_filter();

                        float raycast_distance = MiscThings::GetRaycastDistance(camera->cameraRoot->world.translate, camera_dirY, 500000.0f, player, filter);

                        auto target_pos = player->GetPosition() + camera_dirY * raycast_distance;

                        RE::NiPoint3 throw_dir = target_pos - big_cookie_ref->GetPosition();

                        throw_dir.Unitize();

                        throw_dir *= 300.0f;
                                         

                        body->SetLinearImpulse({ throw_dir.x, throw_dir.y, throw_dir.z, 1.0f });

                        RE::NiPoint3 vel = {};

                        big_cookie_ref->GetLinearVelocity(vel);

                        if (vel.z > -200.0f && cookie_timer3 > 0.5f)
                        {
                            big_cookie_ref->PlaceObjectAtMe(explosion, false);

                            for (auto i = 0; i < 30; i++)
                            {
                                auto new_item = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(small_cookie, big_cookie_ref->GetPosition(), {(float)std::rand() / RAND_MAX * 60.0f, (float)std::rand() / RAND_MAX * 60.0f, (float)std::rand() / RAND_MAX * 60.0f}, player->GetParentCell(), player->GetWorldspace(), nullptr, nullptr, player->GetHandle(), false, true);
                            }
                            big_cookie_ref->Disable();
                            big_cookie_ref->SetDelete(true);


                            reset_cookie_control3();
                        }


                    }
                }
            }
            else
            {
                auto big_cookie_ref = big_cookie.get().get();

                if (big_cookie_ref)
                {
                    if (explosion)
                    {
                        auto the_explosion = (RE::BGSExplosion*)explosion;
                        the_explosion->data.damage = 500.0f;
                        big_cookie_ref->PlaceObjectAtMe(explosion, false);
                    }

                    static auto small_cookie = (RE::TESBoundObject*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1af975));
                    static auto explosion = (RE::BGSExplosion*)RE::TESForm::LookupByID(MiscThings::get_shifted_id_for_mod(2, 0x1bec84));

                    if (small_cookie)
                    {
                        for (auto i = 0; i < 30; i++)
                        {
                            auto new_item = RE::TESDataHandler::GetSingleton()->CreateReferenceAtLocation(small_cookie, big_cookie_ref->GetPosition(), { (float)std::rand() / RAND_MAX * 60.0f, (float)std::rand() / RAND_MAX * 60.0f, (float)std::rand() / RAND_MAX * 60.0f }, player->GetParentCell(), player->GetWorldspace(), nullptr, nullptr, player->GetHandle(), false, true);
                        }
                    }


                    big_cookie_ref->Disable();
                    big_cookie_ref->SetDelete(true);




                    reset_cookie_control3();
                }

            }
               
        }


        cookie_rotate_timer += dtime;
        
    }

}
