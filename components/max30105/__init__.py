from esphome import automation
import esphome.codegen as cg
from esphome import pins
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_TRIGGER_ID,
    CONF_SAMPLE_RATE,
    CONF_MODE,
    CONF_RESOLUTION,
    CONF_INTERRUPT_PIN,
    CONF_THRESHOLD,
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

CONF_ADC_RANGE = "adc_range"
CONF_SAMPLE_AVG = "sample_averaging"

max30105_ns = cg.esphome_ns.namespace("max30105")
MAX30105Component = max30105_ns.class_(
    "MAX30105Component", cg.PollingComponent, i2c.I2CDevice
)

CONF_MAX30105_ID = "max30105_id"

MAX30105_MODE = max30105_ns.enum("MAX30105_MODE")
MAX30105_MODE_OPTIONS = {
    "red": MAX30105_MODE.MAX30105_MODE_HR_ONLY,
    "red_ir": MAX30105_MODE.MAX30105_MODE_SPO2_HR,
    "green_red_ir": MAX30105_MODE.MAX30105_MODE_MULTI_LED,
}

MAX30105_ADC_RANGE = max30105_ns.enum("MAX30105_ADC_RANGE")
MAX30105_ADC_RANGE_OPTIONS = {
    2048: MAX30105_ADC_RANGE.MAX30105_ADC_RANGE_2048,
    4096: MAX30105_ADC_RANGE.MAX30105_ADC_RANGE_4096,
    8192: MAX30105_ADC_RANGE.MAX30105_ADC_RANGE_8192,
    16384: MAX30105_ADC_RANGE.MAX30105_ADC_RANGE_16384,
}

MAX30105_SAMPLE_AVERAGING = max30105_ns.enum("MAX30105_SAMPLE_AVERAGING")
MAX30105_SAMPLE_AVERAGING_OPTIONS = {
    1: MAX30105_SAMPLE_AVERAGING.MAX30105_SAMPLE_AVERAGING_1,
    2: MAX30105_SAMPLE_AVERAGING.MAX30105_SAMPLE_AVERAGING_2,
    4: MAX30105_SAMPLE_AVERAGING.MAX30105_SAMPLE_AVERAGING_4,
    8: MAX30105_SAMPLE_AVERAGING.MAX30105_SAMPLE_AVERAGING_8,
    16: MAX30105_SAMPLE_AVERAGING.MAX30105_SAMPLE_AVERAGING_16,
    32: MAX30105_SAMPLE_AVERAGING.MAX30105_SAMPLE_AVERAGING_32,
}

MAX30105_SAMPLE_RATE = max30105_ns.enum("MAX30105_SAMPLE_RATE")
MAX30105_SAMPLE_RATE_OPTIONS = {
    50: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_50,
    100: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_100,
    200: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_200,
    400: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_400,
    800: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_800,
    1000: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_1000,
    1600: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_1600,
    3200: MAX30105_SAMPLE_RATE.MAX30105_SAMPLE_RATE_3200,
}

MAX30105_RESOLUTION = max30105_ns.enum("MAX30105_RESOLUTION")
MAX30105_RESOLUTION_OPTIONS = {
    15: MAX30105_RESOLUTION.MAX30105_RESOLUTION_15_BIT,
    16: MAX30105_RESOLUTION.MAX30105_RESOLUTION_16_BIT,
    17: MAX30105_RESOLUTION.MAX30105_RESOLUTION_17_BIT,
    18: MAX30105_RESOLUTION.MAX30105_RESOLUTION_18_BIT,
}

CONF_FIFO_ROLLOVER = "fifo_rollover"
CONF_FIFO_THRESHOLD = "fifo_threshold"
CONF_RED_CURRENT = "red_current"
CONF_IR_CURRENT = "ir_current"
CONF_GREEN_CURRENT = "green_current"
CONF_PILOT_CURRENT = "pilot_current"
CONF_FIFO_ALMOST_FULL = "fifo_almost_full"
CONF_DATA_READY = "data_ready"
CONF_ALC_OVERFLOW = "alc_overflow"
CONF_PROX_INT = "prox_int"
CONF_TEMPERATURE_READY = "temp_ready"
CONF_PROXIMITY_THRESHOLD = "proximity_threshold"

