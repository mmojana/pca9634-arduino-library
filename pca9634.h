/*!
 * 
 * 
 * 
 * 
 * 
 */

#ifndef PCA9634_H
#define PCA9634_H

#include <Wire.h>
#include <Arduino.h>

typedef enum {
	STOP_COMMAND,
	ACK
} OutputChangeTrigger;

typedef enum {
	OPEN_DRAIN,
	TOTEM_POLE
} OutputDriverStructure;

typedef enum {
	ZERO,
	ONE_OR_WEAK_HIGH,
	HIGH_Z
} OutputWhenNotEnabled;

class Pca9634 {
public:
  Pca9634();
  Pca9634(const uint8_t i2cSlaveAddr);
  Pca9634(const uint8_t i2cSlaveAddr, TwoWire &i2c);
  
  void begin();
  bool reset();
  void sleep();
  void wakeup();
  
  void setSubaddress1Active(const uint8_t addr);
  void setSubaddress2Active(const uint8_t addr);
  void setSubaddress3Active(const uint8_t addr);
  void setAllCallAddressActive(const uint8_t addr);
  void setAllCallAddressInactive();
  
  void configureOutputs(const bool inverted, const OutputChangeTrigger trigger, const OutputDriverStructure structure, const OutputWhenNotEnabled output);
  
  void configureDimmingEffect(const float ratio);
  void configureBlinkingEffect(const float period, const float dutyCycle);
  
  void setBrightness(const uint8_t channel, const uint16_t value);
  void setEffectEnabled(const uint8_t channel, const bool enabled);

private:
  uint8_t i2cSlaveAddr;
  TwoWire *i2c;

  void setAddressActive(const uint8_t index, const uint8_t addr, boolean enabled);
  void setGroupPwm(const float ratio);
  float clamp(float value, float min, float max);

  uint8_t readRegister(const uint8_t registerAddress);
  void writeRegister(const uint8_t registerAddress, const uint8_t value);
};

#endif
