#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include <vector>
#include <map>

namespace esphome {
namespace daly_bms {

// Forward declaration
class DalyBmsComponent;

class DalyBmsDispatcher : public Component, public uart::UARTDevice {
 public:
  DalyBmsDispatcher() = default;

  void setup() override;
  void loop() override;
  float get_setup_priority() const override;

  // Registra un'istanza BMS con il suo response address
  void register_bms(uint8_t response_addr, DalyBmsComponent *bms);
  
  // Invia dati all'UART
  void write_frame(const uint8_t *data, size_t len);

 protected:
  std::map<uint8_t, DalyBmsComponent*> bms_instances_;
  std::vector<uint8_t> buffer_;
  bool receiving_{false};
  uint8_t expected_length_{0};
  uint32_t last_byte_time_{0};
};

}  // namespace daly_bms
}  // namespace esphome
