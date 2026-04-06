"""
http://file.whycan.com/files/202412/ML307/15_扩展AT用户手册_4G系列_V1.5.0.pdf
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome import automation
from esphome.const import (
    CONF_ID,
)

DEPENDENCIES = ["uart"]
MULTI_CONF = True

ml307r_ns = cg.esphome_ns.namespace("ml307r")
ML307RComponent = ml307r_ns.class_("ML307RComponent", cg.PollingComponent, uart.UARTDevice)

CONF_ML307R_ID = "ml307r_id"
CONF_CMD = "cmd"
CONF_HOST = "host"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ML307RComponent),
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("60s"))
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ml307r",
    require_tx=True,
    require_rx=True,
    parity=None,
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)


SendAtCommandAction = ml307r_ns.class_("SendAtCommandAction", automation.Action)
ML307R_SENDATCOMMAND_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ML307RComponent),
        cv.Optional(CONF_CMD, default=True): cv.templatable(cv.string),
    }
)


@automation.register_action(
    "ml307r.send_at_command", SendAtCommandAction, ML307R_SENDATCOMMAND_SCHEMA, synchronous=True
)
async def as201_set_subscribe_flag_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])  # ML307RComponent
    var = cg.new_Pvariable(action_id, template_arg, paren)  # SendAtCommandAction

    cmd = await cg.templatable(config[CONF_CMD], args, cg.std_string)
    cg.add(var.set_cmd(cmd))
    return var


PingAction = ml307r_ns.class_("PingAction", automation.Action)
ML307R_PING_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(ML307RComponent),
        cv.Optional(CONF_HOST, default="8.8.8.8"): cv.templatable(cv.string),
    }
)


@automation.register_action(
    "ml307r.ping", PingAction, ML307R_PING_SCHEMA, synchronous=True
)
async def ml307r_ping_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])  # ML307RComponent
    var = cg.new_Pvariable(action_id, template_arg, paren)  # PingAction

    host = await cg.templatable(config[CONF_HOST], args, cg.std_string)
    cg.add(var.set_host(host))
    return var
