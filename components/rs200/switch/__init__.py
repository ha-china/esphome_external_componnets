import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_SWITCH,
    ENTITY_CATEGORY_CONFIG,
)

from .. import CONF_RS200_ID, RS200Component, rs200_ns
from .. import (
    RS200_TAG_SLEEP,
    RS200_TAG_REALTIME_RAIN,
    RS200_TAG_LIGHT,
)

DEPENDENCIES = ["rs200"]

ModeSwitch = rs200_ns.class_("ModeSwitch", switch.Switch)

CONF_SLEEP_MODE = "sleep_mode"
CONF_REALTIME_RAIN_MODE = "realtime_rain_mode"
CONF_AMBIENT_LIGHT_MODE = "ambient_light_mode"

# 自定义 mdi 图标 (esphome.const 中没有, 但都是有效的 Material Design Icons)
ICON_POWER_SLEEP = "mdi:power-sleep"            # 光学睡眠模式
ICON_WEATHER_POURING = "mdi:weather-pouring"     # 实时雨量模式
ICON_WHITE_BALANCE_SUNNY = "mdi:white-balance-sunny"  # 环境光测量模式

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_RS200_ID): cv.use_id(RS200Component),
        cv.Optional(CONF_SLEEP_MODE): switch.switch_schema(
            ModeSwitch,
            device_class=DEVICE_CLASS_SWITCH,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_POWER_SLEEP,
        ),
        cv.Optional(CONF_REALTIME_RAIN_MODE): switch.switch_schema(
            ModeSwitch,
            device_class=DEVICE_CLASS_SWITCH,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_WEATHER_POURING,
        ),
        cv.Optional(CONF_AMBIENT_LIGHT_MODE): switch.switch_schema(
            ModeSwitch,
            device_class=DEVICE_CLASS_SWITCH,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_WHITE_BALANCE_SUNNY,
        ),
    }
)


async def to_code(config):
    rs200_component = await cg.get_variable(config[CONF_RS200_ID])

    if cfg := config.get(CONF_SLEEP_MODE):
        s = await switch.new_switch(cfg, RS200_TAG_SLEEP, False)
        await cg.register_parented(s, config[CONF_RS200_ID])
        cg.add(s.init())
        cg.add(rs200_component.set_sleep_mode_switch(s))

    if cfg := config.get(CONF_REALTIME_RAIN_MODE):
        s = await switch.new_switch(cfg, RS200_TAG_REALTIME_RAIN, False)
        await cg.register_parented(s, config[CONF_RS200_ID])
        cg.add(s.init())
        cg.add(rs200_component.set_realtime_rain_mode_switch(s))

    if cfg := config.get(CONF_AMBIENT_LIGHT_MODE):
        s = await switch.new_switch(cfg, RS200_TAG_LIGHT, False)
        await cg.register_parented(s, config[CONF_RS200_ID])
        cg.add(s.init())
        cg.add(rs200_component.set_ambient_light_mode_switch(s))
