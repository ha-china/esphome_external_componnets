from esphome import automation, pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID, CONF_INTEGRATION_TIME, ICON_BRIGHTNESS_5, CONF_GAIN, CONF_INTERRUPT_PIN, CONF_INTERRUPT,
    DEVICE_CLASS_TEMPERATURE, STATE_CLASS_MEASUREMENT, CONF_INTERNAL_TEMPERATURE, CONF_TRIGGER_ID
)

CODEOWNERS = ["@synodriver"]
DEPENDENCIES = ["i2c"]
CONF_CONVERSION_TYPE = "conversion_type"
CONF_LED_DRV_CURRENT = "led_drv_current"
CONF_LED_DRV = "led_drv"
CONF_LED_IND_CURRENT = "led_ind_current"
CONF_LED_IND = "led_ind"
CONF_ON_DATA_READY = "on_data_ready"
CONF_CHANNEL_1 = "channel_1"
CONF_CHANNEL_2 = "channel_2"
CONF_CHANNEL_3 = "channel_3"
CONF_CHANNEL_4 = "channel_4"
CONF_CHANNEL_5 = "channel_5"
CONF_CHANNEL_6 = "channel_6"
CONF_CALIBRATED_X = "calibrated_x"
CONF_CALIBRATED_Y = "calibrated_y"
CONF_CALIBRATED_Z = "calibrated_z"
CONF_CALIBRATED_X1931 = "calibrated_x1931"
CONF_CALIBRATED_Y1931 = "calibrated_y1931"
CONF_CALIBRATED_UPRI = "calibrated_upri"
CONF_CALIBRATED_VPRI = "calibrated_vpri"
CONF_CALIBRATED_U = "calibrated_u"
CONF_CALIBRATED_V = "calibrated_v"
CONF_CALIBRATED_DUV = "calibrated_duv"
CONF_CALIBRATED_LUX = "calibrated_lux"
CONF_CALIBRATED_CCT = "calibrated_cct"

as762x = cg.esphome_ns.namespace("as762x")
AS762XComponent = as762x.class_("AS762XComponent", cg.PollingComponent, i2c.I2CDevice)

AS762X_GAIN = as762x.enum("AS762X_GAIN")
AS762X_GAIN_OPTIONS = {
    "1x": AS762X_GAIN.AS762X_GAIN_1X,
    "3.7x": AS762X_GAIN.AS762X_GAIN_3_7X,
    "16x": AS762X_GAIN.AS762X_GAIN_16X,
    "64x": AS762X_GAIN.AS762X_GAIN_64X,
}

AS762X_CONVERSION_TYPE = as762x.enum("AS762X_CONVERSION_TYPE")
AS762X_CONVERSION_TYPE_OPTIONS = {
    0: AS762X_CONVERSION_TYPE.AS762X_CONVERSION_TYPE_0,
    1: AS762X_CONVERSION_TYPE.AS762X_CONVERSION_TYPE_1,
    2: AS762X_CONVERSION_TYPE.AS762X_CONVERSION_TYPE_2,
    3: AS762X_CONVERSION_TYPE.AS762X_CONVERSION_TYPE_3,
}

AS762X_LED_DRV_CURRENT = as762x.enum("AS762X_LED_DRV_CURRENT")
AS762X_LED_DRV_CURRENT_OPTIONS = {
    "12.5mA" :AS762X_LED_DRV_CURRENT.AS762X_LED_DRV_CURRENT_12_5MA,
    "25mA" :AS762X_LED_DRV_CURRENT.AS762X_LED_DRV_CURRENT_25MA,
    "50mA" :AS762X_LED_DRV_CURRENT.AS762X_LED_DRV_CURRENT_50MA,
    "100mA" :AS762X_LED_DRV_CURRENT.AS762X_LED_DRV_CURRENT_100MA,
}

AS762X_LED_IND_CURRENT = as762x.enum("AS762X_LED_IND_CURRENT")
AS762X_LED_IND_CURRENT_OPTIONS = {
    "1mA": AS762X_LED_IND_CURRENT.AS762X_LED_IND_CURRENT_1MA,
    "2mA": AS762X_LED_IND_CURRENT.AS762X_LED_IND_CURRENT_2MA,
    "4mA": AS762X_LED_IND_CURRENT.AS762X_LED_IND_CURRENT_4MA,
    "8mA": AS762X_LED_IND_CURRENT.AS762X_LED_IND_CURRENT_8MA,
}

