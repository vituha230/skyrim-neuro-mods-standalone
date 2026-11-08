

#pragma once

#include "main.hpp"

namespace Observer {


	void player_state_monitor(float dtime);


	class EventSink :
		public REX::TSingleton<EventSink>,
		public RE::BSTEventSink<RE::TESHitEvent>
	{
	public:
		void Init()
		{
			RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESHitEvent>(this);
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* test_var) override
		{
			if (a_event)
			{
				auto player = RE::PlayerCharacter::GetSingleton();
				RE::TESObjectREFRPtr target = a_event->target;
				RE::FormID weapon_formid = a_event->source;


				if (target && target.get() && target.get() == player)
				{
					if (weapon_formid)
					{
						auto weapon_form = RE::TESObjectREFR::LookupByID(weapon_formid);
						if (weapon_form && weapon_form->formID == MiscThings::get_shifted_id_for_mod(2, 0x1bec84)) //meteorite explosion
						{
							MiscThings::do_reset_player_hp(player->GetBaseActorValue(RE::ActorValue::kHealth));
							player->SetBaseActorValue(RE::ActorValue::kHealth, 5000.0f);


						}
					}
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	};

};
