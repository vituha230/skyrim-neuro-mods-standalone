

#pragma once


namespace MiscThings {

	uint32_t get_player_projectile_filter();
	float GetRaycastDistance(RE::NiPoint3 from, RE::NiPoint3 aimVector, float distance, RE::TESObjectREFR* target, uint32_t filter);


	void add_cookies_to_controll(std::vector<std::pair<RE::ObjectRefHandle, float>> cookies, bool stick_mode);
	void remember_big_cookie(RE::ObjectRefHandle cookie);
	RE::NiPoint3 rotate_around_axis(RE::NiPoint3 v, RE::NiPoint3 axis, float radians);
	void reset_cookie_control();
	void reset_cookie_control2();
	void reset_cookie_control3();

	void explode_cookies_nearby();
	void do_reset_player_hp(float hp);

	void cookie_controller(float dtime);


	int get_shifted_id_for_mod(int mod, uint32_t base_id);

	RE::TESForm* get_hand_contents(bool right);


	void update_player_collision_groups();

}






