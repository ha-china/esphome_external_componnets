import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    STATE_CLASS_MEASUREMENT, UNIT_PERCENT, CONF_EC, CONF_TEMPERATURE,
    UNIT_PARTS_PER_MILLION, UNIT_MICROSIEMENS_PER_CENTIMETER, DEVICE_CLASS_TEMPERATURE, UNIT_CELSIUS,
    ICON_WATER_PERCENT, ICON_CURRENT_AC, ICON_CHEMICAL_WEAPON, ICON_SCALE, ICON_FLASK,
)

from . import CONF_BAX_ID, BAXComponent, CONF_SALINITY

DEPENDENCIES = ["bax"]

CONF_TDS = "tds"
CONF_SPECIFIC_GRAVITY = "specific_gravity"
CONF_HARDNESS = "hardness"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_BAX_ID): cv.use_id(BAXComponent),
            cv.Optional(CONF_TDS): sensor.sensor_schema(
                icon=ICON_WATER_PERCENT,
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(f"{CONF_TDS}2"): sensor.sensor_schema(
                icon=ICON_WATER_PERCENT,
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(f"{CONF_TDS}3"): sensor.sensor_schema(
                icon=ICON_WATER_PERCENT,
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_EC): sensor.sensor_schema(
                icon=ICON_CURRENT_AC,
                unit_of_measurement=UNIT_MICROSIEMENS_PER_CENTIMETER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(f"{CONF_EC}2"): sensor.sensor_schema(
                icon=ICON_CURRENT_AC,
                unit_of_measurement=UNIT_MICROSIEMENS_PER_CENTIMETER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SALINITY): sensor.sensor_schema(
                icon=ICON_CHEMICAL_WEAPON,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SPECIFIC_GRAVITY): sensor.sensor_schema(  # 比重
                icon=ICON_SCALE,
                unit_of_measurement="#",
                accuracy_decimals=4,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(f"{CONF_TEMPERATURE}2"): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(f"{CONF_TEMPERATURE}3"): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HARDNESS): sensor.sensor_schema( # 硬度
                icon=ICON_FLASK,
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
)

async def to_code(config):
    bax_component = await cg.get_variable(config[CONF_BAX_ID])

    if tds_config := config.get(CONF_TDS):
        sens = await sensor.new_sensor(tds_config)
        cg.add(bax_component.set_tds_sensor(sens))
    if tds2_config := config.get(f"{CONF_TDS}2"):
        sens = await sensor.new_sensor(tds2_config)
        cg.add(bax_component.set_tds2_sensor(sens))
    if tds3_config := config.get(f"{CONF_TDS}3"):
        sens = await sensor.new_sensor(tds3_config)
        cg.add(bax_component.set_tds3_sensor(sens))
    if ec_config := config.get(CONF_EC):
        sens = await sensor.new_sensor(ec_config)
        cg.add(bax_component.set_ec_sensor(sens))
    if ec2_config := config.get(f"{CONF_EC}2"):
        sens = await sensor.new_sensor(ec2_config)
        cg.add(bax_component.set_ec2_sensor(sens))
    if salinity_config := config.get(CONF_SALINITY):
        sens = await sensor.new_sensor(salinity_config)
        cg.add(bax_component.set_salinity_sensor(sens))
    if specific_gravity_config := config.get(CONF_SPECIFIC_GRAVITY):
        sens = await sensor.new_sensor(specific_gravity_config)
        cg.add(bax_component.set_specific_gravity_sensor(sens))
    if temperature_config := config.get(CONF_TEMPERATURE):
        sens = await sensor.new_sensor(temperature_config)
        cg.add(bax_component.set_temperature_sensor(sens))
    if temperature2_config := config.get(f"{CONF_TEMPERATURE}2"):
        sens = await sensor.new_sensor(temperature2_config)
        cg.add(bax_component.set_temperature2_sensor(sens))
    if temperature3_config := config.get(f"{CONF_TEMPERATURE}3"):
        sens = await sensor.new_sensor(temperature3_config)
        cg.add(bax_component.set_temperature3_sensor(sens))
    if hardness_config := config.get(CONF_HARDNESS):
        sens = await sensor.new_sensor(hardness_config)
        cg.add(bax_component.set_hardness_sensor(sens))