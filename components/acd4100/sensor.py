from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
    DEVICE_CLASS_CARBON_DIOXIDE, UNIT_CELSIUS, DEVICE_CLASS_TEMPERATURE, CONF_MODE,
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["i2c"]

CONF_BASE = "base"
CONF_R32 = "r32" # 冷媒气体
ICON_AC = "mdi:air-conditioner"

acd4100 = cg.esphome_ns.namespace("acd4100")
ACD4100Component = acd4100.class_("ACD4100Component", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ACD4100Component),
            cv.Optional(CONF_R32): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                icon=ICON_AC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BASE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CARBON_DIOXIDE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(0x2A)),
)

FINAL_VALIDATE_SCHEMA = i2c.final_validate_device_schema("acd4100", max_frequency="100khz")


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_R32 in config:
        sens = await sensor.new_sensor(config[CONF_R32])
        cg.add(var.set_r32_sensor(sens))
    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))
    if CONF_BASE in config:
        sens = await sensor.new_sensor(config[CONF_BASE])
        cg.add(var.set_base_sensor(sens))

ACD4100SetCalibrateModeAction = acd4100.class_("ACD4100SetCalibrateModeAction", automation.Action)
ACD4100_SETCALIBRATEACTIONMODE_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ACD4100Component),
        cv.Required(CONF_MODE): cv.templatable(cv.boolean),
    }
)
@automation.register_action("acd4100.set_calibrate_mode", ACD4100SetCalibrateModeAction, ACD4100_SETCALIBRATEACTIONMODE_SCHEMA, synchronous=True)
async def acd4100_calibrate_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    mode = await cg.templatable(config[CONF_MODE], args, cg.bool_)
    cg.add(var.set_mode(mode))
    return var

ACD4100CalibrateAction = acd4100.class_("ACD4100CalibrateAction", automation.Action)
ACD4100_CALIBRATEACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ACD4100Component),
        cv.Required(CONF_BASE): cv.templatable(cv.positive_int),
    }
)
@automation.register_action("acd4100.calibrate", ACD4100CalibrateAction, ACD4100_CALIBRATEACTION_SCHEMA, synchronous=True)
async def acd4100_calibrate_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    base = await cg.templatable(config[CONF_BASE], args, cg.uint16)
    cg.add(var.set_base(base))
    return var

ACD4100ResetAction = acd4100.class_("ACD4100ResetAction", automation.Action)
ACD4100_RESETACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ACD4100Component),
    }
)
@automation.register_action("acd4100.reset", ACD4100ResetAction, ACD4100_RESETACTION_SCHEMA, synchronous=True)
async def acd4100_reset_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)