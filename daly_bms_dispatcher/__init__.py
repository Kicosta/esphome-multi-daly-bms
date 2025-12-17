import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@s1lvi0"]
DEPENDENCIES = ["uart"]

daly_bms_dispatcher_ns = cg.esphome_ns.namespace("daly_bms")
DalyBmsDispatcher = daly_bms_dispatcher_ns.class_(
    "DalyBmsDispatcher", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DalyBmsDispatcher),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
