#include "pca9634.h"
#include <Wire.h>

typedef enum {
	OFF,
	FULLY_ON,
	BRIGHTNESS_CONTROL,
	BRIGHTNESS_DIMMING_BLINKING_CONTROL
} OutputDriverEffect;

const uint8_t CHANNELS = 8;

const uint8_t DEFAULT_ALL_CALL_I2C_SLAVE_ADDRESS = 0xE0;
const uint8_t DEFAULT_SWRST_I2C_SLAVE_ADDRESS    = 0x03;

const uint8_t REG_MODE1   = 0x00;
const uint8_t REG_MODE2   = 0x01;
const uint8_t REG_PWM0    = 0x02;
const uint8_t REG_GRPPWM  = 0x0A;
const uint8_t REG_GRPFREQ = 0x0B;
const uint8_t REG_LEDOUT0 = 0x0C;
const uint8_t REG_SUBADR1 = 0x0E;

const uint8_t REG_MODE1_MASK_SLEEP    = 0x10;
const uint8_t REG_MODE2_MASK_DMBLNK   = 0x20;

/*!
 * @brief Creates a new instance to control the PCA9634 chip on the default "all call" I2C address with the default TwoWire instance
 */
Pca9634::Pca9634() : i2cSlaveAddr(DEFAULT_ALL_CALL_I2C_SLAVE_ADDRESS), i2c(&Wire) {
}

/*!
 * @brief Creates a new instance to control the PCA9634 chip on the specified I2C address with the default TwoWire instance
 * @param i2cSlaveAddr The address
 */
Pca9634::Pca9634(const uint8_t i2cSlaveAddr) : i2cSlaveAddr(i2cSlaveAddr), i2c(&Wire) {
}

/*!
 * @brief Creates a new instance to control the PCA9634 chip on the specified I2C address with the given TwoWire instance
 * @param i2cSlaveAddr The address
 * @param i2c The TwoWire instance to be used
 */
Pca9634::Pca9634(const uint8_t i2cSlaveAddr, TwoWire &i2c) : i2cSlaveAddr(i2cSlaveAddr), i2c(&i2c) {
}

/*!
 * @brief Initializes the i2c module
 */
void Pca9634::begin() {
	i2c->begin();
}

/*!
 * @brief Triggers a software reset of the chip.
 * All the registers will be set to their power-up default value
 * @return True if there is an error, false otherwise
 */
bool Pca9634::reset() {
	i2c->beginTransmission(DEFAULT_SWRST_I2C_SLAVE_ADDRESS);
	i2c->write(0xA5);
	i2c->write(0x5A);
	return i2c->endTransmission() != 0;
}

/*!
 * @brief Puts the chip in sleep mode.
 * Don't call any other method while sleeping.
 */
void Pca9634::sleep() {
	uint8_t oldMode1 = readRegister(REG_MODE1);
	uint8_t newMode1 = oldMode1 | REG_MODE1_MASK_SLEEP;
	writeRegister(REG_MODE1, newMode1);
}

/*!
 * @brief Wakes up the chip.
 * There is a 500us delay that guarantees that all the functions and registers are available before the method returns.
 */
void Pca9634::wakeup() {
	uint8_t oldMode1 = readRegister(REG_MODE1);
	uint8_t newMode1 = oldMode1 & ~REG_MODE1_MASK_SLEEP;
	writeRegister(REG_MODE1, newMode1);
	delayMicroseconds(500);
}

/*!
 * @brief Sets the first I2C subaddress and activates it.
 * @param addr The address
 */
void Pca9634::setSubaddress1Active(const uint8_t addr) {
	this->setAddressActive(0, addr, true);
}

/*!
 * @brief Sets the second I2C subaddress and activates it.
 * @param addr The address
 */
void Pca9634::setSubaddress2Active(const uint8_t addr) {
	setAddressActive(1, addr, true);
}

/*!
 * @brief Sets the third I2C subaddress and activates it.
 * @param addr The address
 */
