/**
 ******************************************************************************
 * @file    application.cpp
 * @authors  Satish Nair, Zachary Crockett and Mohit Bhoite
 * @version V1.0.0
 * @date    05-November-2013
 * @brief   Tinker application
 ******************************************************************************
  Copyright (c) 2013 Spark Labs, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/  
#include "application.h"
#include <math.h>
#include <string.h> 
#include "HttpClient.h"
#include "idDHT22.h"

/* Function prototypes -------------------------------------------------------*/

int startWatering(String wateringLocation);
int saveConfiguration(String configParameter);
int IsTime();
void poll();
void checkConfig();
void ReadAirSensorToVariables();


SYSTEM_MODE(AUTOMATIC);

#define TIMECTL_MAXTICKS  4294967295L
#define TIMECTL_INIT      0

String deviceToken = "GreenHouseDevice_2424";

HttpClient http;
http_request_t request;
http_response_t response;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
	{ "Content-Type", "text/html" },
	//  { "Accept" , "application/json" },
	{ "Accept", "*/*" },
	{ NULL, NULL } // NOTE: Always terminate headers will NULL
};


//exposed variables
/*
0 = Booted, configuration needed
1 = Monitoring / normal operation
2 = Automated Watering
3 = Manual Watering
*/
int currentState = 0; 
double currentTemperature = 0;
double currentAirHumidity = 0;
double currentLightLevel = 0;
double currentSoilHumidityZone1 = 0;
double currentSoilHumidityZone2 = 0;
double currentDewpoint = 0;


///exposed configuration
String postBackUrl = "oele.azurewebsites.net";
int postBackPort = 80;
int pollingEnabled = -1;
unsigned long pollTimeMark = 0;  //a millisecond time stamp used by the IsTime() function. initialize to 0
unsigned long int pollTimeInterval = 0;  //How many milliseconds we want for the interval cycle.
int wateringAreaOneMaxHumidity = 0;
int wateringAreaOneMinHumidity = 0;
int wateringAreaTwoMaxHumidity = 0;
int wateringAreaTwoMinHumidity = 0;

/*
DHT22 Temp and Humidity Sensor
*/
// declaration for DHT22 handler
int idDHT22pin = D0; //Digital pin for comunications
void dht22_wrapper(); // must be declared before the lib initialization
// DHT instantiate
idDHT22 DHT22_Air(idDHT22pin, dht22_wrapper);


void setup(void) {
	Serial.begin(9600);

	Spark.variable("currentState", &currentState, INT);
	Spark.variable("airtemp", &currentTemperature, DOUBLE);
	Spark.variable("airHumidity", &currentAirHumidity, DOUBLE);
	Spark.variable("airDewpoint", &currentDewpoint, DOUBLE);
	Spark.variable("lightLevel", &currentLightLevel, DOUBLE);
	Spark.variable("soilHumZone1", &currentSoilHumidityZone1, DOUBLE);
	Spark.variable("soilHumZone2", &currentSoilHumidityZone2, DOUBLE);
	
	
	Spark.function("water", startWatering);
	Spark.function("saveConfig", saveConfiguration);

	Serial.println("Setup finished");
}

// This wrapper is in charge of calling
// mus be defined like this for the lib work
void dht22_wrapper() {
	DHT22_Air.isrCallback();
}

void ReadAirSensorToVariables(){
	
	//Serial.println("Retrieving information from Air sensor: ");
	
	DHT22_Air.acquire();

	while (DHT22_Air.acquiring())
		;

	int result = DHT22_Air.getStatus();
	switch (result)
	{
		case IDDHTLIB_OK:

		//	Serial.println("OK");
		
			currentAirHumidity = DHT22_Air.getHumidity();

		//	Serial.print("Humidity (%): ");
		//	Serial.println(currentAirHumidity, 2);

			currentTemperature = DHT22_Air.getCelsius();
		//	Serial.print("Temperature (oC): ");
		//	Serial.println(currentTemperature, 2);

			currentDewpoint = DHT22_Air.getDewPoint();
		//	Serial.print("Dew Point (oC): ");
		//	Serial.println(currentDewpoint);
			break;
	
		case IDDHTLIB_ERROR_CHECKSUM:
			Serial.println("Error\n\r\tChecksum error");
			break;
		case IDDHTLIB_ERROR_ISR_TIMEOUT:
			Serial.println("Error\n\r\tISR Time out error");
			break;
		case IDDHTLIB_ERROR_RESPONSE_TIMEOUT:
			Serial.println("Error\n\r\tResponse time out error");
			break;
		case IDDHTLIB_ERROR_DATA_TIMEOUT:
			Serial.println("Error\n\r\tData time out error");
			break;
		case IDDHTLIB_ERROR_ACQUIRING:
			Serial.println("Error\n\r\tAcquiring");
			break;
		case IDDHTLIB_ERROR_DELTA:
			Serial.println("Error\n\r\tDelta time to small");
			break;
		case IDDHTLIB_ERROR_NOTSTARTED:
			Serial.println("Error\n\r\tNot started");
			break;
		default:
			Serial.println("Unknown error");
			break;
	}
}

