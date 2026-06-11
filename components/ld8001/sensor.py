import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_DISTANCE,
    STATE_CLASS_MEASUREMENT, UNIT_METER,
    DEVICE_CLASS_DISTANCE
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["uart"]

ld8001 = cg.esphome_ns.namespace("ld8001")
LD8001Component = ld8001.class_("LD8001Component", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LD8001Component),
            cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER,
                device_class=DEVICE_CLASS_DISTANCE,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "ld8001",
    baud_rate=115200,
    parity=None,
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))