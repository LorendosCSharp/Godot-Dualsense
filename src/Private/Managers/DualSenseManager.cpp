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
#include "GCore/Interfaces/IPlatformHardwareInfo.h"
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
    std::unique_ptr<IPlatformHardwareInfo> WindowsInstance = std::make_unique<FWindowsPlatform::FWindowsHardware>();
    IPlatformHardwareInfo::SetInstance(std::move(WindowsInstance));
#elif defined(__unix__)
    std::unique_ptr<IPlatformHardwareInfo> LinuxInstance = std::make_unique<FLinuxPlatform::FLinuxHardware>();
    IPlatformHardwareInfo::SetInstance(std::move(LinuxInstance));
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
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("test_lightbar"), &DualSenseManager::test_lightbar);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("test_weapon"), &DualSenseManager::test_weapon);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("test_rumble"), &DualSenseManager::test_rumble);

    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_lightbar","color"), &DualSenseManager::set_lightbar);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_rumble","left_rumble","right_rumble"), &DualSenseManager::set_rumble);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_rumble_for","left_rumble","right_rumble","duration_ms"), &DualSenseManager::set_rumble_for);
    ClassDB::bind_static_method("DualSenseManager", D_METHOD("set_player_leds","led_mask","brightness"), &DualSenseManager::set_player_leds);

    ClassDB::add_signal("DualSenseManager", MethodInfo("device_connected", PropertyInfo(Variant::INT, "gamepad_id")));
    ClassDB::add_signal("DualSenseManager", MethodInfo("device_disconnected", PropertyInfo(Variant::INT, "gamepad_id")));
}

void DualSenseManager::test_rumble() {
    if (const auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        UtilityFunctions::print("test_rumble vibration...");
        gamepad->SetVibration(255, 255);
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}

void DualSenseManager::test_lightbar() {
    if (const auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        gamepad->SetLightbar({255, 255, 0, 0});
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}

void DualSenseManager::test_weapon() {
    if (const auto gamepad = FGodotDeviceRegistry::GetTriggerGamepad(1)) {
        UtilityFunctions::print("test_weapon (weapon 0x25) effect...");
        gamepad->SetWeapon25(
            GamepadDefs::POS_START,
            GamepadDefs::AMP_HIGH,
            GamepadDefs::SUSTAINED,
            GamepadDefs::TriggerForceMask::MASK_FORCE_HIGH,
            static_cast<EDSGamepadHand>(GamepadDefs::GamepadHand::RIGHT_HAND)
        );
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}

// --- Implementation of functions from GamepadCore/Source/Private/GImplementations/Libraries/DualSense/DualSenseLibrary.cpp ---

void DualSenseManager::set_lightbar(Color color) {
    if (const auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        DSCoreTypes::FDSColor newColor = {
            static_cast<uint8_t>(color.r * 255.0f),
            static_cast<uint8_t>(color.g * 255.0f),
            static_cast<uint8_t>(color.b * 255.0f),
            static_cast<uint8_t>(color.a * 255.0f)
        };
        gamepad->SetLightbar(newColor);
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}

void DualSenseManager::set_rumble(int left_rumble, int right_rumble) {
    if (const auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        gamepad->SetVibration(static_cast<uint8_t>(left_rumble), static_cast<uint8_t>(right_rumble));
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

    if (auto gamepad =
        FGodotDeviceRegistry::GetGamepad(gamepad_id))
    {
        gamepad->SetVibration(
            static_cast<uint8_t>(left_rumble),
            static_cast<uint8_t>(right_rumble)
        );

        int my_generation = ++rumble_generation;

        std::thread(
            [gamepad_id, duration_ms, my_generation]()
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(duration_ms)
                );

                if (my_generation != rumble_generation)
                    return;

                if (auto gamepad =
                    FGodotDeviceRegistry::GetGamepad(gamepad_id))
                {
                    gamepad->SetVibration(0, 0);
                }
            }
        ).detach();
    }
}
void DualSenseManager::set_player_leds(GamepadDefs::LedPlayer led_mask, GamepadDefs::LedBrightness brightness){
    if (const auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        gamepad->SetPlayerLed(static_cast<uint8_t>(led_mask), static_cast<uint8_t>(brightness));
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}
void DualSenseManager::set_microphone_led(GamepadDefs::LedMic led){
    if (const auto gamepad = FGodotDeviceRegistry::GetGamepad(1)) {
        gamepad->SetMicrophoneLed(static_cast<uint8_t>(led));
    } else {
        UtilityFunctions::print("Not found gamepad");
    }
}

void DualSenseManager::set_trigger_resistance(GamepadDefs::TriggerPosition StartZone, GamepadDefs::TriggerSoftness Strength, GamepadDefs::GamepadHand Hand){

    if (const auto gamepad = FGodotDeviceRegistry::GetTriggerGamepad(1)) {
        gamepad->SetResistance(
            static_cast<uint8_t>(StartZone),
            static_cast<uint8_t>(Strength),
            static_cast<EDSGamepadHand>(Hand)
        );
    } else {
        UtilityFunctions::print("Not found gamepad");
    }

}