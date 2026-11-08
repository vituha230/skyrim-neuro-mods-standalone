

#include "WalkerProcessor.hpp"
#include "Misc.hpp"
#include "main.hpp"


namespace Observer
{

	RE::TESRace* old_player_race = nullptr;


	void player_state_monitor(float dtime)
	{

		auto player = RE::PlayerCharacter::GetSingleton();
		static auto ranger_perk = (RE::BGSPerk*)RE::TESForm::LookupByID(0x58F63);

		if (ranger_perk)
		{
			if (!player->HasPerk(ranger_perk))
			{
				if (MiscThings::get_hand_contents(true) && MiscThings::get_hand_contents(true)->formID == MiscThings::get_shifted_id_for_mod(1, 0x665ed))
				{
					bool test123 = false;
					player->GetGraphVariableBool("bPerkQuickShot", test123);
					if (!test123)
						player->SetGraphVariableBool("bPerkQuickShot", true);
				}
				else
				{
					bool test123 = false;
					player->GetGraphVariableBool("bPerkQuickShot", test123);
					if (test123)
						player->SetGraphVariableBool("bPerkQuickShot", false);
				}

			}
		}


		if (player->race != old_player_race)
		{
			old_player_race = player->race;

			MiscThings::update_player_collision_groups();
		}
	}
}