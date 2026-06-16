import esphome.codegen as cg
from esphome import automation
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_TYPE
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

CONF_BAX_ID = "bax_id"

bax_ns = cg.esphome_ns.namespace("bax")
BAXComponent = bax_ns.class_("BAXComponent", cg.PollingComponent, uart.UARTDevice)

BAX_TYPE = bax_ns.enum("BAX_TYPE", is_class=True)
BAX_TYPE_ENUM = {
    "BA012": BAX_TYPE.BA012,  # done
    "BA022": BAX_TYPE.BA022,  # done
    "BA111": BAX_TYPE.BA111,  # done
    "BA121": BAX_TYPE.BA121,  # done
    "BA234": BAX_TYPE.BA234,  # done
    "BA311": BAX_TYPE.BA311,  # done
    "BAT3U": BAX_TYPE.BAT3U, # done
}

CONF_SALINITY = "salinity"
CONF_RESISTANCE = "resistance"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BAXComponent),
            cv.Required(CONF_TYPE): cv.enum(BAX_TYPE_ENUM),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(uart.UART_DEVICE_SCHEMA),
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "bax",
    require_tx=True,
    require_rx=True,
    parity=None,
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_type_(config[CONF_TYPE]))

# aka base line calibrate
ZeroPointCalibrateAction = bax_ns.class_("ZeroPointCalibrateAction", automation.Action)
ZERO_POINT_CALIBRATE_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(BAXComponent),
    }
)
@automation.register_action("bax.zero_point_calibrate", ZeroPointCalibrateAction, ZERO_POINT_CALIBRATE_ACTION_SCHEMA, synchronous=True)
async def zero_point_calibrate_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

TwiceCalibrateAction = bax_ns.class_("TwiceCalibrateAction", automation.Action)
TWICE_CALIBRATE_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(BAXComponent),
        cv.Required(CONF_SALINITY): cv.templatable(cv.positive_float),
    }
)
@automation.register_action("bax.twice_calibrate", TwiceCalibrateAction, TWICE_CALIBRATE_ACTION_SCHEMA, synchronous=True)
async def twice_calibrate_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    salinity = await cg.templatable(config[CONF_SALINITY], args, cg.float_)
    cg.add(var.set_salinity(salinity))
    return var

SetNTCResistanceAction = bax_ns.class_("SetNTCResistanceAction", automation.Action)
SET_NTC_RESISTANCE_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(BAXComponent),
        cv.Required(CONF_RESISTANCE): cv.templatable(cv.uint32_t),
    }
)
@automation.register_action("bax.set_ntc_resistance", SetNTCResistanceAction, SET_NTC_RESISTANCE_ACTION_SCHEMA, synchronous=True)
async def set_ntc_resistance_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    resistance = await cg.templatable(config[CONF_RESISTANCE], args, cg.uint32)
    cg.add(var.set_resistance(resistance))
    return var

SetNTCBValueAction = bax_ns.class_("SetNTCBValueAction", automation.Action)
SET_NTC_B_VALUE_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(BAXComponent),
        cv.Required("b"): cv.templatable(cv.uint16_t),
    }
)
@automation.register_action("bax.set_ntc_b_value", SetNTCBValueAction, SET_NTC_B_VALUE_ACTION_SCHEMA, synchronous=True)
async def set_ntc_b_value_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    b_value = await cg.templatable(config["b"], args, cg.uint16)
    cg.add(var.set_b_value(b_value))
    return var