DataReadyTrigger = as762x.class_("DataReadyTrigger", automation.Trigger.template())

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS762XComponent),
            cv.Optional(CONF_INTERRUPT, default=True): cv.boolean,
            cv.Optional(CONF_GAIN, default="64x"): cv.enum(AS762X_GAIN_OPTIONS),
            cv.Optional(CONF_CONVERSION_TYPE, default=2): cv.enum(AS762X_CONVERSION_TYPE_OPTIONS),
            cv.Optional(CONF_INTEGRATION_TIME, default=255): cv.uint8_t,
            cv.Optional(CONF_LED_DRV_CURRENT, default="100mA"): cv.enum(AS762X_LED_DRV_CURRENT_OPTIONS),
            cv.Optional(CONF_LED_DRV, default=True): cv.boolean,
            cv.Optional(CONF_LED_IND_CURRENT, default="8mA"): cv.enum(AS762X_LED_IND_CURRENT_OPTIONS),
            cv.Optional(CONF_LED_IND, default=True): cv.boolean,
            cv.Optional(CONF_INTERRUPT_PIN): cv.All(
                pins.internal_gpio_input_pin_schema
            ),
            cv.Optional(CONF_ON_DATA_READY): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(DataReadyTrigger),
                }
            ),
            cv.Optional(CONF_CHANNEL_1): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHANNEL_2): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHANNEL_3): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHANNEL_4): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHANNEL_5): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHANNEL_6): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_X): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_Y): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_Z): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_X1931): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_Y1931): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_UPRI): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_VPRI): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_U): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_V): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_DUV): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_LUX): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CALIBRATED_CCT): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                icon=ICON_BRIGHTNESS_5,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_INTERNAL_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement="#",
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("20s"))
    .extend(i2c.i2c_device_schema(0x49)),  # as7621
)

FINAL_VALIDATE_SCHEMA = i2c.final_validate_device_schema("as762x", max_frequency="400khz")


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_interrupt(config[CONF_INTERRUPT]))
    cg.add(var.set_gain(config[CONF_GAIN]))
    cg.add(var.set_conversion_type(config[CONF_CONVERSION_TYPE]))
    cg.add(var.set_integration_time(config[CONF_INTEGRATION_TIME]))
    cg.add(var.set_led_drv_current(config[CONF_LED_DRV_CURRENT]))
    cg.add(var.set_led_drv(config[CONF_LED_DRV]))
    cg.add(var.set_led_ind_current(config[CONF_LED_IND_CURRENT]))
    cg.add(var.set_led_ind(config[CONF_LED_IND]))
    if pin := config.get(CONF_INTERRUPT_PIN):
        interrupt_pin = await cg.gpio_pin_expression(pin)
        cg.add(var.set_interrupt_pin(interrupt_pin))
    for conf in config.get(CONF_ON_DATA_READY, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    if CONF_CHANNEL_1 in config:
        sens = await sensor.new_sensor(config[CONF_CHANNEL_1])
        cg.add(var.set_channel_1_sensor(sens))
    if CONF_CHANNEL_2 in config:
        sens = await sensor.new_sensor(config[CONF_CHANNEL_2])
        cg.add(var.set_channel_2_sensor(sens))
    if CONF_CHANNEL_3 in config:
        sens = await sensor.new_sensor(config[CONF_CHANNEL_3])
        cg.add(var.set_channel_3_sensor(sens))
    if CONF_CHANNEL_4 in config:
        sens = await sensor.new_sensor(config[CONF_CHANNEL_4])
        cg.add(var.set_channel_4_sensor(sens))
    if CONF_CHANNEL_5 in config:
        sens = await sensor.new_sensor(config[CONF_CHANNEL_5])
        cg.add(var.set_channel_5_sensor(sens))
    if CONF_CHANNEL_6 in config:
        sens = await sensor.new_sensor(config[CONF_CHANNEL_6])
        cg.add(var.set_channel_6_sensor(sens))
    if CONF_CALIBRATED_X in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_X])
        cg.add(var.set_calibrated_x_sensor(sens))
    if CONF_CALIBRATED_Y in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_Y])
        cg.add(var.set_calibrated_y_sensor(sens))
    if CONF_CALIBRATED_Z in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_Z])
        cg.add(var.set_calibrated_z_sensor(sens))
    if CONF_CALIBRATED_X1931 in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_X1931])
        cg.add(var.set_calibrated_x1931_sensor(sens))
    if CONF_CALIBRATED_Y1931 in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_Y1931])
        cg.add(var.set_calibrated_y1931_sensor(sens))
    if CONF_CALIBRATED_UPRI in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_UPRI])
        cg.add(var.set_calibrated_upri_sensor(sens))
    if CONF_CALIBRATED_VPRI in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_VPRI])
        cg.add(var.set_calibrated_vpri_sensor(sens))
    if CONF_CALIBRATED_U in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_U])
        cg.add(var.set_calibrated_u_sensor(sens))
    if CONF_CALIBRATED_V in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_V])
        cg.add(var.set_calibrated_v_sensor(sens))
    if CONF_CALIBRATED_DUV in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_DUV])
        cg.add(var.set_calibrated_duv_sensor(sens))
    if CONF_CALIBRATED_LUX in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_LUX])
        cg.add(var.set_calibrated_lux_sensor(sens))
    if CONF_CALIBRATED_CCT in config:
        sens = await sensor.new_sensor(config[CONF_CALIBRATED_CCT])
        cg.add(var.set_calibrated_cct_sensor(sens))
    if CONF_INTERNAL_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_INTERNAL_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))


AS762XResetAction = as762x.class_("AS762XResetAction", automation.Action)
AS762X_RESETACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(AS762XComponent),
    }
)


@automation.register_action("as762x.reset", AS762XResetAction, AS762X_RESETACTION_SCHEMA, synchronous=True)
async def as762x_reset_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