void Pca9634::setSubaddress3Active(const uint8_t addr) {
	setAddressActive(2, addr, true);
}

/*!
 * @brief Sets the "all call" I2C subaddress (active by default)
 * @param addr The address
 */
void Pca9634::setAllCallAddressActive(const uint8_t addr) {
	setAddressActive(3, addr, true);
}

/*!
 * @brief Disables the "all call" I2C address (active by default)
 */
void Pca9634::setAllCallAddressInactive() {
	setAddressActive(3, 0, false);
}

void Pca9634::setAddressActive(const uint8_t index, const uint8_t addr, boolean enabled) {
	if(enabled) {
		writeRegister(REG_SUBADR1 + index, (addr << 1) & 0xFF);
	}
	uint8_t oldMode1 = readRegister(REG_MODE1);
	uint8_t enableBit = 0x08 >> index;
	uint8_t newMode1;
	if(enabled) {
		newMode1 = oldMode1 | enableBit;
	} else {
		newMode1 = oldMode1 & ~enableBit;
	}
	writeRegister(REG_MODE1, newMode1);
}

/*!
 * @brief Configures the behavior of the output drivers
 * @param inverted Iif true, the outputs will be inverted (w.r.t the common anode config)
 * @param trigger When the outputs must change.  Change of the outputs at the STOP command allows synchronizing outputs of more than one PCA9634.
 * @param structure The output driver final transistors configuration. Some newer LEDs include integrated Zener diodes to limit voltage transients, reduce EMI and protect the LEDs, and these must be driven only in the open-drain mode to prevent overheating the IC.
 * @param output The desired output when ~OE=1
 */
void Pca9634::configureOutputs(const bool inverted, const OutputChangeTrigger trigger, const OutputDriverStructure structure, const OutputWhenNotEnabled output) {
	uint8_t oldMode2 = readRegister(REG_MODE2);
	uint8_t newMode2 = (oldMode2 & REG_MODE2_MASK_DMBLNK) | (inverted << 4) | (trigger << 3) | (structure << 2) | output;
	writeRegister(REG_MODE2, newMode2);
}

/*!
 * @brief Sets the parameters for the global dimming effect. 
 * This will disable the global blinking.
 * The effect must be applied to the single outputs with setEffectEnabled().
 * A 190 Hz fixed frequency signal is superimposed with the 97 kHz individual brightness control signal. 
 * @param ratio The dimming ratio. A value of 0.5 will half the output Vrms.
 */
void Pca9634::configureDimmingEffect(const float ratio) {
	uint8_t oldMode2 = readRegister(REG_MODE2);
	uint8_t newMode2 = oldMode2 & ~REG_MODE2_MASK_DMBLNK;
	writeRegister(REG_MODE2, newMode2);
	this->setGroupPwm(ratio);
}

/*!
 * @brief Sets the parameters for the global blinking effect. 
 * This will disable the global dimming
 * The effect must be applied to the single outputs with setEffectEnabled().
 * @param period The period, in seconds of the blinking effect. The value is clamped between 0.041 and 10.73.
 * @param dutyCycle The blinking duty cycle. A value of 0.1 will keep the output on for 10% of the time.
 * 
 */
void Pca9634::configureBlinkingEffect(const float period, const float dutyCycle) {
	uint8_t oldMode2 = readRegister(REG_MODE2);
	uint8_t newMode2 = oldMode2 | REG_MODE2_MASK_DMBLNK;
	writeRegister(REG_MODE2, newMode2);
	this->setGroupPwm(dutyCycle);
	writeRegister(REG_GRPFREQ, (int)clamp(24.0 * period - 1.0, 0.0, 255.0));
}

void Pca9634::setGroupPwm(const float ratio) {
	writeRegister(REG_GRPPWM, (int)clamp(ratio * 256.0, 0.0, 255.0));
}

float Pca9634::clamp(float value, float min, float max) {
	if(value > max) {
		return max;
	} else if(value < min) {
		return min;
	} else {
		return value;
	}
}