CONF_ON_POWER_READY = "on_power_ready"
CONF_ON_FIFO_ALMOST_FULL = "on_fifo_almost_full"
CONF_ON_DATA_READY = "on_data_ready"
CONF_ON_ALC_OVERFLOW = "on_alc_overflow"
CONF_ON_PROX_INT = "on_prox_int"
CONF_ON_TEMPERATURE_READY = "on_temperature_ready"

PowerReadyTrigger = max30105_ns.class_("PowerReadyTrigger", automation.Trigger.template())
FifoAlmostFullTrigger = max30105_ns.class_("FifoAlmostFullTrigger", automation.Trigger.template())
DataReadyTrigger = max30105_ns.class_("DataReadyTrigger", automation.Trigger.template())
ALCOverflowTrigger = max30105_ns.class_("ALCOverflowTrigger", automation.Trigger.template())
ProximityInterruptTrigger = max30105_ns.class_("ProximityInterruptTrigger", automation.Trigger.template())
TemperatureReadyTrigger = max30105_ns.class_("TemperatureReadyTrigger", automation.Trigger.template(cg.float_))

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MAX30105Component),
            cv.Optional(CONF_MODE, default="green_red_ir"): cv.enum(MAX30105_MODE_OPTIONS),
            cv.Optional(CONF_ADC_RANGE, default=16384): cv.enum(MAX30105_ADC_RANGE_OPTIONS),
            cv.Optional(CONF_SAMPLE_AVG, default=32): cv.enum(MAX30105_SAMPLE_AVERAGING_OPTIONS),
            cv.Optional(CONF_FIFO_ROLLOVER, default=False): cv.boolean,
            cv.Optional(CONF_FIFO_THRESHOLD, default=0): cv.int_range(min=0, max=15),

            cv.Optional(CONF_SAMPLE_RATE, default=50): cv.enum(MAX30105_SAMPLE_RATE_OPTIONS),
            cv.Optional(CONF_RESOLUTION, default=18): cv.enum(MAX30105_RESOLUTION_OPTIONS),
            cv.Optional(CONF_RED_CURRENT, default=0x7F): cv.int_range(min=0x00, max=0xFF),
            cv.Optional(CONF_IR_CURRENT, default=0x7F): cv.int_range(min=0x00, max=0xFF),
            cv.Optional(CONF_GREEN_CURRENT, default=0x7F): cv.int_range(min=0x00, max=0xFF),
            cv.Optional(CONF_PILOT_CURRENT, default=0x7F): cv.int_range(min=0x00, max=0xFF),
            cv.Optional(CONF_FIFO_ALMOST_FULL, default=False): cv.boolean,
            cv.Optional(CONF_DATA_READY, default=False): cv.boolean,
            cv.Optional(CONF_ALC_OVERFLOW, default=False): cv.boolean,
            cv.Optional(CONF_PROX_INT, default=False): cv.boolean,
            cv.Optional(CONF_TEMPERATURE_READY, default=False): cv.boolean,

            cv.Optional(CONF_PROXIMITY_THRESHOLD, default=100): cv.int_range(min=0, max=255),

            cv.Optional(CONF_INTERRUPT_PIN): cv.All(
                pins.internal_gpio_input_pin_schema
            ),
            cv.Optional(CONF_ON_POWER_READY): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PowerReadyTrigger),
                }
            ),
            cv.Optional(CONF_ON_FIFO_ALMOST_FULL): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(FifoAlmostFullTrigger),
                }
            ),
            cv.Optional(CONF_ON_DATA_READY): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(DataReadyTrigger),
                }
            ),
            cv.Optional(CONF_ON_ALC_OVERFLOW): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ALCOverflowTrigger),
                }
            ),
            cv.Optional(CONF_ON_PROX_INT): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ProximityInterruptTrigger),
                }
            ),
            cv.Optional(CONF_ON_TEMPERATURE_READY): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TemperatureReadyTrigger),
                }
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(0x57)),
)

