from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID,
    CONF_HUMIDITY,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    DEVICE_CLASS_HUMIDITY,
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["i2c"]

ash01ib = cg.esphome_ns.namespace("ash01ib")
ASH01IBComponent = ash01ib.class_("ASH01IBComponent", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ASH01IBComponent),
            cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_HUMIDITY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(0x15)),
)
# no unit / true value

FINAL_VALIDATE_SCHEMA = i2c.final_validate_device_schema("ash01ib", max_frequency="400khz")


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_HUMIDITY in config:
        sens = await sensor.new_sensor(config[CONF_HUMIDITY])
        cg.add(var.set_humidity_sensor(sens))

ASH01IBStartMeasurementAction = ash01ib.class_("ASH01IBStartMeasurementAction", automation.Action)
ASH01IB_START_MEASUREMENT_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ASH01IBComponent),
    }
)
@automation.register_action("ash01ib.start_measurement", ASH01IBStartMeasurementAction, ASH01IB_START_MEASUREMENT_ACTION_SCHEMA, synchronous=True)
async def ash01ib_start_measurement_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

ASH01IBStopMeasurementAction = ash01ib.class_("ASH01IBStopMeasurementAction", automation.Action)
ASH01IB_STOP_MEASUREMENT_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ASH01IBComponent),
    }
)
@automation.register_action("ash01ib.stop_measurement", ASH01IBStopMeasurementAction, ASH01IB_STOP_MEASUREMENT_ACTION_SCHEMA, synchronous=True)
async def ash01ib_stop_measurement_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

ASH01IBStartCalibrationAction = ash01ib.class_("ASH01IBStartCalibrationAction", automation.Action)
ASH01IB_START_CALIBRATION_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ASH01IBComponent),
    }
)
@automation.register_action("ash01ib.start_calibration", ASH01IBStartCalibrationAction, ASH01IB_START_CALIBRATION_ACTION_SCHEMA, synchronous=True)
async def ash01ib_start_calibration_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

ASH01IBStopCalibrationAction = ash01ib.class_("ASH01IBStopCalibrationAction", automation.Action)
ASH01IB_STOP_CALIBRATION_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ASH01IBComponent),
    }
)
@automation.register_action("ash01ib.stop_calibration", ASH01IBStopCalibrationAction, ASH01IB_STOP_CALIBRATION_ACTION_SCHEMA, synchronous=True)
async def ash01ib_stop_calibration_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)