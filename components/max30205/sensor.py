from esphome import automation
from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    CONF_ID, CONF_MODE, CONF_TEMPERATURE, UNIT_CELSIUS, ICON_THERMOMETER,
    DEVICE_CLASS_TEMPERATURE, CONF_TIMEOUT, CONF_INTERRUPT_PIN, CONF_TRIGGER_ID
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["i2c"]

CONF_DATA_FORMAT = "data_format"
CONF_FAULT_QUEUE = "fault_queue"
CONF_POLARITY = "polarity"
CONF_ON_OVERTEMPERATURE = "on_over_temperature"
CONF_HIGH_THRESHOLD = "high_threshold"
CONF_LOW_THRESHOLD = "low_threshold"

max30205_ns = cg.esphome_ns.namespace("max30205")
MAX30205Component = max30205_ns.class_("MAX30205Component", cg.PollingComponent, i2c.I2CDevice)

MAX30205_DATA_FORMAT = max30205_ns.enum("MAX30205_DATA_FORMAT")
MAX30205_DATA_FORMAT_OPTIONS = {
    "normal": MAX30205_DATA_FORMAT.MAX30205_DATA_FORMAT_NORMAL,
    "extended": MAX30205_DATA_FORMAT.MAX30205_DATA_FORMAT_EXTENDED
}

MAX30205_FAULT_QUEUE = max30205_ns.enum("MAX30205_FAULT_QUEUE")
MAX30205_FAULT_QUEUE_OPTIONS = {
    1: MAX30205_FAULT_QUEUE.MAX30205_FAULT_QUEUE_1,
    2: MAX30205_FAULT_QUEUE.MAX30205_FAULT_QUEUE_2,
    4: MAX30205_FAULT_QUEUE.MAX30205_FAULT_QUEUE_4,
    6: MAX30205_FAULT_QUEUE.MAX30205_FAULT_QUEUE_6,
}

MAX30205_PIN_POLARITY = max30205_ns.enum("MAX30205_PIN_POLARITY")
MAX30205_PIN_POLARITY_OPTIONS = {
    "low": MAX30205_PIN_POLARITY.MAX30205_PIN_POLARITY_LOW,
    "high": MAX30205_PIN_POLARITY.MAX30205_PIN_POLARITY_HIGH
}

MAX30205_MODE = max30205_ns.enum("MAX30205_MODE")
MAX30205_MODE_OPTIONS = {
    "comparator": MAX30205_MODE.MAX30205_MODE_COMPARATOR,
    "interrupt": MAX30205_MODE.MAX30205_MODE_INTERRUPT,
}

OverTemperatureTrigger = max30205_ns.class_("OverTemperatureTrigger", automation.Trigger.template(cg.float_))

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MAX30205Component),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TIMEOUT, default=False): cv.boolean,
            cv.Optional(CONF_DATA_FORMAT, default="normal"): cv.enum(MAX30205_DATA_FORMAT_OPTIONS),
            cv.Optional(CONF_FAULT_QUEUE, default=1): cv.enum(MAX30205_FAULT_QUEUE_OPTIONS),
            cv.Optional(CONF_POLARITY, default="low"): cv.enum(MAX30205_PIN_POLARITY_OPTIONS),
            cv.Optional(CONF_MODE, default="interrupt"): cv.enum(MAX30205_MODE_OPTIONS),
            cv.Optional(CONF_HIGH_THRESHOLD, 80): cv.int_,
            cv.Optional(CONF_LOW_THRESHOLD, 75): cv.int_,

            cv.Optional(CONF_INTERRUPT_PIN): cv.All(
                pins.internal_gpio_input_pin_schema
            ),
            cv.Optional(CONF_ON_OVERTEMPERATURE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(OverTemperatureTrigger),
                }
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(0x90)),
)

FINAL_VALIDATE_SCHEMA = i2c.final_validate_device_schema("max30205", max_frequency="400khz")


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))
    cg.add(var.set_timeout(config[CONF_TIMEOUT]))
    cg.add(var.set_data_format(config[CONF_DATA_FORMAT]))
    cg.add(var.set_fault_queue(config[CONF_FAULT_QUEUE]))
    cg.add(var.set_polarity(config[CONF_POLARITY]))
    cg.add(var.set_mode(config[CONF_MODE]))
    cg.add(var.set_high_threshold(config[CONF_HIGH_THRESHOLD]))
    cg.add(var.set_low_threshold(config[CONF_LOW_THRESHOLD]))
    if pin := config.get(CONF_INTERRUPT_PIN):
        interrupt_pin = await cg.gpio_pin_expression(pin)
        cg.add(var.set_interrupt_pin(interrupt_pin))
    for conf in config.get(CONF_ON_OVERTEMPERATURE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.float_, "temperature"), ], conf)


MAX30205ShutdownAction = max30205_ns.class_("MAX30205ShutdownAction", automation.Action)
MAX30205_SHUTDOWNACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30205Component),
    }
)


@automation.register_action("max30205.shutdown", MAX30205ShutdownAction, MAX30205_SHUTDOWNACTION_SCHEMA, synchronous=True)
async def max30205_shutdown_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


MAX30205WakeupAction = max30205_ns.class_("MAX30205WakeupAction", automation.Action)
MAX30205_WAKEUPACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30205Component),
    }
)


@automation.register_action("max30205.wakeup", MAX30205WakeupAction, MAX30205_WAKEUPACTION_SCHEMA, synchronous=True)
async def max30205_wakeup_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
