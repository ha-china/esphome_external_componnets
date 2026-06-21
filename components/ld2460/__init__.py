import esphome.codegen as cg
from esphome import automation
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_ANGLE,
    CONF_SENSITIVITY,
    CONF_HEIGHT, CONF_MODE, CONF_ON_DATA, CONF_TRIGGER_ID
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

CONF_LD2460_ID = "ld2460_id"

ld2460_ns = cg.esphome_ns.namespace("ld2460")
LD2460Component = ld2460_ns.class_("LD2460Component", cg.Component, uart.UARTDevice)

MAX_TARGETS = 5

CONF_DETECT_DISTANCE = "detect_distance"
CONF_DETECT_START_ANGLE = "detect_start_angle"
CONF_DETECT_END_ANGLE = "detect_end_angle"
CONF_UPLOAD = "upload"

LD2460DataTrigger = ld2460_ns.class_("LD2460DataTrigger", automation.Trigger.template())

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LD2460Component),
            cv.Optional(CONF_HEIGHT, default=2.6): cv.positive_float,
            cv.Optional(CONF_ANGLE, default=30): cv.positive_float,
            cv.Optional(CONF_MODE, default="Side"): cv.string,
            cv.Optional(CONF_DETECT_DISTANCE, default=6): cv.positive_float,
            cv.Optional(CONF_DETECT_START_ANGLE, default=-45): cv.float_,
            cv.Optional(CONF_DETECT_END_ANGLE, default=45): cv.float_,
            cv.Optional(CONF_SENSITIVITY, default="High"): cv.string,
            cv.Optional(CONF_ON_DATA): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(LD2460DataTrigger),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA),
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld2460",
    require_tx=True,
    require_rx=True,
    parity=None,
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_height_(config[CONF_HEIGHT]))
    cg.add(var.set_angle_(config[CONF_ANGLE]))
    cg.add(var.set_mode_(config[CONF_MODE]))
    cg.add(var.set_detect_distance_(config[CONF_DETECT_DISTANCE]))
    cg.add(var.set_detect_start_angle_(config[CONF_DETECT_START_ANGLE]))
    cg.add(var.set_detect_end_angle_(config[CONF_DETECT_END_ANGLE]))
    cg.add(var.set_sensitivity_(config[CONF_SENSITIVITY]))
    for conf in config.get(CONF_ON_DATA, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)


LD2460EnableUploadAction = ld2460_ns.class_("LD2460EnableUploadAction", automation.Action)
LD2460_ENABLE_UPLOADACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2460Component),
        cv.Required(CONF_UPLOAD): cv.templatable(cv.boolean),
    }
)
@automation.register_action("ld2460.enable_upload", LD2460EnableUploadAction, LD2460_ENABLE_UPLOADACTION_SCHEMA, synchronous=True)
async def ld2460_enable_upload_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    upload = await cg.templatable(config[CONF_UPLOAD], args, cg.bool_)
    cg.add(var.set_upload(upload))
    return var

LD2460SetInstallParamsAction = ld2460_ns.class_("LD2460SetInstallParamsAction", automation.Action)
LD2460_SET_INSTALL_PARAMS_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2460Component),
        cv.Required(CONF_HEIGHT): cv.templatable(cv.positive_float),
        cv.Required(CONF_ANGLE): cv.templatable(cv.positive_float),
    }
)
@automation.register_action("ld2460.set_install_params", LD2460SetInstallParamsAction, LD2460_SET_INSTALL_PARAMS_ACTION_SCHEMA, synchronous=True)
async def ld2460_set_install_params_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    height = await cg.templatable(config[CONF_HEIGHT], args, cg.float_)
    cg.add(var.set_height(height))
    angle = await cg.templatable(config[CONF_ANGLE], args, cg.float_)
    cg.add(var.set_angle(angle))
    return var

LD2460RestartAction = ld2460_ns.class_("LD2460RestartAction", automation.Action)
LD2460_RESTART_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2460Component),
    }
)
@automation.register_action("ld2460.restart", LD2460RestartAction, LD2460_RESTART_ACTION_SCHEMA, synchronous=True)
async def ld2460_restart_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2460FactoryResetAction = ld2460_ns.class_("LD2460FactoryResetAction", automation.Action)
LD2460_FACTORY_RESET_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2460Component),
    }
)
@automation.register_action("ld2460.factory_reset", LD2460FactoryResetAction, LD2460_FACTORY_RESET_ACTION_SCHEMA, synchronous=True)
async def ld2460_factory_reset_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2460SetDetectRangeAction = ld2460_ns.class_("LD2460SetDetectRangeAction", automation.Action)
LD2460_SET_DETECT_RANGE_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2460Component),
        cv.Required(CONF_DETECT_DISTANCE): cv.templatable(cv.positive_float),
        cv.Required(CONF_DETECT_START_ANGLE): cv.templatable(cv.float_),
        cv.Required(CONF_DETECT_END_ANGLE): cv.templatable(cv.float_),
    }
)
@automation.register_action("ld2460.set_detect_range", LD2460SetDetectRangeAction, LD2460_SET_DETECT_RANGE_ACTION_SCHEMA, synchronous=True)
async def ld2460_set_detect_range_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    detect_distance = await cg.templatable(config[CONF_DETECT_DISTANCE], args, cg.float_)
    cg.add(var.set_detect_distance(detect_distance))
    detect_start_angle = await cg.templatable(config[CONF_DETECT_START_ANGLE], args, cg.float_)
    cg.add(var.set_detect_start_angle(detect_start_angle))
    detect_end_angle = await cg.templatable(config[CONF_DETECT_END_ANGLE], args, cg.float_)
    cg.add(var.set_detect_end_angle(detect_end_angle))
    return var