extends DualSenseManager

func _on_device_connected(device_id: int):
	print("DualSense Connected! ID: ", device_id)
	# Test methods available in DualSenseManager
	#test_lightbar()   # Sets lightbar to red
	set_player_leds(0x0, 0x0)
	#test_rumble()     # Tests vibration

func _on_device_disconnected(device_id: int):
	print("DualSense Disconnected: ", device_id)
	

func changeToRandomColor():
	var newColor:Color
	newColor=Color.from_hsv(randf(),1,1,1)
	print("New Color was set: ",newColor)
	set_lightbar(newColor)
	pass
	
func changeToRandomRumble():
	
	var leftRumble:int =randi_range(0,255)
	var rightRumble:int =randi_range(0,255)
	print(r"New Rumble was set: %d, %d" % [leftRumble,rightRumble])
	set_rumble(leftRumble,rightRumble)
	pass
func changeToRandomRumbleFor():
	
	var leftRumble:int =randi_range(0,255)
	var rightRumble:int =randi_range(0,255)
	var rumbleDuration:int =randi_range(0,5000)
	print(r"New Rumble was set: %d, %d for %d milliseconds" % [leftRumble,rightRumble,rumbleDuration])
	set_rumble_for(leftRumble,rightRumble,rumbleDuration)
	pass

var currentLed:int=0x00
func changePlayerLeds():
	currentLed+=1
	if currentLed > 32:
		currentLed=0
	var customValue:int= 0x10
	set_player_leds(customValue,GamepadDefs.LedBrightness.BRIGHTNESS_HIGH)
	pass
func testAudioHaptics():
	var packet := PackedByteArray()

	for i in range(32):
		var v = int((sin(i * 0.4) * 0.5 + 0.5) * 255.0)

		packet.append(v)
		packet.append(v)
		
	send_audio_haptic(packet)
	pass


func testResistance():
	set_trigger_resistance(GamepadDefs.MASK_POS_EARLY,GamepadDefs.MASK_FORCE_HIGH,GamepadDefs.LEFT_HAND)
	
	pass
	
func getBattery():
	print(get_battery())
	pass
