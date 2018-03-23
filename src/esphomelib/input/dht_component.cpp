//
// Created by Otto Winter on 26.11.17.
//

#include "esphomelib/input/dht_component.h"

#include "esphomelib/log.h"
#include "esphomelib/espmath.h"
#include "esphomelib/helpers.h"
#include "esphomelib/espmath.h"

namespace esphomelib {

namespace input {

using namespace esphomelib::sensor;

static const char *TAG = "input::dht";

DHTComponent::DHTComponent(uint8_t pin, uint32_t update_interval)
    : temperature_sensor_(new DHTTemperatureSensor(update_interval)),
      humidity_sensor_(new DHTHumiditySensor(update_interval)),
      PollingObject(update_interval) {
  this->set_pin(pin);
}

void DHTComponent::setup() {

  ESP_LOGCONFIG(TAG, "Setting up DHT...");
  ESP_LOGCONFIG(TAG, "    Pin: %u", this->pin_);
  ESP_LOGCONFIG(TAG, "    Model: %u", this->model_);
  this->dht_.setup(this->pin_, this->model_);
  ESP_LOGD(TAG, "    DHT.setup() -> status: %u", this->dht_.getStatus());
  ESP_LOGCONFIG(TAG, "    Update Interval: %u", this->get_update_interval());

  this->set_interval("check", this->get_update_interval(), [&]() {
    auto temp_hum = run_without_interrupts<std::pair<float, float>>([this] {
      return std::make_pair(this->dht_.getTemperature(), this->dht_.getHumidity());
    });
    float temperature = temp_hum.first;
    float humidity = temp_hum.second;

    if (temperature == 0.0 && humidity == 1.0) {
      // Sometimes DHT22 queries fail in a weird way where temperature is exactly 0.0 °C
      // and humidity is exactly 1%. This *exact* value pair shouldn't really happen
      // that much out in the wild (unless you're in a very cold dry room).
      // FIXME
      ESP_LOGW(TAG, "Got invalid temperature %f°C and humidity %f%% pair.");
      return;
    }

    ESP_LOGD(TAG, "Got Temperature=%.1f°C Humidity=%.1f%%", temperature, humidity);

    if (!isnan(temperature))
      this->temperature_sensor_->push_new_value(temperature, this->dht_.getNumberOfDecimalsTemperature());
    else
      ESP_LOGW(TAG, "Invalid Temperature: %f!C", temperature);

    if (!isnan(humidity))
      this->humidity_sensor_->push_new_value(humidity, this->dht_.getNumberOfDecimalsHumidity());
    else
      ESP_LOGW(TAG, "Invalid Humidity: %f%%", humidity);
  });
}

float DHTComponent::get_setup_priority() const {
  return setup_priority::HARDWARE;
}

TemperatureSensor *DHTComponent::get_temperature_sensor() const {
  return this->temperature_sensor_;
}

HumiditySensor *DHTComponent::get_humidity_sensor() const {
  return this->humidity_sensor_;
}
uint8_t DHTComponent::get_pin() const {
  return this->pin_;
}
void DHTComponent::set_pin(uint8_t pin) {
  assert_is_pin(pin);
  assert_construction_state(this);
  this->pin_ = pin;
}
void DHTComponent::set_dht_model(DHT::DHT_MODEL_t model) {
  assert_construction_state(this);
  this->model_ = model;
}
const DHT &DHTComponent::get_dht() const {
  return this->dht_;
}
void DHTComponent::set_update_interval(uint32_t update_interval) {
  this->update_interval_ = update_interval;
  this->temperature_sensor_->set_update_interval(update_interval);
  this->humidity_sensor_->set_update_interval(update_interval);
}

DHTTemperatureSensor::DHTTemperatureSensor(uint32_t update_interval)
    : PollingObject(update_interval) { }

DHTHumiditySensor::DHTHumiditySensor(uint32_t update_interval)
    : PollingObject(update_interval) { }

} // namespace input

} // namespace esphomelib
