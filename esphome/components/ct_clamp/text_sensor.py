import esphome.codegen as cg

ct_clamp_ns = cg.esphome_ns.namespace("ct_clamp")
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import text_sensor
from esphome.components.text_sensor import TextSensorPublishAction
from esphome.const import CONF_ID, CONF_LAMBDA, CONF_STATE, CONF_SENSOR

CTClampSensor = ct_clamp_ns.class_(
    "CTClampSensor", text_sensor.TextSensor, cg.PollingComponent
)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema()
    .extend(
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
    await cg.register_component(var, config)