FINAL_VALIDATE_SCHEMA = i2c.final_validate_device_schema("max30105", max_frequency="400khz")


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_mode(config[CONF_MODE]))
    cg.add(var.set_adc_range(config[CONF_ADC_RANGE]))
    cg.add(var.set_sample_avg(config[CONF_SAMPLE_AVG]))
    cg.add(var.set_fifo_rollover(config[CONF_FIFO_ROLLOVER]))
    cg.add(var.set_fifo_threshold(config[CONF_FIFO_THRESHOLD]))
    cg.add(var.set_sample_rate(config[CONF_SAMPLE_RATE]))
    cg.add(var.set_resolution(config[CONF_RESOLUTION]))
    cg.add(
        var.set_current(config[CONF_RED_CURRENT], config[CONF_IR_CURRENT], config[CONF_GREEN_CURRENT],
                        config[CONF_PILOT_CURRENT]))
    cg.add(var.set_interrupts(config[CONF_FIFO_ALMOST_FULL], config[CONF_DATA_READY],
                              config[CONF_ALC_OVERFLOW], config[CONF_PROX_INT], config[CONF_TEMPERATURE_READY]))

    cg.add(var.set_proximity_threshold(config[CONF_PROXIMITY_THRESHOLD]))
    if pin := config.get(CONF_INTERRUPT_PIN):
        interrupt_pin = await cg.gpio_pin_expression(pin)
        cg.add(var.set_interrupt_pin(interrupt_pin))
    for conf in config.get(CONF_ON_POWER_READY, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)  # 自定义trigger的构造方法
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_FIFO_ALMOST_FULL, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_DATA_READY, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_ALC_OVERFLOW, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_PROX_INT, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_TEMPERATURE_READY, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.float_, "temperature"), ], conf)  # 中间是trigger中可以引用的变量的名字和类型


# 无参数automation
MAX30105ResetAction = max30105_ns.class_("MAX30105ResetAction", automation.Action)
MAX30105_RESETACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
    }
)


@automation.register_action("max30105.reset", MAX30105ResetAction, MAX30105_RESETACTION_SCHEMA, synchronous=True)
async def max30105_reset_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


MAX30105ShutdownAction = max30105_ns.class_("MAX30105ShutdownAction", automation.Action)
MAX30105_SHUTDOWNACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
    }
)


@automation.register_action("max30105.shutdown", MAX30105ShutdownAction, MAX30105_SHUTDOWNACTION_SCHEMA, synchronous=True)
async def max30105_shutdown_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


MAX30105WakeupAction = max30105_ns.class_("MAX30105WakeupAction", automation.Action)
MAX30105_WAKEUPACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
    }
)


@automation.register_action("max30105.wakeup", MAX30105WakeupAction, MAX30105_WAKEUPACTION_SCHEMA, synchronous=True)
async def max30105_wakeup_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


MAX30105SetProximityThresholdAction = max30105_ns.class_("MAX30105SetProximityThresholdAction", automation.Action)
MAX30105_SETPROXIMITYTHRESHOLDACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
        cv.Required(CONF_THRESHOLD): cv.templatable(cv.int_range(min=0, max=255)),
    }
)


@automation.register_action("max30105.set_proximity_threshold", MAX30105SetProximityThresholdAction,
                            MAX30105_SETPROXIMITYTHRESHOLDACTION_SCHEMA, synchronous=True)
async def max30105_set_proximity_threshold_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    threshold = await cg.templatable(config[CONF_THRESHOLD], args, cg.uint8)
    cg.add(var.set_threshold(threshold))
    return var


MAX30105SetModeAction = max30105_ns.class_("MAX30105SetModeAction", automation.Action)
MAX30105_SETMODEACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
        cv.Required(CONF_MODE): cv.templatable(cv.enum(MAX30105_MODE_OPTIONS)),
    }
)


