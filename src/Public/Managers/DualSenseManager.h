#pragma once
#include "API/GamepadDefs.h"
#include <godot_cpp/classes/node.hpp>
#include "GCore/Types/DSCoreTypes.h"
using namespace godot;

namespace godot {
	class DualSenseManager : public Node {
		GDCLASS(DualSenseManager, Node)
	public:
		DualSenseManager();
		~DualSenseManager();
		
		static DualSenseManager *get_singleton() { return singleton; }
		
		
		virtual void _ready() override;
		virtual void _process(double delta) override;
		virtual void _exit_tree() override;
		
		static void test_rumble();
		static void test_weapon();
		static void test_lightbar();
		static void set_lightbar(Color color);
		static void set_rumble(int left_rumble, int right_rumble);
		static void set_rumble_for(int left_rumble, int right_rumble,int duration_ms);
		static void set_player_leds(GamepadDefs::LedPlayer led_mask, GamepadDefs::LedBrightness brightness);
		static void set_microphone_led(GamepadDefs::LedMic led);
		static void set_trigger_resistance(GamepadDefs::TriggerPosition StartZone, GamepadDefs::TriggerSoftness Strength, GamepadDefs::GamepadHand Hand);
	private:
		static DualSenseManager *singleton;
		static std::atomic<int> rumble_generation;
	protected:
		static void _bind_methods();
	};
}