#include "daly_bms_dispatcher.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

// Include per avere la definizione completa di DalyBmsComponent
// Usiamo il path relativo ESPHome-style
#include "../daly_bms/daly_bms.h"

namespace esphome {
namespace daly_bms {

static const char *const TAG = "daly_bms_dispatcher";
static const uint8_t DALY_FRAME_SIZE = 13;

void DalyBmsDispatcher::setup() {
  ESP_LOGCONFIG(TAG, "Daly BMS Dispatcher setup - managing %d BMS instances", this->bms_instances_.size());
}

float DalyBmsDispatcher::get_setup_priority() const {
  return setup_priority::BUS - 1.0f;  // Prima di DalyBmsComponent
}

void DalyBmsDispatcher::register_bms(uint8_t response_addr, DalyBmsComponent *bms) {
  this->bms_instances_[response_addr] = bms;
  ESP_LOGCONFIG(TAG, "Registered BMS with response address 0x%02X", response_addr);
}

void DalyBmsDispatcher::write_frame(const uint8_t *data, size_t len) {
  this->write_array(data, len);
  this->flush();
}

void DalyBmsDispatcher::loop() {
  const uint32_t now = millis();
  
  // Timeout per reset buffer se nessun byte da troppo tempo
  if (this->receiving_ && (now - this->last_byte_time_ > 200)) {
    if (this->buffer_.size() > 0) {
      ESP_LOGW(TAG, "Frame timeout, resetting buffer (had %d bytes)", this->buffer_.size());
    }
    this->buffer_.clear();
    this->receiving_ = false;
  }
  
  // Leggi tutti i byte disponibili
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->last_byte_time_ = now;
    
    // Se non stiamo ricevendo, cerca start byte
    if (!this->receiving_) {
      if (byte == 0xA5) {
        this->buffer_.clear();
        this->buffer_.push_back(byte);
        this->receiving_ = true;
        this->expected_length_ = DALY_FRAME_SIZE;
      }
      continue;
    }
    
    // Aggiungi byte al buffer
    this->buffer_.push_back(byte);
    
    // Se abbiamo il byte del data length (posizione 4)
    if (this->buffer_.size() == 4) {
      this->expected_length_ = byte + 5;  // data_length + header(4) + checksum(1)
    }
    
    // Se abbiamo un frame completo
    if (this->buffer_.size() >= DALY_FRAME_SIZE && 
        this->buffer_.size() >= this->expected_length_) {
      
      // Estrai l'indirizzo di risposta (byte 2)
      uint8_t response_addr = this->buffer_[1];
      
      ESP_LOGV(TAG, "Complete frame received: addr=0x%02X, len=%d", 
               response_addr, this->buffer_.size());
      
      // Trova l'istanza BMS corrispondente
      auto it = this->bms_instances_.find(response_addr);
      if (it != this->bms_instances_.end()) {
        ESP_LOGV(TAG, "Dispatching frame to BMS instance (response_addr=0x%02X)", response_addr);
        it->second->process_frame(this->buffer_);
      } else {
        ESP_LOGV(TAG, "No BMS instance registered for response address 0x%02X", response_addr);
      }
      
      // Reset per il prossimo frame
      this->buffer_.clear();
      this->receiving_ = false;
    }
    
    // Protezione: se il buffer diventa troppo grande, resetta
    if (this->buffer_.size() > 50) {
      ESP_LOGW(TAG, "Buffer overflow, resetting (size=%d)", this->buffer_.size());
      this->buffer_.clear();
      this->receiving_ = false;
    }
  }
}

}  // namespace daly_bms
}  // namespace esphome
