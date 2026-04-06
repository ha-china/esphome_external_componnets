import esphome.codegen as cg
from esphome import automation
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_ID,
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

CONF_AS201_ID = "as201_id"

as201 = cg.esphome_ns.namespace("as201")
AS201Component = as201.class_("AS201Component", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS201Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA),
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "as201",
    require_tx=True,
    require_rx=True,
    parity=None,
    stop_bits=1,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

GetDataOnceAction = as201.class_("GetDataOnceAction", automation.Action)
AS201_GETDATAONCE_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(AS201Component),
    }
)
@automation.register_action(
    "as201.get_data_once", GetDataOnceAction, AS201_GETDATAONCE_SCHEMA, synchronous=True
)
async def as201_get_data_once_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID]) # AS201Component
    var = cg.new_Pvariable(action_id, template_arg, paren) # GetDataOnceAction
    return var

CalibrateAction = as201.class_("CalibrateAction", automation.Action)
AS201_CALIBRATE_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(AS201Component),
    }
)
@automation.register_action(
    "as201.calibrate", CalibrateAction, AS201_CALIBRATE_SCHEMA, synchronous=True
)
async def as201_calibrate_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID]) # AS201Component
    var = cg.new_Pvariable(action_id, template_arg, paren) # CalibrateAction
    return var

StartCalibrateMagneticFieldAction = as201.class_("StartCalibrateMagneticFieldAction", automation.Action)
AS201_STARTCALIBRATEMAGNETICFIELD_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(AS201Component),
    }
)
@automation.register_action(
    "as201.start_calibrate_magnetic_field", StartCalibrateMagneticFieldAction, AS201_STARTCALIBRATEMAGNETICFIELD_SCHEMA, synchronous=True
)
async def as201_start_calibrate_magnetic_field_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID]) # AS201Component
    var = cg.new_Pvariable(action_id, template_arg, paren) # StartCalibrateMagneticFieldAction
    return var

FinishCalibrateMagneticFieldAction = as201.class_("FinishCalibrateMagneticFieldAction", automation.Action)
AS201_FINISHCALIBRATEMAGNETICFIELD_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(AS201Component),
    }
)
@automation.register_action(
    "as201.finish_calibrate_magnetic_field", FinishCalibrateMagneticFieldAction, AS201_FINISHCALIBRATEMAGNETICFIELD_SCHEMA, synchronous=True
)
async def as201_finish_calibrate_magnetic_field_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID]) # AS201Component
    var = cg.new_Pvariable(action_id, template_arg, paren) # FinishCalibrateMagneticFieldAction
    return var

SetSubscribeFlagAction = as201.class_("SetSubscribeFlagAction", automation.Action)
AS201_SETSUBSCRIBEFLAG_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(AS201Component),
        cv.Optional("accel", default=True): cv.templatable(cv.boolean),
        cv.Optional("gyro", default=True): cv.templatable(cv.boolean),
        cv.Optional("angle", default=True): cv.templatable(cv.boolean),
        cv.Optional("field_strength", default=True): cv.templatable(cv.boolean),
        cv.Optional("quaternion", default=True): cv.templatable(cv.boolean),
        cv.Optional("temperature", default=True): cv.templatable(cv.boolean),
        cv.Optional("pressure", default=True): cv.templatable(cv.boolean),
        cv.Optional("height", default=True): cv.templatable(cv.boolean),
    }
)
@automation.register_action(
    "as201.set_subscribe_flag", SetSubscribeFlagAction, AS201_SETSUBSCRIBEFLAG_SCHEMA, synchronous=True
)
async def as201_set_subscribe_flag_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID]) # AS201Component
    var = cg.new_Pvariable(action_id, template_arg, paren) # SetSubscribeFlagAction

    accel = await cg.templatable(config["accel"], args, cg.bool_)
    cg.add(var.set_accel(accel))
    gyro = await cg.templatable(config["gyro"], args, cg.bool_)
    cg.add(var.set_gyro(gyro))
    angle = await cg.templatable(config["angle"], args, cg.bool_)
    cg.add(var.set_angle(angle))
    field_strength = await cg.templatable(config["field_strength"], args, cg.bool_)
    cg.add(var.set_field_strength(field_strength))
    quaternion = await cg.templatable(config["quaternion"], args, cg.bool_)
    cg.add(var.set_quaternion(quaternion))
    temperature = await cg.templatable(config["temperature"], args, cg.bool_)
    cg.add(var.set_temperature(temperature))
    pressure = await cg.templatable(config["pressure"], args, cg.bool_)
    cg.add(var.set_pressure(pressure))
    height = await cg.templatable(config["height"], args, cg.bool_)
    cg.add(var.set_height(height))
    return var