/*!
 * @brief Sets the brightness of a channel.
 * @param channel The channel (0 - 7) whose brighness must be set.
 * @param value The brightness intensity. The PWM duty cycle will be set to value / 256. If the effects are disabled for this channel, the maximum value is 256, 255 otherwise. The minimum value is 0.
 */
void Pca9634::setBrightness(const uint8_t channel, const uint16_t value) {
	if(channel >= CHANNELS) {
		return;
	}
	uint8_t ledoutRegisterAddress = REG_LEDOUT0 + channel / 4;
	uint8_t ledoutBitShift = (channel & 0x03) << 1;
	uint8_t ledoutRegisterValue = readRegister(ledoutRegisterAddress);
	OutputDriverEffect oldLedoutValue = (ledoutRegisterValue >> ledoutBitShift) & 0x03;
	uint16_t maxValue = oldLedoutValue == BRIGHTNESS_DIMMING_BLINKING_CONTROL ? 0XFF : 0X100;
    uint16_t clampedValue = value > maxValue ? maxValue : value;
	OutputDriverEffect newLedoutValue;
	if(clampedValue == 0X100) {
		newLedoutValue = FULLY_ON;
	} else if(oldLedoutValue == BRIGHTNESS_DIMMING_BLINKING_CONTROL) {
		newLedoutValue = BRIGHTNESS_DIMMING_BLINKING_CONTROL;
	} else {
		newLedoutValue = BRIGHTNESS_CONTROL;
	}
	if(newLedoutValue != oldLedoutValue) {
		writeRegister(ledoutRegisterAddress, ledoutRegisterValue & ~(0x03 << ledoutBitShift) | (newLedoutValue << ledoutBitShift));
	}
	if(newLedoutValue != FULLY_ON) {
		writeRegister(REG_PWM0 + channel, clampedValue);
	}
}

/*!
 * @brief Enables or disables the dimming/blinking effects for a channel.
 * @param channel The channel (0 - 7) whose flag must be set.
 * @param enabled Whether or not the effects must be enabled.
 */
void Pca9634::setEffectEnabled(const uint8_t channel, const bool enabled) {
	if(channel >= CHANNELS) {
		return;
	}
	uint8_t ledoutRegisterAddress = REG_LEDOUT0 + channel / 4;
	uint8_t ledoutBitShift = (channel & 0x03) << 1;
	uint8_t ledoutRegisterValue = readRegister(ledoutRegisterAddress);
	OutputDriverEffect oldLedoutValue = (ledoutRegisterValue >> ledoutBitShift) & 0x03;
	if(enabled) {
		if(oldLedoutValue == FULLY_ON) {
			writeRegister(REG_PWM0 + channel, 0xFF);
		}
		if(oldLedoutValue != BRIGHTNESS_DIMMING_BLINKING_CONTROL) {
			writeRegister(ledoutRegisterAddress, ledoutRegisterValue & ~(0x03 << ledoutBitShift) | (BRIGHTNESS_DIMMING_BLINKING_CONTROL << ledoutBitShift));
		}
	} else if(oldLedoutValue == BRIGHTNESS_DIMMING_BLINKING_CONTROL) {
		writeRegister(ledoutRegisterAddress, ledoutRegisterValue & ~(0x03 << ledoutBitShift) | (BRIGHTNESS_CONTROL << ledoutBitShift));
	}
}

uint8_t Pca9634::readRegister(const uint8_t registerAddress) {
	i2c->beginTransmission(i2cSlaveAddr);
	i2c->write(registerAddress & 0x1F);
	i2c->endTransmission();
	i2c->requestFrom(i2cSlaveAddr, (uint8_t)1);
	return i2c->read();
}

void Pca9634::writeRegister(const uint8_t registerAddress, const uint8_t value) {
	i2c->beginTransmission(i2cSlaveAddr);
	i2c->write(registerAddress & 0x1F);
	i2c->write(value);
	i2c->endTransmission();
}


