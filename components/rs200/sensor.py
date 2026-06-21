import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    STATE_CLASS_MEASUREMENT, CONF_AMBIENT_LIGHT, CONF_TEMPERATURE,
    UNIT_CELSIUS,
    DEVICE_CLASS_TEMPERATURE, DEVICE_CLASS_EMPTY,
)

from . import CONF_RS200_ID, RS200Component

DEPENDENCIES = ["rs200"]

CONF_TEMPERATURE_UPDATE_INTERVAL = "temperature_update_interval"
CONF_RAW_RAIN = "raw_rain"

# 自定义 mdi 图标 (esphome.const 中没有, 但都是有效的 Material Design Icons)
ICON_WHITE_BALANCE_SUNNY = "mdi:white-balance-sunny"  # 环境光
ICON_WEATHER_RAINY = "mdi:weather-rainy"           # 雨量状态

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_RS200_ID): cv.use_id(RS200Component),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_UPDATE_INTERVAL, default="60s"): cv.update_interval,
            cv.Optional(CONF_AMBIENT_LIGHT): sensor.sensor_schema(
                unit_of_measurement="#",
                icon=ICON_WHITE_BALANCE_SUNNY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_EMPTY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_RAW_RAIN): sensor.sensor_schema(
                unit_of_measurement="#",
                icon=ICON_WEATHER_RAINY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_EMPTY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
)


async def to_code(config):
    rs200_component = await cg.get_variable(config[CONF_RS200_ID])

    if temp_config := config.get(CONF_TEMPERATURE):
        sens = await sensor.new_sensor(temp_config)
        cg.add(rs200_component.set_temperature_sensor(sens))
        cg.add(rs200_component.set_temperature_update_interval(config[CONF_TEMPERATURE_UPDATE_INTERVAL]))

    if light_config := config.get(CONF_AMBIENT_LIGHT):
        sens = await sensor.new_sensor(light_config)
        cg.add(rs200_component.set_light_sensor(sens))

    if rain_config := config.get(CONF_RAW_RAIN):
        sens = await sensor.new_sensor(rain_config)
        cg.add(rs200_component.set_raw_rain_sensor(sens))