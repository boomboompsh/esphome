import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor, voltage_sampler
from esphome.const import (
    CONF_SENSOR,
    DEVICE_CLASS_CURRENT,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
)

AUTO_LOAD = ["voltage_sampler"]
CODEOWNERS = ["@jesserockz"]

CONF_SAMPLE_DURATION = "sample_duration"

ct_clamp_ns = cg.esphome_ns.namespace("ct_clamp")
CTClampSensor = ct_clamp_ns.class_("CTClampSensor", text_sensor.TextSensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema().extend(
        {
            cv.Required(CONF_SENSOR): cv.use_id(voltage_sampler.VoltageSampler),
            cv.Optional(
                CONF_SAMPLE_DURATION, default="200ms"
            ): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    print(var)
    print(config)
    await cg.register_component(var, config)

    sens = await cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_source(sens))
    cg.add(var.set_sample_duration(config[CONF_SAMPLE_DURATION]))
