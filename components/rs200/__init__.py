import esphome.codegen as cg
from esphome import automation
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

CONF_RS200_ID = "rs200_id"

rs200_ns = cg.esphome_ns.namespace("rs200")
RS200Component = rs200_ns.class_("RS200Component", cg.Component, uart.UARTDevice)

# 帧标识: 数据编号 (低7位), 与 C++ 端 RS200Tag 枚举一一对应, 见数据手册 表 2.4
RS200_TAG_VERSION = 0
RS200_TAG_RAIN = 1
RS200_TAG_SYSTEM_STATUS = 2
RS200_TAG_OPTICAL = 3
RS200_TAG_REALTIME_RAIN = 4
RS200_TAG_OUTPUT_FREQ = 5
RS200_TAG_V1 = 6
RS200_TAG_V2 = 7
RS200_TAG_V3 = 8
RS200_TAG_S1 = 9
RS200_TAG_S2 = 10
RS200_TAG_S3 = 11
RS200_TAG_N1 = 12
RS200_TAG_N2 = 13
RS200_TAG_N3 = 14
RS200_TAG_LIGHT = 15
RS200_TAG_TEMPERATURE = 16
RS200_TAG_SLEEP = 17
RS200_TAG_BOOT = 18


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(RS200Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA),
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "rs200",
    baud_rate=115200,
    require_tx=True,
    require_rx=True,
    parity=None,
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

SendCommandAction = rs200_ns.class_("SendCommandAction", automation.Action)
SEND_COMMAND_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(RS200Component),
        cv.Required("write"): cv.templatable(cv.boolean),
        cv.Required("tag"): cv.templatable(cv.uint8_t),
        cv.Required("data"): cv.templatable(cv.uint16_t),
    }
)
@automation.register_action("rs200.send_command", SendCommandAction, SEND_COMMAND_ACTION_SCHEMA, synchronous=True)
async def send_command_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    write = await cg.templatable(config["write"], args, cg.bool_)
    cg.add(var.set_write(write))
    tag = await cg.templatable(config["tag"], args, cg.uint8)
    cg.add(var.set_tag(tag))
    data = await cg.templatable(config["data"], args, cg.uint16)
    cg.add(var.set_data(data))
    return var
