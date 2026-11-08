
#pragma warning( disable : 4305 )

#include "main.hpp"
#include "WalkerProcessor.hpp"
#include <vector>
#include "Misc.hpp"
#include "Observer.hpp"



namespace Hooks {

    struct LoadingMenuProcessMessage { 
        static RE::UI_MESSAGE_RESULTS thunk(RE::LoadingMenu* menu, RE::UIMessage& a_message) {
            if (a_message.type.get() == RE::UI_MESSAGE_TYPE::kShow) {

                MiscThings::reset_cookie_control();
                MiscThings::reset_cookie_control2();
                MiscThings::reset_cookie_control3();
            }

            if (a_message.type.get() == RE::UI_MESSAGE_TYPE::kHide) {

                MiscThings::update_player_collision_groups();
            }

            return originalFunction(menu, a_message);
        }
        static inline REL::Relocation<decltype(thunk)> originalFunction;
        static inline void Install() { originalFunction = REL::Relocation<std::uintptr_t>(RE::VTABLE_LoadingMenu[0]).write_vfunc(0x4, thunk); }
    };


}


//this update doesnt work in pause
class MyHook {
    static void Update(RE::PlayerCharacter* a, float dtime) 
    {

        std::string result = "";

        auto player = RE::PlayerCharacter::GetSingleton();
        auto player_ref = player->AsReference();
        auto player_actor = (RE::Actor*)player_ref;

        
        _Update(a, dtime);

       
        MiscThings::cookie_controller(dtime);
        Observer::player_state_monitor(dtime);

    //end of myhookupdate
    }


    static inline REL::Relocation<decltype(Update)> _Update;

public:
    static void Hook() {
        _Update = REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0xAD, Update);
    }
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SKSE ZONE

// For every message sent to the plugin by SKSE, log the message.
void MessageListener(SKSE::MessagingInterface::Message* message) {
    switch (message->type) {
        // Descriptions are taken from the original skse64 library
        // See:
        // https://github.com/ianpatt/skse64/blob/09f520a2433747f33ae7d7c15b1164ca198932c3/skse64/PluginAPI.h#L193-L212
    case SKSE::MessagingInterface::kPostLoad:
        REX::INFO("kPostLoad: sent to registered plugins once all plugins have been loaded");
        break;
    case SKSE::MessagingInterface::kPostPostLoad:
        REX::INFO(
            "kPostPostLoad: sent right after kPostLoad to facilitate the correct dispatching/registering of "
            "messages/listeners");
        break;
    case SKSE::MessagingInterface::kPreLoadGame:
        // message->dataLen: length of file path, data: char* file path of .ess savegame file
        REX::INFO("kPreLoadGame: sent immediately before savegame is read");
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
        // You will probably want to handle this event if your plugin uses a Preload callback
        // as there is a chance that after that callback is invoked the game will encounter an error
        // while loading the saved game (eg. corrupted save) which may require you to reset some of your
        // plugin state.
        REX::INFO("kPostLoadGame: sent after an attempt to load a saved game has finished");
        break;
    case SKSE::MessagingInterface::kSaveGame:
        REX::INFO("kSaveGame");
        break;
    case SKSE::MessagingInterface::kDeleteGame:
        // message->dataLen: length of file path, data: char* file path of .ess savegame file
        REX::INFO("kDeleteGame: sent right before deleting the .skse cosave and the .ess save");
        break;
    case SKSE::MessagingInterface::kInputLoaded:
        REX::INFO("kInputLoaded: sent right after game input is loaded, right before the main menu initializes");
        break;
    case SKSE::MessagingInterface::kNewGame:
        // message-data: CharGen TESQuest pointer (Note: I haven't confirmed the usefulness of this yet!)
        REX::INFO("kNewGame: sent after a new game is created, before the game has loaded");
        break;
    case SKSE::MessagingInterface::kDataLoaded:
        REX::INFO("kDataLoaded: sent after the data handler has loaded all its forms");
        MyHook::Hook();
        SKSE::AllocTrampoline(14);

        Observer::EventSink::GetSingleton()->Init();
        break;
    default:
        REX::INFO("Unknown system message of type: {}", message->type);
        break;
    }
}


SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
    Hooks::LoadingMenuProcessMessage::Install();
    WalkerProcessor::install_hook();

	SKSE::GetMessagingInterface()->RegisterListener(MessageListener);


	return true;
}
