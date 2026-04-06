# https://h.hlktech.com/download/HLK-LD2451-24G/1/LD2451%20串口通信协议%20V1.03.pdf
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import uart
from esphome.const import CONF_ID, CONF_DIRECTION, CONF_BAUD_RATE

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True
MAX_TARGETS = 20
CONF_LD2451_ID = "ld2451_id"
CONF_VALID_TRIGS = "valid_trigs"
CONF_SIGNAL_THRESHOLD = "signal_threshold"
CONF_MAX_DISTANCE = "max_distance"
CONF_MIN_DISTANCE = "min_distance"
CONF_MIN_SPEED = "min_speed"
CONF_NO_TARGET_DELAY = "no_target_delay"

ld2451 = cg.esphome_ns.namespace("ld2451")
LD2451Component = ld2451.class_("LD2451Component", cg.Component, uart.UARTDevice)

LD2451_DIRECTION = ld2451.enum("LD2451_DIRECTION")
LD2451_DIRECTION_OPTIONS = {
    "away": LD2451_DIRECTION.LD2451_DIRECTION_AWAY,
    "towards": LD2451_DIRECTION.LD2451_DIRECTION_TOWARDS,
    "both": LD2451_DIRECTION.LD2451_DIRECTION_BOTH
}

LD2451_BAUD_RATE = ld2451.enum("LD2451_BAUD_RATE")
LD2451_BAUD_RATE_OPTIONS = {
   9600    : LD2451_BAUD_RATE.LD2451_BAUD_RATE_9600,
   19200   : LD2451_BAUD_RATE.LD2451_BAUD_RATE_19200,
   38400   : LD2451_BAUD_RATE.LD2451_BAUD_RATE_38400,
   57600   : LD2451_BAUD_RATE.LD2451_BAUD_RATE_57600,
   115200  : LD2451_BAUD_RATE.LD2451_BAUD_RATE_115200,
   230400  : LD2451_BAUD_RATE.LD2451_BAUD_RATE_230400,
   256000  : LD2451_BAUD_RATE.LD2451_BAUD_RATE_256000,
   460800  : LD2451_BAUD_RATE.LD2451_BAUD_RATE_460800,
}


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LD2451Component),
            cv.Optional(CONF_VALID_TRIGS, default=1): cv.int_range(1, 0x0A),
            cv.Optional(CONF_SIGNAL_THRESHOLD, default=3): cv.int_range(0, 8),

            cv.Optional(CONF_MAX_DISTANCE, default=255): cv.int_range(0x0A, 0xFF),
            cv.Optional(CONF_DIRECTION, default="both"): cv.enum(LD2451_DIRECTION_OPTIONS),
            cv.Optional(CONF_MIN_SPEED, default=0): cv.int_range(0x00, 0x78),
            cv.Optional(CONF_NO_TARGET_DELAY, default=1): cv.uint8_t,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld2451",
    require_tx=True,
    require_rx=True,
    parity=None,
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_valid_trigs(config[CONF_VALID_TRIGS]))
    cg.add(var.set_signal_threshold(config[CONF_SIGNAL_THRESHOLD]))

    cg.add(var.set_max_distance(config[CONF_MAX_DISTANCE]))
    cg.add(var.set_direction(config[CONF_DIRECTION]))
    cg.add(var.set_min_speed(config[CONF_MIN_SPEED]))
    cg.add(var.set_no_target_delay(config[CONF_NO_TARGET_DELAY]))

LD2451SetBaudRateAction = ld2451.class_("LD2451SetBaudRateAction", automation.Action)
LD2451_SET_BAUD_RATE_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2451Component),
        cv.Required(CONF_BAUD_RATE): cv.enum(LD2451_BAUD_RATE_OPTIONS)
    }
)
@automation.register_action("ld2451.set_baud_rate", LD2451SetBaudRateAction, LD2451_SET_BAUD_RATE_ACTION_SCHEMA, synchronous=True)
async def ld2451_set_baud_rate_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    baud_rate = await cg.templatable(config[CONF_BAUD_RATE], args, cg.uint8)
    cg.add(var.set_baud_rate(baud_rate))
    return var

