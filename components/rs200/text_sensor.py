import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_VERSION, ENTITY_CATEGORY_DIAGNOSTIC, ICON_CHIP,
)

from . import CONF_RS200_ID, RS200Component

DEPENDENCIES = ["rs200"]

CONF_RAIN = "rain"
CONF_SYSTEM_STATUS = "system_status"

# 自定义 mdi 图标 (esphome.const 中没有, 但都是有效的 Material Design Icons)
ICON_WEATHER_RAINY = "mdi:weather-rainy"           # 雨量状态
ICON_ALERT_CIRCLE_OUTLINE = "mdi:alert-circle-outline"  # 系统状态

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_RS200_ID): cv.use_id(RS200Component),
            cv.Optional(CONF_VERSION): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon=ICON_CHIP,
            ),
            cv.Optional(CONF_RAIN): text_sensor.text_sensor_schema(
                icon=ICON_WEATHER_RAINY,
            ),
            cv.Optional(CONF_SYSTEM_STATUS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon=ICON_ALERT_CIRCLE_OUTLINE,
            ),
        }
    )
)


async def to_code(config):
    rs200_component = await cg.get_variable(config[CONF_RS200_ID])

    if version_config := config.get(CONF_VERSION):
        sens = await text_sensor.new_text_sensor(version_config)
        cg.add(rs200_component.set_version_text_sensor(sens))
    if rain_config := config.get(CONF_RAIN):
        sens = await text_sensor.new_text_sensor(rain_config)
        cg.add(rs200_component.set_rain_text_sensor(sens))
    if status_config := config.get(CONF_SYSTEM_STATUS):
        sens = await text_sensor.new_text_sensor(status_config)
        cg.add(rs200_component.set_system_status_text_sensor(sens))
