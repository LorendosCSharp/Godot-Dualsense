#include "Managers/DualSenseManager.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include "Adapter/GodotDeviceRegistry.h"
#ifdef _WIN32
#include "Platforms/Windows/WindowsHardwarePolicy.h"
#endif
#ifdef __unix__
#include "Platforms/Linux/LinuxHardwarePolicy.h"
#endif
#include "GCore/Interfaces/IPlatformHardware.h"
#include <thread>
#include <chrono>
using namespace godot;

DualSenseManager *DualSenseManager::singleton = nullptr;

DualSenseManager::DualSenseManager() {
    singleton = this;
    connect("device_connected",Callable(this, "_on_device_connected"));
}

DualSenseManager::~DualSenseManager() {
    if (singleton == this) {
        singleton = nullptr;
    }
}

void DualSenseManager::_ready() {
    UtilityFunctions::print("[DualSenseManager] Initialize GamepadCore...");

    // 1.Hardware
#ifdef _WIN32
    std::unique_ptr<IPlatformHardware> WindowsInstance = std::make_unique<FWindowsPlatform::FWindowsHardware>();
    IPlatformHardwareInfo::SetInstance(std::move(WindowsInstance));
#elif defined(__unix__)
    std::unique_ptr<IPlatformHardware> LinuxInstance = std::make_unique<FLinuxPlatform::FLinuxHardware>();
    IPlatformHardware::SetInstance(std::move(LinuxInstance));
#endif

    FGodotDeviceRegistry::Initialize();
    UtilityFunctions::print("[DualSenseManager] GamepadCore initialized!");
}

void DualSenseManager::_process(double delta) {
    FGodotDeviceRegistry::DiscoverDevices(delta);
}

void DualSenseManager::_exit_tree() {
    FGodotDeviceRegistry::Shutdown();
}

void DualSenseManager::_bind_methods() {

    ClassDB::bind_static_method("DualSenseManager", D_METHOD("get_battery"), &DualSenseManager::get_battery);


    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_lightbar","color"), &DualSenseManager::set_lightbar);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_rumble","left_rumble","right_rumble"), &DualSenseManager::set_rumble);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_rumble_for","left_rumble","right_rumble","duration_ms"), &DualSenseManager::set_rumble_for);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_player_leds","led_mask","brightness"), &DualSenseManager::set_player_leds);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("send_audio_haptic","data"), &DualSenseManager::send_audio_haptic);

    //trigger effects
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_resistance","start_zone","strength","hand"), &DualSenseManager::set_trigger_resistance);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_galloping","start_position","end_position","first_foot","second_foot","frequency","hand"), &DualSenseManager::set_trigger_galloping);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_gamecube","hand"), &DualSenseManager::set_trigger_gamecube);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_bow","start_zone","snap_back","hand"), &DualSenseManager::set_trigger_bow);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_weapon","start_zone","amplitude","behavior","trigger","hand"), &DualSenseManager::set_trigger_weapon);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_machine_gun","start_zone","behavior","amplitude","frequency","hand"), &DualSenseManager::set_trigger_machine_gun);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_machine","start_zone","behavior_flag","force","amplitude","period","frequency","hand"), &DualSenseManager::set_trigger_machine);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_custom","hand","hex_bytes"), &DualSenseManager::set_trigger_custom);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_trigger_stop","hand"), &DualSenseManager::set_trigger_stop);

    ClassDB::add_signal("DualSenseManager", MethodInfo("device_connected", PropertyInfo(Variant::INT, "gamepad_id")));
    ClassDB::add_signal("DualSenseManager", MethodInfo("device_disconnected", PropertyInfo(Variant::INT, "gamepad_id")));
}


float DualSenseManager::get_battery(){
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        return gamepad->GetBattery();
    }
    return 0;
}


