#ifndef ESPHOMELIB_LIGHT_LIGHT_EFFECT_H
#define ESPHOMELIB_LIGHT_LIGHT_EFFECT_H

#include "esphomelib/defines.h"

#ifdef USE_LIGHT

#include <string>
#include "esphomelib/light/light_traits.h"
#include "esphomelib/light/light_state.h"

ESPHOMELIB_NAMESPACE_BEGIN

namespace light {

class LightState;

class LightEffect {
 public:
  explicit LightEffect(const std::string &name);

  /// Initialize this LightEffect. Will be called once after creation.
  virtual void start();

  virtual void start_();

  /// Called when this effect is about to be removed
  virtual void stop();

  /// Apply this effect. Use the provided state for starting transitions, ...
  virtual void apply() = 0;

  const std::string &get_name();

  /// Internal method called by the LightState when this light effect is registered in it.
  virtual void init();

  void init_(LightState *state);

 protected:
  LightState *state_{nullptr};
  std::string name_;
};

/// Random effect. Sets random colors every 10 seconds and slowly transitions between them.
class RandomLightEffect : public LightEffect {
 public:
  explicit RandomLightEffect(const std::string &name);

  void apply() override;

  void set_transition_length(uint32_t transition_length);

  void set_update_interval(uint32_t update_interval);

 protected:
  uint32_t last_color_change_{0};
  uint32_t transition_length_{7500};
  uint32_t update_interval_{10000};
};

class LambdaLightEffect : public LightEffect {
 public:
  LambdaLightEffect(const std::string &name, const std::function<void()> &f, uint32_t update_interval);

  void apply() override;

 protected:
  std::function<void()> f_;
  uint32_t update_interval_;
  uint32_t last_run_{0};
};

struct StrobeLightEffectColor {
  LightColorValues color;
  uint32_t duration;
};

class StrobeLightEffect : public LightEffect {
 public:
  StrobeLightEffect(const std::string &name);
  void apply() override;

  void set_colors(const std::vector<StrobeLightEffectColor> &colors);

 protected:
  std::vector<StrobeLightEffectColor> colors_;
  uint32_t last_switch_{0};
  size_t at_color_{0};
};

class FlickerLightEffect : public LightEffect {
 public:
  FlickerLightEffect(const std::string &name);

  void apply() override;

  void set_alpha(float alpha);
  void set_intensity(float intensity);
 protected:
  float intensity_{0.015f};
  float alpha_{0.95f};
};

} // namespace light

ESPHOMELIB_NAMESPACE_END

#endif //USE_LIGHT

#endif //ESPHOMELIB_LIGHT_LIGHT_EFFECT_H
