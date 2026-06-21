import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    UNIT_MILLISECOND,
    ICON_TIMER,
    ICON_GRAIN,
    ICON_COUNTER,
)

from .. import CONF_RS200_ID, RS200Component, rs200_ns
from .. import (
    RS200_TAG_V1, RS200_TAG_V2, RS200_TAG_V3,
    RS200_TAG_S1, RS200_TAG_S2, RS200_TAG_S3,
    RS200_TAG_N1, RS200_TAG_N2, RS200_TAG_N3,
    RS200_TAG_OPTICAL,
)

DEPENDENCIES = ["rs200"]

CONF_OUTPUT_INTERVAL = "output_interval"
CONF_V1 = "v1"
CONF_V2 = "v2"
CONF_V3 = "v3"
CONF_S1 = "s1"
CONF_S2 = "s2"
CONF_S3 = "s3"
CONF_N1 = "n1"
CONF_N2 = "n2"
CONF_N3 = "n3"
CONF_OPTICAL_CALIBRATION = "optical_calibration"

OutputIntervalNumber = rs200_ns.class_("OutputIntervalNumber", number.Number)
ThresholdNumber = rs200_ns.class_("ThresholdNumber", number.Number)

# 自定义 mdi 图标 (esphome.const 中没有, 但都是有效的 Material Design Icons)
ICON_WAVES = "mdi:waves"        # V 参数: 玻璃表面雨滴流动活跃程度
ICON_TUNE = "mdi:tune"          # 光学校准值

# V 参数(雨滴流动活跃度) -> waves, S 参数(分布不均匀度) -> grain, N 参数(判定次数) -> counter
# 输出频率 -> timer
CONFIG_SCHEMA = cv.All(cv.Schema(
    {
        cv.GenerateID(CONF_RS200_ID): cv.use_id(RS200Component),
        cv.Optional(CONF_OUTPUT_INTERVAL): number.number_schema(
            OutputIntervalNumber,
            unit_of_measurement=UNIT_MILLISECOND,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_TIMER,
        ),
        cv.Optional(CONF_V1): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_WAVES,
        ),
        cv.Optional(CONF_V2): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_WAVES,
        ),
        cv.Optional(CONF_V3): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_WAVES,
        ),
        cv.Optional(CONF_S1): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_GRAIN,
        ),
        cv.Optional(CONF_S2): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_GRAIN,
        ),
        cv.Optional(CONF_S3): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_GRAIN,
        ),
        cv.Optional(CONF_N1): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_COUNTER,
        ),
        cv.Optional(CONF_N2): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_COUNTER,
        ),
        cv.Optional(CONF_N3): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_COUNTER,
        ),
        cv.Optional(CONF_OPTICAL_CALIBRATION): number.number_schema(
            ThresholdNumber,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon=ICON_TUNE,
        ),
    }
))


async def _register_threshold(config, key, tag, setter):
    if cfg := config.get(key):
        n = await number.new_number(
            cfg,
            tag,
            min_value=1 if key.startswith("n") else 0,  # N 参数范围 1~10, 其余 0~65535
            max_value=10 if key.startswith("n") else 65535,
            step=1,
        )
        await cg.register_parented(n, config[CONF_RS200_ID])
        cg.add(setter(n))


async def to_code(config):
    rs200_component = await cg.get_variable(config[CONF_RS200_ID])

    if output_interval_config := config.get(CONF_OUTPUT_INTERVAL):
        n = await number.new_number(
            output_interval_config,
            min_value=0,
            max_value=450,
            step=50,
        )
        await cg.register_parented(n, config[CONF_RS200_ID])
        cg.add(rs200_component.set_output_interval_number(n))

    await _register_threshold(config, CONF_V1, RS200_TAG_V1, rs200_component.set_v1_number)
    await _register_threshold(config, CONF_V2, RS200_TAG_V2, rs200_component.set_v2_number)
    await _register_threshold(config, CONF_V3, RS200_TAG_V3, rs200_component.set_v3_number)
    await _register_threshold(config, CONF_S1, RS200_TAG_S1, rs200_component.set_s1_number)
    await _register_threshold(config, CONF_S2, RS200_TAG_S2, rs200_component.set_s2_number)
    await _register_threshold(config, CONF_S3, RS200_TAG_S3, rs200_component.set_s3_number)
    await _register_threshold(config, CONF_N1, RS200_TAG_N1, rs200_component.set_n1_number)
    await _register_threshold(config, CONF_N2, RS200_TAG_N2, rs200_component.set_n2_number)
    await _register_threshold(config, CONF_N3, RS200_TAG_N3, rs200_component.set_n3_number)
    await _register_threshold(config, CONF_OPTICAL_CALIBRATION, RS200_TAG_OPTICAL,
                               rs200_component.set_optical_calibration_number)
