import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_RESTART,
    ENTITY_CATEGORY_CONFIG,
    ICON_RESTART,
)

from .. import CONF_BAX_ID, BAXComponent, bax_ns

DEPENDENCIES = ["bax"]

ZeroPointCalibrateButton = bax_ns.class_("ZeroPointCalibrateButton", button.Button)

CONF_ZERO_POINT_CALIBRATE = "zero_point_calibrate"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_BAX_ID): cv.use_id(BAXComponent),
    cv.Optional(CONF_ZERO_POINT_CALIBRATE): button.button_schema(
        ZeroPointCalibrateButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_RESTART,
    ),

}


async def to_code(config):
    bax_component = await cg.get_variable(config[CONF_BAX_ID])

    if zero_point_calibrate_config := config.get(CONF_ZERO_POINT_CALIBRATE):
        b = await button.new_button(zero_point_calibrate_config)
        await cg.register_parented(b, config[CONF_BAX_ID])
        cg.add(bax_component.set_zero_point_calibrate_button(b))
