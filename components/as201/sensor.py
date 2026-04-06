import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_TEMPERATURE, UNIT_CELSIUS, DEVICE_CLASS_TEMPERATURE, CONF_PRESSURE, \
    UNIT_PASCAL, DEVICE_CLASS_ATMOSPHERIC_PRESSURE, CONF_HEIGHT, UNIT_METER,
    STATE_CLASS_MEASUREMENT, UNIT_METER_PER_SECOND_SQUARED, ICON_BRIEFCASE_DOWNLOAD,
    UNIT_DEGREE_PER_SECOND, ICON_SCREEN_ROTATION, UNIT_DEGREES, CONF_FIELD_STRENGTH_X, CONF_FIELD_STRENGTH_Y,
    CONF_FIELD_STRENGTH_Z, UNIT_MICROTESLA, ICON_MAGNET,
)
from . import CONF_AS201_ID, AS201Component

DEPENDENCIES = ["as201"]

CONF_ACCEL_X = "accel_x"
CONF_ACCEL_Y = "accel_y"
CONF_ACCEL_Z = "accel_z"
CONF_GYRO_X = "gyro_x"
CONF_GYRO_Y = "gyro_y"
CONF_GYRO_Z = "gyro_z"
CONF_ANGLE_X = "angle_x"
CONF_ANGLE_Y = "angle_y"
CONF_ANGLE_Z = "angle_z"
CONF_Q0 = "q0"
CONF_Q1 = "q1"
CONF_Q2 = "q2"
CONF_Q3 = "q3"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_AS201_ID): cv.use_id(AS201Component),
            cv.Optional(CONF_ACCEL_X): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
                icon=ICON_BRIEFCASE_DOWNLOAD,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ACCEL_Y): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
                icon=ICON_BRIEFCASE_DOWNLOAD,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ACCEL_Z): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
                icon=ICON_BRIEFCASE_DOWNLOAD,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_GYRO_X): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREE_PER_SECOND,
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_GYRO_Y): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREE_PER_SECOND,
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_GYRO_Z): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREE_PER_SECOND,
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_ANGLE_X): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREES,
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT
            ),
            cv.Optional(CONF_ANGLE_Y): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREES,
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT
            ),
            cv.Optional(CONF_ANGLE_Z): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREES,
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT
            ),

            cv.Optional(CONF_FIELD_STRENGTH_X): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROTESLA,
                icon=ICON_MAGNET,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FIELD_STRENGTH_Y): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROTESLA,
                icon=ICON_MAGNET,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FIELD_STRENGTH_Z): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROTESLA,
                icon=ICON_MAGNET,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_Q0): sensor.sensor_schema(
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_Q1): sensor.sensor_schema(
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_Q2): sensor.sensor_schema(
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_Q3): sensor.sensor_schema(
                icon=ICON_SCREEN_ROTATION,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PRESSURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PASCAL,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ATMOSPHERIC_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_HEIGHT): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ATMOSPHERIC_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            )
        }
    )
)

async def to_code(config):
    as201_component = await cg.get_variable(config[CONF_AS201_ID])

    if CONF_ACCEL_X in config:
        sens = await sensor.new_sensor(config[CONF_ACCEL_X])
        cg.add(as201_component.set_accel_x_sensor(sens))
    if CONF_ACCEL_Y in config:
        sens = await sensor.new_sensor(config[CONF_ACCEL_Y])
        cg.add(as201_component.set_accel_y_sensor(sens))
    if CONF_ACCEL_Z in config:
        sens = await sensor.new_sensor(config[CONF_ACCEL_Z])
        cg.add(as201_component.set_accel_z_sensor(sens))

    if CONF_GYRO_X in config:
        sens = await sensor.new_sensor(config[CONF_GYRO_X])
        cg.add(as201_component.set_gyro_x_sensor(sens))
    if CONF_GYRO_Y in config:
        sens = await sensor.new_sensor(config[CONF_GYRO_Y])
        cg.add(as201_component.set_gyro_y_sensor(sens))
    if CONF_GYRO_Z in config:
        sens = await sensor.new_sensor(config[CONF_GYRO_Z])
        cg.add(as201_component.set_gyro_z_sensor(sens))

    if CONF_ANGLE_X in config:
        sens = await sensor.new_sensor(config[CONF_ANGLE_X])
        cg.add(as201_component.set_angle_x_sensor(sens))
    if CONF_ANGLE_Y in config:
        sens = await sensor.new_sensor(config[CONF_ANGLE_Y])
        cg.add(as201_component.set_angle_y_sensor(sens))
    if CONF_ANGLE_Z in config:
        sens = await sensor.new_sensor(config[CONF_ANGLE_Z])
        cg.add(as201_component.set_angle_z_sensor(sens))

    if CONF_FIELD_STRENGTH_X in config:
        sens = await sensor.new_sensor(config[CONF_FIELD_STRENGTH_X])
        cg.add(as201_component.set_field_strength_x_sensor(sens))
    if CONF_FIELD_STRENGTH_Y in config:
        sens = await sensor.new_sensor(config[CONF_FIELD_STRENGTH_Y])
        cg.add(as201_component.set_field_strength_y_sensor(sens))
    if CONF_FIELD_STRENGTH_Z in config:
        sens = await sensor.new_sensor(config[CONF_FIELD_STRENGTH_Z])
        cg.add(as201_component.set_field_strength_z_sensor(sens))

    if CONF_Q0 in config:
        sens = await sensor.new_sensor(config[CONF_Q0])
        cg.add(as201_component.set_q0_sensor(sens))
    if CONF_Q1 in config:
        sens = await sensor.new_sensor(config[CONF_Q1])
        cg.add(as201_component.set_q1_sensor(sens))
    if CONF_Q2 in config:
        sens = await sensor.new_sensor(config[CONF_Q2])
        cg.add(as201_component.set_q2_sensor(sens))
    if CONF_Q3 in config:
        sens = await sensor.new_sensor(config[CONF_Q3])
        cg.add(as201_component.set_q3_sensor(sens))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(as201_component.set_temperature_sensor(sens))
    if CONF_PRESSURE in config:
        sens = await sensor.new_sensor(config[CONF_PRESSURE])
        cg.add(as201_component.set_pressure_sensor(sens))
    if CONF_HEIGHT in config:
        sens = await sensor.new_sensor(config[CONF_HEIGHT])
        cg.add(as201_component.set_height_sensor(sens))