void DualSenseManager::set_lightbar(Color color) {
    if (auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        DSCoreTypes::FDSColor newColor = {
            static_cast<uint8_t>(color.r * 255.0f),
            static_cast<uint8_t>(color.g * 255.0f),
            static_cast<uint8_t>(color.b * 255.0f)
        };
        if (auto lightbar = gamepad->GetIGamepadLightbar()) {
            lightbar->SetLightbar(newColor);
            gamepad->UpdateOutput();
        } else {
            UtilityFunctions::print("Lightbar interface not available");
        }
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}

void DualSenseManager::set_rumble(int left_rumble, int right_rumble) {
    if (auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if (auto rumbles = gamepad->GetIGamepadRumbles()) {
            rumbles->SetVibration(static_cast<uint8_t>(left_rumble), static_cast<uint8_t>(right_rumble));
            gamepad->UpdateOutput();
        } else {
            UtilityFunctions::print("Rumbles interface not available");
        }
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}
// Best would be to reimplement this in some other way, but I will stick with this
std::atomic<int> DualSenseManager::rumble_generation = 0;
void DualSenseManager::set_rumble_for(
    int left_rumble,
    int right_rumble,
    int duration_ms
) {
    int gamepad_id = 1;

    if (auto gamepad = FGodotDeviceRegistry::GetGamepad(gamepad_id))
    {
        if (auto rumbles = gamepad->GetIGamepadRumbles()) {
            rumbles->SetVibration(
                static_cast<uint8_t>(left_rumble),
                static_cast<uint8_t>(right_rumble)
            );
            gamepad->UpdateOutput();
        }

        int my_generation = ++rumble_generation;

        std::thread(
            [gamepad_id, duration_ms, my_generation]()
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(duration_ms)
                );

                if (my_generation != rumble_generation)
                    return;

                if (auto gamepad = FGodotDeviceRegistry::GetGamepad(gamepad_id))
                {
                    if (auto rumbles = gamepad->GetIGamepadRumbles()) {
                        rumbles->SetVibration(0, 0);
                    }
                    gamepad->UpdateOutput();
                }
            }
        ).detach();
    }
}
void DualSenseManager::set_player_leds(GamepadDefs::LedPlayer led_mask, GamepadDefs::LedBrightness brightness){
    if (auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if (auto lightbar = gamepad->GetIGamepadLightbar()) {
            lightbar->SetPlayerLed(static_cast<EDSPlayer>(static_cast<uint8_t>(led_mask)), static_cast<uint8_t>(brightness));
            gamepad->UpdateOutput();
        } else {
            UtilityFunctions::print("Lightbar interface not available");
        }
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}
void DualSenseManager::set_microphone_led(GamepadDefs::LedMic led){
    if (auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if (auto lightbar = gamepad->GetIGamepadLightbar()) {
            lightbar->SetMicrophoneLed(static_cast<EDSMic>(static_cast<uint8_t>(led)));
            gamepad->UpdateOutput();
        } else {
            UtilityFunctions::print("Lightbar interface not available");
        }
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}

void DualSenseManager::set_trigger_resistance(GamepadDefs::TriggerPositionMask start_zone, GamepadDefs::TriggerForceMask strenght, GamepadDefs::GamepadHand hand){

    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger=gamepad->GetIGamepadTrigger()){
            trigger->SetResistance(
                static_cast<uint8_t>(start_zone),
                static_cast<uint8_t>(strenght),
                static_cast<EDSGamepadHand>(hand)
            );
            gamepad->UpdateOutput();
        }
    } else {
        UtilityFunctions::print("Not found gamepad");
    }

}
// Broken for now, I can't get the right effect in godot. IDK what values need to pass here
void DualSenseManager::set_trigger_galloping(
    GamepadDefs::TriggerPositionMask start_position,
    GamepadDefs::TriggerPositionMask end_position,
    GamepadDefs::TriggerAmplitude first_foot,
    GamepadDefs::TriggerAmplitude second_foot,
    uint8_t frequency,
    GamepadDefs::GamepadHand hand)
{
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){
            trigger->SetGalloping23(
                static_cast<uint8_t>(start_position),
                static_cast<uint8_t>(end_position),
                static_cast<uint8_t>(first_foot),
                static_cast<uint8_t>(second_foot),
                frequency,
                static_cast<EDSGamepadHand>(hand)
            );
        }
        gamepad->UpdateOutput();
    }
}
void DualSenseManager::set_trigger_gamecube(
    GamepadDefs::GamepadHand hand)
{
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){
            trigger->SetGameCube(
                static_cast<EDSGamepadHand>(hand)
            );
        }

    }
}
void DualSenseManager::set_trigger_bow(
    GamepadDefs::TriggerPositionMask start_zone,
    GamepadDefs::SnapBack snap_back,
    GamepadDefs::GamepadHand hand)
{
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){
            trigger->SetBow22(
                static_cast<uint8_t>(start_zone),
                static_cast<uint8_t>(snap_back),
                static_cast<EDSGamepadHand>(hand)
            );
        }

    }
}
void DualSenseManager::set_trigger_weapon(
    GamepadDefs::TriggerPositionMask start_zone,
    GamepadDefs::TriggerAmplitude amplitude,
    GamepadDefs::TriggerEffectBehavior behavior,
    GamepadDefs::WeaponTrigger triggerStrength,
    GamepadDefs::GamepadHand hand)
{
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){
            trigger->SetWeapon25(
                static_cast<uint8_t>(start_zone),
                static_cast<uint8_t>(amplitude),
                static_cast<uint8_t>(behavior),
                static_cast<uint8_t>(triggerStrength),
                static_cast<EDSGamepadHand>(hand)
            );
        }

    }
}
void DualSenseManager::set_trigger_machine_gun(
    GamepadDefs::TriggerPositionMask start_zone,
    GamepadDefs::TriggerEffectBehavior behavior,
    GamepadDefs::TriggerAmplitude amplitude,
    uint8_t frequency,
    GamepadDefs::GamepadHand hand)
{
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){
            trigger->SetMachineGun26(
                static_cast<uint8_t>(start_zone),
                static_cast<uint8_t>(behavior),
                static_cast<uint8_t>(amplitude),
                frequency,
                static_cast<EDSGamepadHand>(hand)
            );
        }

    }
}
void DualSenseManager::set_trigger_machine(
    GamepadDefs::TriggerPositionMask start_zone,
    GamepadDefs::TriggerEffectBehavior behavior_flag,
    GamepadDefs::TriggerForceMask force,
    GamepadDefs::TriggerAmplitude amplitude,
    uint8_t period,
    uint8_t frequency,
    GamepadDefs::GamepadHand hand)
{
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){

            trigger->SetMachine27(
                static_cast<uint8_t>(start_zone),
                static_cast<uint8_t>(behavior_flag),
                static_cast<uint8_t>(force),
                static_cast<uint8_t>(amplitude),
                period,
                frequency,
                static_cast<EDSGamepadHand>(hand)
            );
        }

    }
}
void DualSenseManager::set_trigger_custom(
    GamepadDefs::GamepadHand hand,
    const PackedByteArray &hex_bytes)
{
    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){

            std::vector<uint8_t> data;
            data.reserve(hex_bytes.size());

            for (int i = 0; i < hex_bytes.size(); i++) {
                data.push_back(static_cast<uint8_t>(hex_bytes[i]));
            }

            trigger->SetCustomTrigger(
                static_cast<EDSGamepadHand>(hand),
                data
            );
        }
    }
}
void DualSenseManager::set_trigger_stop(GamepadDefs::GamepadHand hand){

    if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        if(auto trigger = gamepad->GetIGamepadTrigger()){
            trigger->StopTrigger(
                static_cast<EDSGamepadHand>(hand)
            );
        }
    }
}

void DualSenseManager::send_audio_haptic(const PackedByteArray &data){
    // if ( auto gamepad = FGodotDeviceRegistry::GetGamepad(1))
    // {
    //     std::vector<uint8_t> native_data;
    //     native_data.reserve(data.size());

    //     for (int i = 0; i < data.size(); i++)
    //     {
    //         native_data.push_back(static_cast<uint8_t>(data[i]));
    //     }
    //     UtilityFunctions::print(data);
    //      auto haptics=gamepad->GetIGamepadHaptics();
    //     haptics->AudioHapticUpdate(native_data);
    // }
}