LD2451EnableConfigAction = ld2451.class_("LD2451EnableConfigAction", automation.Action)
LD2451_ENABLE_CONFIG_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2451Component),
    }
)
@automation.register_action("ld2451.enable_config", LD2451EnableConfigAction, LD2451_ENABLE_CONFIG_ACTION_SCHEMA, synchronous=True)
async def ld2451_enable_config_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2451DisableConfigAction = ld2451.class_("LD2451DisableConfigAction", automation.Action)
LD2451_DISABLE_CONFIG_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2451Component),
    }
)
@automation.register_action("ld2451.disable_config", LD2451DisableConfigAction, LD2451_DISABLE_CONFIG_ACTION_SCHEMA, synchronous=True)
async def ld2451_disable_config_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2451ResetAction = ld2451.class_("LD2451ResetAction", automation.Action)
LD2451_RESET_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2451Component),
    }
)
@automation.register_action("ld2451.reset", LD2451ResetAction, LD2451_RESET_ACTION_SCHEMA, synchronous=True)
async def ld2451_reset_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2451RestartAction = ld2451.class_("LD2451RestartAction", automation.Action)
LD2451_RESTART_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2451Component),
    }
)
@automation.register_action("ld2451.restart", LD2451RestartAction, LD2451_RESTART_ACTION_SCHEMA, synchronous=True)
async def ld2451_restart_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2451SetSensitivityAction = ld2451.class_("LD2451SetSensitivityAction", automation.Action)
LD2451_SET_SENSITIVITY_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2451Component),
        cv.Required(CONF_VALID_TRIGS): cv.templatable(cv.int_range(1, 0x0A)),
        cv.Required(CONF_SIGNAL_THRESHOLD): cv.templatable(cv.int_range(0, 8)),
    }
)
@automation.register_action("ld2451.set_sensitivity", LD2451SetSensitivityAction, LD2451_SET_SENSITIVITY_ACTION_SCHEMA, synchronous=True)
async def ld2451_set_sensitivity_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    valid_trigs = await cg.templatable(config[CONF_VALID_TRIGS], args, cg.uint8)
    cg.add(var.set_valid_trigs(valid_trigs))

    signal_threshold = await cg.templatable(config[CONF_SIGNAL_THRESHOLD], args, cg.uint8)
    cg.add(var.set_signal_threshold(signal_threshold))

    return var

LD2451SetTargetDetectConfigAction = ld2451.class_("LD2451SetTargetDetectConfigAction", automation.Action)
LD2451_SET_TARGET_DETECT_CONFIG_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2451Component),
        cv.Required(CONF_MAX_DISTANCE): cv.templatable(cv.int_range(0x0A, 0xFF)),
        cv.Required(CONF_DIRECTION): cv.templatable(cv.enum(LD2451_DIRECTION_OPTIONS)),
        cv.Required(CONF_MIN_SPEED): cv.templatable(cv.int_range(0x00, 0x78)),
        cv.Required(CONF_NO_TARGET_DELAY): cv.templatable(cv.uint8_t),
    }
)
@automation.register_action("ld2451.set_target_detect_config", LD2451SetTargetDetectConfigAction, LD2451_SET_TARGET_DETECT_CONFIG_ACTION_SCHEMA, synchronous=True)
async def ld2451_set_target_detect_config_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    max_distance = await cg.templatable(config[CONF_MAX_DISTANCE], args, cg.uint8)
    cg.add(var.set_max_distance(max_distance))

    direction = await cg.templatable(config[CONF_DIRECTION], args, cg.uint8)
    cg.add(var.set_direction(direction))

    min_speed = await cg.templatable(config[CONF_MIN_SPEED], args, cg.uint8)
    cg.add(var.set_min_speed(min_speed))

    no_target_delay = await cg.templatable(config[CONF_NO_TARGET_DELAY], args, cg.uint8)
    cg.add(var.set_no_target_delay(no_target_delay))

    return var