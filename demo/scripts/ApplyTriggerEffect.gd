extends Node

@export var trigger_effect: OptionButton
@export var effects_settigns: Array[Control]

func _ready() -> void:
	trigger_effect.item_selected.connect(_on_effect_changed)

	for setting in effects_settigns:
		setting.visible = false

	_on_effect_changed(trigger_effect.selected)


func _on_effect_changed(index: int) -> void:
	# --- UI switching ---
	for i in range(effects_settigns.size()):
		effects_settigns[i].visible = (i == index)



# =========================
# MAIN DISPATCH
# =========================
func _apply_trigger() -> void:
	var index:int = trigger_effect.selected
	var panel := effects_settigns[index]
	match index:

		0: # RESISTANCE
			print(_get_i(panel, 2))
			DualSenseManager.set_trigger_resistance(
				_get_i(panel, 0),
				_get_i(panel, 1),
				_get_i(panel, 2)
			)

		1: # GALLOPING
			DualSenseManager.set_trigger_galloping(
				_get_i(panel, 0),
				_get_i(panel, 1),
				_get_i(panel, 2),
				_get_i(panel, 3),
				_get_i(panel, 4),
				_get_i(panel, 5)
			)

		2: # GAMECUBE
			DualSenseManager.set_trigger_gamecube(
				_get_i(panel, 0)
			)

		3: # BOW
			DualSenseManager.set_trigger_bow(
				_get_i(panel, 0),
				_get_i(panel, 1),
				_get_i(panel, 2)
			)

		4: # WEAPON
			DualSenseManager.set_trigger_weapon(
				_get_i(panel, 0),
				_get_i(panel, 1),
				_get_i(panel, 2),
				_get_i(panel, 3),
				_get_i(panel, 4)
			)

		5: # MACHINE GUN
			DualSenseManager.set_trigger_machine_gun(
				_get_i(panel, 0),
				_get_i(panel, 1),
				_get_i(panel, 2),
				_get_i(panel, 3),
				_get_i(panel, 4)
			)

		6: # MACHINE
			DualSenseManager.set_trigger_machine(
				_get_i(panel, 0),
				_get_i(panel, 1),
				_get_i(panel, 2),
				_get_i(panel, 3),
				_get_i(panel, 4),
				_get_i(panel, 5),
				_get_i(panel, 6)
			)

		7: # CUSTOM
			var bytes := []
			for c in panel.get_children():
				if c is LineEdit:
					bytes.append(int(c.text))
				elif c is SpinBox:
					bytes.append(int(c.value))

			DualSenseManager.set_trigger_custom(
				_get_i(panel, 0),
				bytes
			)

		8: # STOP
			DualSenseManager.set_trigger_stop(
				_get_i(panel, 0)
			)


# =========================
# SAFE CHILD READER
# =========================
func _get_i(panel: Control, index: int) -> int:
	var controls: Array = []
	_collect_inputs(panel, controls)

	if index >= 0 and index < controls.size():
		return _read_value(controls[index])

	return 0


func _collect_inputs(node: Node, out: Array) -> void:
	for child in node.get_children():

		if child is OptionButton or child is SpinBox or child is LineEdit:
			out.append(child)

		# IMPORTANT: recurse into HBox / VBox / Containers
		elif child is Control:
			_collect_inputs(child, out)


func _read_value(node: Node) -> int:
	if node is OptionButton:
		return node.selected

	if node is SpinBox:
		return int(node.value)

	if node is LineEdit:
		return int(node.text)

	return 0
