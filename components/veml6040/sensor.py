from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID, CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, CONF_INTEGRATION_TIME, CONF_MODE,
    ICON_BRIGHTNESS_5, UNIT_LUX, DEVICE_CLASS_ILLUMINANCE
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["i2c"]

CONF_TRIGGER = "trigger"

veml6040 = cg.esphome_ns.namespace("veml6040")
VEML6040Component = veml6040.class_("VEML6040Component", cg.PollingComponent, i2c.I2CDevice)

VEML6040_INTEGRATION_TIME = veml6040.enum("VEML6040_INTEGRATION_TIME")
VEML6040_INTEGRATION_TIME_OPTIONS = {
    "40ms": VEML6040_INTEGRATION_TIME.VEML6040_INTEGRATION_TIME_40MS,
    "80ms": VEML6040_INTEGRATION_TIME.VEML6040_INTEGRATION_TIME_80MS,
    "160ms": VEML6040_INTEGRATION_TIME.VEML6040_INTEGRATION_TIME_160MS,
    "320ms": VEML6040_INTEGRATION_TIME.VEML6040_INTEGRATION_TIME_320MS,
    "640ms": VEML6040_INTEGRATION_TIME.VEML6040_INTEGRATION_TIME_640MS,
    "1280ms": VEML6040_INTEGRATION_TIME.VEML6040_INTEGRATION_TIME_1280MS,
}

VEML6040_MODE = veml6040.enum("VEML6040_MODE")
VEML6040_MODE_OPTIONS = {
    "auto": VEML6040_MODE.VEML6040_MODE_AUTO,
    "force": VEML6040_MODE.VEML6040_MODE_FORCE,
}

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VEML6040Component),
            cv.Optional(CONF_RED): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                icon=ICON_BRIGHTNESS_5,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ILLUMINANCE,
            ),
            cv.Optional(CONF_GREEN): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                icon=ICON_BRIGHTNESS_5,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ILLUMINANCE,
            ),
            cv.Optional(CONF_BLUE): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                icon=ICON_BRIGHTNESS_5,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ILLUMINANCE,
            ),
            cv.Optional(CONF_WHITE): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                icon=ICON_BRIGHTNESS_5,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ILLUMINANCE,
            ),
            cv.Optional(CONF_INTEGRATION_TIME, default="80ms"): cv.enum(VEML6040_INTEGRATION_TIME_OPTIONS),
            cv.Optional(CONF_TRIGGER, default=False): cv.boolean,
            cv.Optional(CONF_MODE, default="auto"): cv.enum(VEML6040_MODE_OPTIONS),

        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(0x08)),
)

FINAL_VALIDATE_SCHEMA = i2c.final_validate_device_schema("veml6040", max_frequency="400khz")


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_RED in config:
        sens = await sensor.new_sensor(config[CONF_RED])
        cg.add(var.set_red_sensor(sens))
    if CONF_GREEN in config:
        sens = await sensor.new_sensor(config[CONF_GREEN])
        cg.add(var.set_green_sensor(sens))
    if CONF_BLUE in config:
        sens = await sensor.new_sensor(config[CONF_BLUE])
        cg.add(var.set_blue_sensor(sens))
    if CONF_WHITE in config:
        sens = await sensor.new_sensor(config[CONF_WHITE])
        cg.add(var.set_white_sensor(sens))
    cg.add(var.set_integration_time(config[CONF_INTEGRATION_TIME]))
    cg.add(var.set_trig(config[CONF_TRIGGER]))
    cg.add(var.set_mode(config[CONF_MODE]))


VEML6040ShutdownAction = veml6040.class_("VEML6040ShutdownAction", automation.Action)
VEML6040_SHUTDOWNACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(VEML6040Component),
    }
)


@automation.register_action("veml6040.shutdown", VEML6040ShutdownAction, VEML6040_SHUTDOWNACTION_SCHEMA, synchronous=True)
async def veml6040_shutdown_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


VEML6040WakeupAction = veml6040.class_("VEML6040WakeupAction", automation.Action)
VEML6040_WAKEUPACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(VEML6040Component),
    }
)


@automation.register_action("veml6040.wakeup", VEML6040WakeupAction, VEML6040_WAKEUPACTION_SCHEMA, synchronous=True)
async def veml6040_wakeup_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