int IsTime(unsigned long *timeMark, unsigned long timeInterval){
	unsigned long timeCurrent;
	unsigned long timeElapsed;
	int result = false;

	timeCurrent = millis();
	if (timeCurrent<*timeMark) {  //Rollover detected
		timeElapsed = (TIMECTL_MAXTICKS - *timeMark) + timeCurrent;  //elapsed=all the ticks to overflow + all the ticks since overflow
	}
	else {
		timeElapsed = timeCurrent - *timeMark;
	}

	if (timeElapsed >= timeInterval) {
		*timeMark = timeCurrent;
		result = true;
	}
	return(result);
}

void poll(){

	request.hostname = postBackUrl;
	request.port = postBackPort;
	request.path = "/poll";

	//request.body = "token=" + deviceToken;
	http.get(request, response, headers);
	Serial.println(response.status);
	Serial.println(response.body);
}

int startWatering(String wateringLocation)
{
	if (wateringLocation == "First"){
		Serial.println("WATERING: Zone 1");
		return 1; //Todo: Execute watering process
	}
	if (wateringLocation == "Second"){
		Serial.println("WATERING: Zone 2");
		return 2; //Todo: Execute watering process
	}
	return -1;
}


int saveConfiguration(String configParameter)
{
	if (configParameter.startsWith("$pollingon:")){
		int startIndex = configParameter.lastIndexOf(":") + 1;
		pollingEnabled = configParameter.substring(startIndex, configParameter.length()).toInt();
		Serial.println("Config: pollingEnabled set");
		checkConfig();
		return 2;
	}
	if (configParameter.startsWith("$interval:")){
		int startIndex = configParameter.lastIndexOf(":") + 1;
		pollTimeInterval = configParameter.substring(startIndex, configParameter.length()).toInt();
		pollTimeInterval = pollTimeInterval * 60 * 1000;
		Serial.println("Config: postback interval set");
		checkConfig();
		return 3;
	}
	if (configParameter.startsWith("$water1max:")){
		int startIndex = configParameter.lastIndexOf(":") + 1;
		wateringAreaOneMaxHumidity = configParameter.substring(startIndex, configParameter.length()).toInt();
		Serial.println("Config: max soil humidity set for area one");
		checkConfig();
		return 4;
	}
	if (configParameter.startsWith("$water1min:")){
		int startIndex = configParameter.lastIndexOf(":") + 1;
		wateringAreaOneMinHumidity = configParameter.substring(startIndex, configParameter.length()).toInt();
		Serial.println("Config: min soil humidity set for area one");
		checkConfig();
		return 5;
	}
	if (configParameter.startsWith("$water2max:")){
		int startIndex = configParameter.lastIndexOf(":") + 1;
		wateringAreaTwoMaxHumidity = configParameter.substring(startIndex, configParameter.length()).toInt();
		Serial.println("Config: max soil humidity set for area two");
		checkConfig();
		return 6;
	}
	if (configParameter.startsWith("$water2min:")){
		int startIndex = configParameter.lastIndexOf(":") + 1;
		wateringAreaTwoMinHumidity = configParameter.substring(startIndex, configParameter.length()).toInt();
		Serial.println("Config: min soil humidity set for area two");
		checkConfig();
		return 7;
	}
	return -1;
}

void checkConfig(){

	//Check if all parameters have been set
	if (pollTimeInterval != 0
		&& pollingEnabled != -1
		&& wateringAreaOneMaxHumidity != 0
		&& wateringAreaOneMinHumidity != 0
		&& wateringAreaTwoMaxHumidity != 0
		&& wateringAreaTwoMinHumidity != 0){
		currentState = 1;
		Serial.println("Config final: ");
	}
	else{
		Serial.println("Config not final: ");
	}
}

void loop() {
	delay(1000);
	Serial.println("Loop");

	//Serial.println("Read in Variables");

	ReadAirSensorToVariables();

	currentSoilHumidityZone1 = random(30, 29);
	currentSoilHumidityZone2 = random(40, 29);
	currentLightLevel = random(50, 29);

	
	if (pollingEnabled == 1 && (currentState == 0 || IsTime(&pollTimeMark, pollTimeInterval))) {  //Is it time to call home?
		poll();
	}
	//other code here
}
