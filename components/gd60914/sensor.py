import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_MODE,
    STATE_CLASS_MEASUREMENT,
    CONF_TEMPERATURE,
    UNIT_CELSIUS,
    DEVICE_CLASS_TEMPERATURE
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]

gd60914_ns = cg.esphome_ns.namespace("gd60914")
GD60914Component = gd60914_ns.class_("GD60914Component", cg.PollingComponent, uart.UARTDevice)

GD60914_MODE = gd60914_ns.enum("GD60914_MODE")
GD60914_MODE_OPTIONS = {
    "object": GD60914_MODE.GD60914_MODE_OBJ,
    "forehead": GD60914_MODE.GD60914_MDOE_FOREHEAD,
    "wrist": GD60914_MODE.GD60914_MDOE_WRIST,
}

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(GD60914Component),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MODE, default="object"): cv.enum(GD60914_MODE_OPTIONS)
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_mode(config[CONF_MODE]))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))

# 无参数automation
GD60914ResetAction = gd60914_ns.class_("GD60914ResetAction", automation.Action)
GD60914_RESETACTION_SCHEMA  = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(GD60914Component),
    }
)
@automation.register_action("gd60914.reset", GD60914ResetAction, GD60914_RESETACTION_SCHEMA, synchronous=True)
async def gd60914_reset_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

GD60914Calibrate35Action = gd60914_ns.class_("GD60914Calibrate35Action", automation.Action)
GD60914_CALIBRATE35ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(GD60914Component),
    }
)
@automation.register_action("gd60914.calibrate35", GD60914Calibrate35Action, GD60914_CALIBRATE35ACTION_SCHEMA, synchronous=True)
async def gd60914_calibrate35_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

GD60914Calibrate42Action = gd60914_ns.class_("GD60914Calibrate42Action", automation.Action)
GD60914_CALIBRATE42ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(GD60914Component),
    }
)
@automation.register_action("gd60914.calibrate42", GD60914Calibrate42Action, GD60914_CALIBRATE42ACTION_SCHEMA, synchronous=True)
async def gd60914_calibrate42_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
