#include <math.h>
#include <TemperatureSensor.h>
#include <ReefuinoRelay.h>
#include <ReefuinoThermostat.h>

#define ThermistorPIN 0   // Analog Pin 0
#define RelayChillerPIN 9 
#define RelayHeaterPIN 8

double temperatureToKeep = 24.0;

TemperatureSensor temperatureSensor(ThermistorPIN);
ReefuinoRelay chillerRelay(RelayChillerPIN);
ReefuinoRelay heaterRelay(RelayHeaterPIN);
ReefuinoThermostat thermostat(temperatureSensor, chillerRelay, heaterRelay, temperatureToKeep);

void setup() {
  Serial.begin(115200);  
  
  Serial.print("Temperature is set to: ");   
    Serial.println(temperatureToKeep, DEC);   
}

void loop() {
  double temp = thermostat.checkTemperature(); 
  
  if(thermostat.isHeating()){
    Serial.println("Heater is on");    
  } else if(thermostat.isChilling()){
    Serial.println("Chiller is on");    
  } else {
    Serial.println("Temperature control is taking a nap");    
  }
  delay(1000); 
}

