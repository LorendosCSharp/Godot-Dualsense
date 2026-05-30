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

		static float get_battery();

		static void set_lightbar(Color color);
		static void set_rumble(int left_rumble, int right_rumble);
		static void set_rumble_for(int left_rumble, int right_rumble,int duration_ms);
		static void set_player_leds(GamepadDefs::LedPlayer led_mask, GamepadDefs::LedBrightness brightness);
		static void set_microphone_led(GamepadDefs::LedMic led);
		static void send_audio_haptic(const PackedByteArray &data);
		// trigger effects
		static void set_trigger_resistance(
			GamepadDefs::TriggerPositionMask start_zone, 
			GamepadDefs::TriggerForceMask strenght, 
			GamepadDefs::GamepadHand hand
		);

		static void set_trigger_galloping(
			GamepadDefs::TriggerPositionMask start_position,
			GamepadDefs::TriggerPositionMask end_position,
			GamepadDefs::TriggerAmplitude first_foot,
			GamepadDefs::TriggerAmplitude second_foot,
			uint8_t frequency,
			GamepadDefs::GamepadHand hand
		);

		static void set_trigger_gamecube(
			GamepadDefs::GamepadHand hand
		);

		static void set_trigger_bow(
			GamepadDefs::TriggerPositionMask start_zone,
			GamepadDefs::SnapBack snap_back,
			GamepadDefs::GamepadHand hand
		);

		static void set_trigger_weapon(
			GamepadDefs::TriggerPositionMask start_zone,
			GamepadDefs::TriggerAmplitude amplitude,
			GamepadDefs::TriggerEffectBehavior behavior,
			GamepadDefs::WeaponTrigger trigger,
			GamepadDefs::GamepadHand hand
		);

		static void set_trigger_machine_gun(
			GamepadDefs::TriggerPositionMask start_zone,
			GamepadDefs::TriggerEffectBehavior behavior,
			GamepadDefs::TriggerAmplitude amplitude,
			uint8_t frequency,
			GamepadDefs::GamepadHand hand
		);

		static void set_trigger_machine(
			GamepadDefs::TriggerPositionMask start_zone,
			GamepadDefs::TriggerEffectBehavior behavior_flag,
			GamepadDefs::TriggerForceMask force,
			GamepadDefs::TriggerAmplitude amplitude,
			uint8_t period,
			uint8_t frequency,
			GamepadDefs::GamepadHand hand
		);

		static void set_trigger_custom(
			GamepadDefs::GamepadHand hand,
			const PackedByteArray &hex_bytes
		);

		static void set_trigger_stop(GamepadDefs::GamepadHand hand);
	private:
		static DualSenseManager *singleton;
		static std::atomic<int> rumble_generation;
	protected:
		static void _bind_methods();
	};
}