@automation.register_action("max30105.set_mode", MAX30105SetModeAction, MAX30105_SETMODEACTION_SCHEMA, synchronous=True)
async def max30105_set_mode_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    mode = await cg.templatable(config[CONF_MODE], args, cg.uint8)
    cg.add(var.set_mode(mode))
    return var


MAX30105SetLedCurrentAction = max30105_ns.class_("MAX30105SetLedCurrentAction", automation.Action)
MAX30105_SETLEDCURRENTACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
        cv.Required(CONF_RED_CURRENT): cv.templatable(cv.int_range(min=0x00, max=0xFF)),
        cv.Required(CONF_IR_CURRENT): cv.templatable(cv.int_range(min=0x00, max=0xFF)),
        cv.Required(CONF_GREEN_CURRENT): cv.templatable(cv.int_range(min=0x00, max=0xFF)),
        cv.Optional(CONF_PILOT_CURRENT, default=0x7F): cv.templatable(cv.int_range(min=0x00, max=0xFF)),
    }
)


@automation.register_action("max30105.set_led_current", MAX30105SetLedCurrentAction,
                            MAX30105_SETLEDCURRENTACTION_SCHEMA, synchronous=True)
async def max30105_set_led_current_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    red_current = await cg.templatable(config[CONF_RED_CURRENT], args, cg.uint8)
    cg.add(var.set_red_current(red_current))
    ir_current = await cg.templatable(config[CONF_IR_CURRENT], args, cg.uint8)
    cg.add(var.set_ir_current(ir_current))
    green_current = await cg.templatable(config[CONF_GREEN_CURRENT], args, cg.uint8)
    cg.add(var.set_green_current(green_current))
    pilot_current = await cg.templatable(config.get(CONF_PILOT_CURRENT, 0x7F), args, cg.uint8)
    cg.add(var.set_pilot_current(pilot_current))
    return var


MAX30105EnableInterruptsAction = max30105_ns.class_("MAX30105EnableInterruptsAction", automation.Action)
MAX30105_ENABLEINTERRUPTSACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
        cv.Required(CONF_FIFO_ALMOST_FULL): cv.templatable(cv.boolean),
        cv.Required(CONF_DATA_READY): cv.templatable(cv.boolean),
        cv.Required(CONF_ALC_OVERFLOW): cv.templatable(cv.boolean),
        cv.Required(CONF_PROX_INT): cv.templatable(cv.boolean),
        cv.Required(CONF_TEMPERATURE_READY): cv.templatable(cv.boolean),
    }
)


@automation.register_action("max30105.enable_interrupts", MAX30105EnableInterruptsAction,
                            MAX30105_ENABLEINTERRUPTSACTION_SCHEMA, synchronous=True)
async def max30105_enable_interrupts_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    fifo_almost_full = await cg.templatable(config[CONF_FIFO_ALMOST_FULL], args, cg.bool_)
    cg.add(var.set_fifo_almost_full(fifo_almost_full))
    data_ready = await cg.templatable(config[CONF_DATA_READY], args, cg.bool_)
    cg.add(var.set_data_ready(data_ready))
    alc_overflow = await cg.templatable(config[CONF_ALC_OVERFLOW], args, cg.bool_)
    cg.add(var.set_alc_overflow(alc_overflow))
    prox_int = await cg.templatable(config[CONF_PROX_INT], args, cg.bool_)
    cg.add(var.set_prox_int(prox_int))
    temp_ready = await cg.templatable(config[CONF_TEMPERATURE_READY], args, cg.bool_)
    cg.add(var.set_temp_ready(temp_ready))
    return var


MAX30105SimulateInterruptAction = max30105_ns.class_("MAX30105SimulateInterruptAction", automation.Action)
MAX30105_SIMULATEINTERRUPSACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(MAX30105Component),
    }
)


@automation.register_action("max30105.simulate_interrupt", MAX30105SimulateInterruptAction,
                            MAX30105_SIMULATEINTERRUPSACTION_SCHEMA, synchronous=True)
async def max30105_simulate_interrupt_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
