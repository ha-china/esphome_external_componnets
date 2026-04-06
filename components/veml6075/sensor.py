from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID, CONF_INTEGRATION_TIME, ICON_BRIGHTNESS_5, DEVICE_CLASS_EMPTY
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["i2c"]

veml6075 = cg.esphome_ns.namespace("veml6075")
VEML6075Component = veml6075.class_("VEML6075Component", cg.PollingComponent, i2c.I2CDevice)

UNIT_COUNTS = "#"
UNIT_UVI = "UVI"
CONF_UV_INDEX = "uv_index"
CONF_UVA = "UVA"
CONF_UVD = "UVD"  # reserved
CONF_UVB = "UVB"
CONF_UV_COMP1 = "UV_COMP1"
CONF_UV_COMP2 = "UV_COMP2"
CONF_DYNAMIC_SETTING = "dynamic_setting"
CONF_TRIGGER = "trigger"
CONF_ACTIVE_FORCE_MODE = "active_force_mode"

VEML6075IntegrationTime = veml6075.enum("VEML6075IntegrationTime")
VEML6075IntegrationTimeOptions = {
    "50ms": VEML6075IntegrationTime.VEML6075_IT_50MS,
    "100ms": VEML6075IntegrationTime.VEML6075_IT_100MS,
    "200ms": VEML6075IntegrationTime.VEML6075_IT_200MS,
    "400ms": VEML6075IntegrationTime.VEML6075_IT_400MS,
    "800ms": VEML6075IntegrationTime.VEML6075_IT_800MS,
}

VEML6075DynamicSetting = veml6075.enum("VEML6075DynamicSetting")
VEML6075DynamicSettingOptions = {
    "normal": VEML6075DynamicSetting.VEML6075_NORMAL_DYNAMIC,
    "high": VEML6075DynamicSetting.VEML6075_HIGH_DYNAMIC
}

# VEML6075Trigger = veml6075.enum("VEML6075Trigger")
# VEML6075TriggerOptions = {
#     0: VEML6075Trigger.VEML6075_TRIGGER_NONE,
#     1: VEML6075Trigger.VEML6075_TRIGGER_ONCE
# }

VEML6075ActiveForceMode = veml6075.enum("VEML6075ActiveForceMode")
VEML6075ActiveForceModeOptions = {
    "disable": VEML6075ActiveForceMode.VEML6075_ACTIVE_FORCE_MODE_DISABLE,
    "enable": VEML6075ActiveForceMode.VEML6075_ACTIVE_FORCE_MODE_ENABLE,
}

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VEML6075Component),
            cv.Optional(CONF_INTEGRATION_TIME, default="800ms"): cv.enum(VEML6075IntegrationTimeOptions),
            cv.Optional(CONF_DYNAMIC_SETTING, default="high"): cv.enum(VEML6075DynamicSettingOptions),
            # cv.Optional(CONF_TRIGGER, default=0): cv.enum(VEML6075TriggerOptions),
            cv.Optional(CONF_ACTIVE_FORCE_MODE, default="enable"): cv.enum(VEML6075ActiveForceModeOptions),

            cv.Optional(CONF_UVA): sensor.sensor_schema(
                unit_of_measurement=UNIT_COUNTS,
                icon=ICON_BRIGHTNESS_5,
                accuracy_decimals=1,
            ),
            cv.Optional(CONF_UVB): sensor.sensor_schema(
                unit_of_measurement=UNIT_COUNTS,
                icon=ICON_BRIGHTNESS_5,
                accuracy_decimals=1,
            ),
            cv.Optional(CONF_UV_INDEX): sensor.sensor_schema(
                unit_of_measurement=UNIT_UVI,
                icon=ICON_BRIGHTNESS_5,
                accuracy_decimals=5,
                device_class=DEVICE_CLASS_EMPTY,
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(0x10)),
)

FINAL_VALIDATE_SCHEMA = i2c.final_validate_device_schema("veml6075", max_frequency="400khz")


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_it(config[CONF_INTEGRATION_TIME]))
    cg.add(var.set_dynamic_setting(config[CONF_DYNAMIC_SETTING]))
    cg.add(var.set_active_force_mode(config[CONF_ACTIVE_FORCE_MODE]))
    if CONF_UVA in config:
        sens = await sensor.new_sensor(config[CONF_UVA])
        cg.add(var.set_uva_sensor(sens))
    if CONF_UVB in config:
        sens = await sensor.new_sensor(config[CONF_UVB])
        cg.add(var.set_uvb_sensor(sens))
    if CONF_UV_INDEX in config:
        sens = await sensor.new_sensor(config[CONF_UV_INDEX])
        cg.add(var.set_uvi_sensor(sens))

VEML6075SetCoefficientsAction = veml6075.class_("VEML6075SetCoefficientsAction", automation.Action)
VEML6075_SET_COEFFICIENTS_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(VEML6075Component),
        cv.Required("UVA_A"): cv.templatable(cv.float_),
        cv.Required("UVA_B"): cv.templatable(cv.float_),
        cv.Required("UVB_C"): cv.templatable(cv.float_),
        cv.Required("UVB_D"): cv.templatable(cv.float_),
        cv.Required("UVA_response"): cv.templatable(cv.float_),
        cv.Required("UVB_response"): cv.templatable(cv.float_),
    }
)
@automation.register_action("veml6075.set_coefficients", VEML6075SetCoefficientsAction, VEML6075_SET_COEFFICIENTS_ACTION_SCHEMA, synchronous=True)
async def veml6075_set_coefficients_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    UVA_A = await cg.templatable(config["UVA_A"], args, cg.float_)
    cg.add(var.set_uva_a(UVA_A))
    UVA_B = await cg.templatable(config["UVA_B"], args, cg.float_)
    cg.add(var.set_uva_b(UVA_B))
    UVB_C = await cg.templatable(config["UVB_C"], args, cg.float_)
    cg.add(var.set_uvb_c(UVB_C))
    UVB_D = await cg.templatable(config["UVB_D"], args, cg.float_)
    cg.add(var.set_uvb_d(UVB_D))
    UVA_response = await cg.templatable(config["UVA_response"], args, cg.float_)
    cg.add(var.set_uva_response(UVA_response))
    UVB_response = await cg.templatable(config["UVB_response"], args, cg.float_)
    cg.add(var.set_uvb_response(UVB_response))
    return var