@tool
extends Node

var enum_name := ""

@export var option_button: OptionButton

func _ready() -> void:
	populate()

func _get_property_list():
	return [{
		"name": "enum_name",
		"type": TYPE_STRING,
		"hint": PROPERTY_HINT_ENUM,
		"hint_string": ",".join(
			ClassDB.class_get_enum_list("GamepadDefs")
		)
	}]

func _get(property):
	if property == "enum_name":
		return enum_name

func _set(property, value):
	if property == "enum_name":
		enum_name = value
		populate()
		return true

	return false

func populate():
	if not option_button:
		return

	option_button.clear()

	var constants = ClassDB.class_get_enum_constants(
		"GamepadDefs",
		enum_name
	)

	for constant_name in constants:
		var value = ClassDB.class_get_integer_constant(
			"GamepadDefs",
			constant_name
		)

		option_button.add_item(constant_name, value)
