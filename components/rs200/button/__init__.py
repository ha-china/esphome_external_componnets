import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
)

from .. import CONF_RS200_ID, RS200Component, rs200_ns

DEPENDENCIES = ["rs200"]

CONF_CALIBRATE = "calibrate"

CalibrateButton = rs200_ns.class_("CalibrateButton", button.Button)

# 自定义 mdi 图标 (esphome.const 中没有, 但都是有效的 Material Design Icons)
ICON_AUTO_FIX = "mdi:auto-fix"  # 光学校准触发

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_RS200_ID): cv.use_id(RS200Component),
        cv.Optional(CONF_CALIBRATE): button.button_schema(
            CalibrateButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_AUTO_FIX,
        ),
    }
)


async def to_code(config):
    rs200_component = await cg.get_variable(config[CONF_RS200_ID])

    if cfg := config.get(CONF_CALIBRATE):
        b = await button.new_button(cfg)
        await cg.register_parented(b, config[CONF_RS200_ID])
        cg.add(rs200_component.set_calibrate_button(b))
