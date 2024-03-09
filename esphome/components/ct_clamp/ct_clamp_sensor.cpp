#include "ct_clamp_sensor.h"

#include "esphome/core/log.h"
#include <cmath>
#include <sched.h>
#include <time.h>
#include <cmath>
#include <cstdlib>
#include "esp_tls_crypto.h"

namespace esphome {
namespace ct_clamp {

static const char *const TAG = "ct_clamp";

void CTClampSensor::dump_config() {
  LOG_SENSOR("", "CT Clamp Sensor", this);
  ESP_LOGCONFIG(TAG, "  Sample Duration: %.2fs", this->sample_duration_ / 1e3f);
  LOG_UPDATE_INTERVAL(this);
}

void CTClampSensor::update() {
  // Update only starts the sampling phase, in loop() the actual sampling is happening.

  // Request a high loop() execution interval during sampling phase.
  this->high_freq_.start();

  // Set timeout for ending sampling phase
  this->set_timeout("read", this->sample_duration_, [this]() {
    this->is_sampling_ = false;
    this->high_freq_.stop();

    if (this->num_samples_ == 0) {
      // Shouldn't happen, but let's not crash if it does.
      this->publish_state(NAN);
      return;
    }

    const float rms_ac_dc_squared = this->sample_squared_sum_ / this->num_samples_;
    const float rms_dc = this->sample_sum_ / this->num_samples_;
    const float rms_ac_squared = rms_ac_dc_squared - rms_dc * rms_dc;
    float rms_ac = 0;
    if (rms_ac_squared > 0)
      rms_ac = std::sqrt(rms_ac_squared);
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    time_t seconds = spec.tv_sec;
    uint32_t ms = round(spec.tv_nsec / 1e6);
    ESP_LOGD(TAG, "'%s' - Raw AC Value: %.3fA after %d different samples (%d SPS) %d", this->name_.c_str(), rms_ac,
             this->num_samples_, 1000 * this->num_samples_ / this->sample_duration_, ms);
    const char *input = (char*)this->waveform.data();
    unsigned char* output = (unsigned char *)malloc(this->waveform.size()*sizeof(float)*4/3+10);
    size_t outlen;

    esp_crypto_base64_encode(output, 64, &outlen, (unsigned char*)input, this->waveform.size()*sizeof(float));
    ESP_LOGD(TAG,"%s",output);
    free(output);
    //for(int i=0; i < this->waveform.size(); i++){
    //  ESP_LOGD(TAG, "%d,%.3f,%.1f%%",this->sample_times[i],this->waveform[i],(float)i/this->num_samples_*100);
    //}
    //esp_crypto_base64_encode(nullptr, 0, &n, reinterpret_cast<const uint8_t *>(user_info.c_str()), user_info.size());
    
    this->publish_state(rms_ac);
  });

  // Set sampling values
  this->last_value_ = 0.0;
  this->num_samples_ = 0;
  this->sample_sum_ = 0.0f;
  this->sample_squared_sum_ = 0.0f;
  this->is_sampling_ = true;
  this->waveform.clear();
  this->sample_times.clear();
}

void CTClampSensor::loop() {
  if (!this->is_sampling_)
    return;

  // Perform a single sample
  float value = this->source_->sample();
  if (std::isnan(value))
    return;

  // Assuming a sine wave, avoid requesting values faster than the ADC can provide them
  //if (this->last_value_ == value)
  //  return;
  this->last_value_ = value;

  this->num_samples_++;
  this->sample_sum_ += value;
  this->sample_squared_sum_ += value * value;
  this->waveform.push_back(value);
  struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    this->sample_times.push_back(spec.tv_nsec);
}

}  // namespace ct_clamp
}  // namespace esphome
