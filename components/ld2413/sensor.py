import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_UPDATE_INTERVAL,
    CONF_DISTANCE,
    STATE_CLASS_MEASUREMENT, UNIT_MILLIMETER, DEVICE_CLASS_DISTANCE
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]
CONF_MAX_DISTANCE = "max_distance"
CONF_MIN_DISTANCE = "min_distance"

ld2413 = cg.esphome_ns.namespace("ld2413")
LD2413Component = ld2413.class_("LD2413Component", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LD2413Component),
            cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIMETER,
                device_class=DEVICE_CLASS_DISTANCE,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_DISTANCE, default=10500): cv.positive_int,
            cv.Optional(CONF_MIN_DISTANCE, default=0): cv.positive_int,
            cv.Optional(CONF_UPDATE_INTERVAL, default="20s"): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld2413",
    baud_rate=115200,
    parity=None,
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_max_distance_attr(config[CONF_MAX_DISTANCE]))
    cg.add(var.set_min_distance_attr(config[CONF_MIN_DISTANCE]))
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    if CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))


LD2413UpdateDoorLimitAction = ld2413.class_("LD2413UpdateDoorLimitAction", automation.Action)
LD2413_UPDATE_DOOR_LIMIT_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2413Component),
    }
)
@automation.register_action("ld2413.update_door_limit", LD2413UpdateDoorLimitAction, LD2413_UPDATE_DOOR_LIMIT_ACTION_SCHEMA, synchronous=True)
async def ld2413_update_door_limit_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2413EnableConfigAction = ld2413.class_("LD2413EnableConfigAction", automation.Action)
LD2413_ENABLE_CONFIG_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2413Component),
    }
)
@automation.register_action("ld2413.enable_config", LD2413EnableConfigAction, LD2413_ENABLE_CONFIG_ACTION_SCHEMA, synchronous=True)
async def ld2413_enable_config_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2413DisableConfigAction = ld2413.class_("LD2413DisableConfigAction", automation.Action)
LD2413_DISABLE_CONFIG_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2413Component),
    }
)
@automation.register_action("ld2413.disable_config", LD2413DisableConfigAction, LD2413_DISABLE_CONFIG_ACTION_SCHEMA, synchronous=True)
async def ld2413_disable_config_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

LD2413SetMaxDistanceAction = ld2413.class_("LD2413SetMaxDistanceAction", automation.Action)
LD2413_SET_MAX_DISTANCE_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2413Component),
        cv.Required(CONF_MAX_DISTANCE): cv.templatable(cv.positive_int),
    }
)
@automation.register_action("ld2413.set_max_distance", LD2413SetMaxDistanceAction, LD2413_SET_MAX_DISTANCE_ACTION_SCHEMA, synchronous=True)
async def ld2413_set_max_distance_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    max_distance = await cg.templatable(config[CONF_MAX_DISTANCE], args, cg.uint16)
    cg.add(var.set_max_distance(max_distance))
    return var

LD2413SetMinDistanceAction = ld2413.class_("LD2413SetMinDistanceAction", automation.Action)
LD2413_SET_MIN_DISTANCE_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2413Component),
        cv.Required(CONF_MIN_DISTANCE): cv.templatable(cv.positive_int),
    }
)
@automation.register_action("ld2413.set_min_distance", LD2413SetMinDistanceAction, LD2413_SET_MIN_DISTANCE_ACTION_SCHEMA, synchronous=True)
async def ld2413_set_min_distance_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    min_distance = await cg.templatable(config[CONF_MIN_DISTANCE], args, cg.uint16)
    cg.add(var.set_min_distance(min_distance))
    return var

LD2413SetReportIntervalAction = ld2413.class_("LD2413SetReportIntervalAction", automation.Action)
LD2413_SET_REPORT_INTERVAL_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(LD2413Component),
        cv.Required(CONF_UPDATE_INTERVAL): cv.templatable(cv.positive_time_period_milliseconds),
    }
)
@automation.register_action("ld2413.set_report_interval", LD2413SetReportIntervalAction, LD2413_SET_REPORT_INTERVAL_ACTION_SCHEMA, synchronous=True)
async def ld2413_set_report_interval_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    update_interval = await cg.templatable(config[CONF_UPDATE_INTERVAL], args, cg.uint16)
    cg.add(var.set_update_interval(update_interval))
    return var