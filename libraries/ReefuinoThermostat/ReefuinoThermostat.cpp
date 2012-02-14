#include "ReefuinoThermostat.h"
#include "TemperatureSensor.h"
#include "ReefuinoRelay.h"

const double actionBuffer = 0.5;
const double harmfullFactor = 1.5;

//Time to wait before taking an actions. Prevents frequent stops and starts due to short temperature fluctuations.
const long ACTIVATION_DELAY = 60000; //60 seconds

// Set a WARN status when the heater/chiller is operating for more than 2hs
static unsigned long DANGEROUS_OP_TIME_IN_SEC = 60 * 60 * 2; //2hs

unsigned long nextActivationInMillis = 0;
unsigned long lastActivationTime = 0;

bool shouldDelayActivation = false;

ReefuinoThermostat::ReefuinoThermostat(TemperatureSensor ts,
		ReefuinoRelay chillerRelay, ReefuinoRelay heaterRelay,
		double temperatureToKeep) :
		_tempToKeep(temperatureToKeep), _temperatureSensor(ts), _chillerRelay(

	status = RESTING;
}

ReefuinoThermostat::~ReefuinoThermostat() {/*nothing to destruct*/
}

float ReefuinoThermostat::checkTemperature() {
	float temp = _temperatureSensor.readCelsius(); // read ADC and  convert it to Celsius

	if (nextActivationInMillis > millis()) {
		Serial.println("delaying activation. \n");
		shouldDelayActivation = true;
	} else {
		shouldDelayActivation = false;
	}

	if (temp >= (_tempToKeep + actionBuffer)) {
		if (!shouldDelayActivation && status != CHILLING
				&& status != CHILLING_WARN) {
			_chillerRelay.turnOn();
			_heaterRelay.turnOff();
			status = CHILLING;
			resetActivationTimmer();
		} else {
			Serial.println("hot but waiting.");
		}

	} else if (temp < (_tempToKeep - actionBuffer)) {
		if (!shouldDelayActivation && status != HEATING
				&& status != HEATING_WARN) {
			_heaterRelay.turnOn();
			_chillerRelay.turnOff();
			status = HEATING;
			resetActivationTimmer();
		} else {
			Serial.println("cold but waiting.");
		}
	} else if (temp <= _tempToKeep || temp >= _tempToKeep) {
		if (!shouldDelayActivation && status != RESTING) {
			_chillerRelay.turnOff();
			_heaterRelay.turnOff();
			status = RESTING;
			resetActivationTimmer();
		} else {
			Serial.println("waiting to rest.");
		}
	}
	checkHarmfulOperationTime();

	return temp;
}

void ReefuinoThermostat::resetActivationTimmer() {
	lastActivationTime = millis();
	nextActivationInMillis = (long) millis() + ACTIVATION_DELAY;
}

long ReefuinoThermostat::getSecondsRemainingForNextActivation() {
	if (nextActivationInMillis > (millis() + 1000)) {
		long result = (nextActivationInMillis - millis());
		return result;
	} else {
		return 0;
	}
}

ReefuinoThermostat::~ReefuinoThermostat() {/*nothing to destruct*/
}

float ReefuinoThermostat::checkTemperature() {
	float temp = _temperatureSensor.readCelsius(); // read ADC and  convert it to Celsius

	if (nextActivationInMillis > millis()) {
		Serial.println("delaying activation. \n");
		shouldDelayActivation = true;
	} else {
		shouldDelayActivation = false;
	}
	if (temp >= (_tempToKeep + actionBuffer)) {
		if (!shouldDelayActivation && status != CHILLING
				&& status != CHILLING_WARN) {
			_chillerRelay.turnOn();
			_heaterRelay.turnOff();
			status = CHILLING;

			resetActivationTimmer();
		} else {
			Serial.println("hot but waiting.");
		}
	} else if (temp < (_tempToKeep - actionBuffer)) {
		if (!shouldDelayActivation && status != HEATING
				&& status != HEATING_WARN) {
			_heaterRelay.turnOn();
			_chillerRelay.turnOff();
			status = HEATING;

			resetActivationTimmer();
		} else {
			Serial.println("cold but waiting.");
		}
	} else if (temp <= _tempToKeep || temp >= _tempToKeep) {
		if (!shouldDelayActivation && status != RESTING) {
			_chillerRelay.turnOff();
			_heaterRelay.turnOff();
			status = RESTING;
			resetActivationTimmer();
		} else {
			Serial.println("waiting to rest.");
		}
	}
	checkHarmfulOperationTime();
	return temp;
}

void ReefuinoThermostat::resetActivationTimmer() {
	lastActivationTime = millis();
	nextActivationInMillis = (long) millis() + ACTIVATION_DELAY;
}

long ReefuinoThermostat::getSecondsRemainingForNextActivation() {
	if (nextActivationInMillis > (millis() + 1000)) {
		long result = (nextActivationInMillis - millis(tun	return result;
	} else {
		return 0;
	}
}

/**
 * Last time the thermostat activated either chiller or heater.
 * In milliseconds
 */
long ReefuinoThermostat::getLastActivationTime() {
	return lastActivationTime;
}

/**
 * Check whether the chiller or heater are working for a long time.
 * Can be used to prevent disasters due to broken equipment.
 */
void ReefuinoThermostat::checkHarmfulOperationTime() {
	if (status == HEATING && isWorkingTooLong()) {
		status = HEATING_WARN;
	} else if (status == CHILLING && isWorkingTooLong()) {
		status = CHILLING_WARN;
	}
}

/*Is the temperature dangerously high or low? */
bool ReefuinoThermostat::isHarmfulTemperature() {
	double temp = _temperatureSensor.readCelsius();
	if (temp >= _tempToKeep + harmfullFactor
			|| temp <= _tempToKeep - harmfullFactor) {
		return true;
	} else {
		return false;
	}
}

bool ReefuinoThermostat::isWorkingTooLong() {
	return ((millis() - lastActivationTime) / 1000) > DANGEROUS_OP_TIME_IN_SEC;
}

bool ReefuinoThermostat::isHeating() {
	return _heaterRelay.isOn();
}

bool ReefuinoThermostat::isChilling() {
	return _chillerRelay.isOn();
}

ThermostatStatus ReefuinoThermostat::getStatus() {
	return status;
}

String ReefuinoThermostat::getStatusStr() {
	return lookup[status];
}

