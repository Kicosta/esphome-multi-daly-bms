import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID

CODEOWNERS = ["@s1lvi0", "Kicosta"]
MULTI_CONF = True
DEPENDENCIES = ["uart"]

CONF_BMS_DALY_ID = "bms_daly_id"
CONF_RESPONSE_ADDRESS = "response_address"
CONF_DISPATCHER_ID = "dispatcher_id"

daly_bms = cg.esphome_ns.namespace("daly_bms")
DalyBmsComponent = daly_bms.class_(
    "DalyBmsComponent", cg.PollingComponent
)
DalyBmsDispatcher = daly_bms.class_(
    "DalyBmsDispatcher", cg.Component, uart.UARTDevice
)

# Schema per il dispatcher (deve essere configurato una sola volta)
DISPATCHER_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DalyBmsDispatcher),
    }
).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DalyBmsComponent),
            cv.Optional(CONF_ADDRESS, default=0x80): cv.hex_uint8_t,
            cv.Optional(CONF_RESPONSE_ADDRESS, default=0x01): cv.hex_uint8_t,
            cv.Required(CONF_DISPATCHER_ID): cv.use_id(DalyBmsDispatcher),
        }
    )
    .extend(cv.polling_component_schema("30s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Debug: stampa i valori letti
    addr = config[CONF_ADDRESS]
    resp_addr = config[CONF_RESPONSE_ADDRESS]
    
    # Log durante la compilazione
    import logging
    _LOGGER = logging.getLogger(__name__)
    _LOGGER.warning(f"Configuring daly_bms: addr=0x{addr:02X}, response_addr=0x{resp_addr:02X}")
    
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_response_address(config[CONF_RESPONSE_ADDRESS]))
    
    # Collega al dispatcher
    dispatcher = await cg.get_variable(config[CONF_DISPATCHER_ID])
    cg.add(var.set_dispatcher(dispatcher))
