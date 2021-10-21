/**
 * @file raw.h
 * @brief Functions around the raw configuration - decoding
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */

#ifndef CONFIG_RAW_H_
#define CONFIG_RAW_H_

#ifndef DECODED_H_
#define DECODED_H_

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "parameters_id.h"

#define RAW_CONFIGURATION_TEST 0

#define NUMBER_OF_INSTRUMENTS 16

#define SOH 		0x01
#define STX 		0x02
#define ETX			0x03
#define EOT			0x04

#define LENGTH_IN_BYTE_PARAMETER_ID				0x02
#define LENGTH_IN_BYTE_PARAMETER_VALUE			0x04

typedef enum {
	COM_UART1 = 1,
	COM_UART2,
	COM_UART3,
	COM_UART4,
	COM_UART5,
	COM_UART6,
	COM_UART7,
	COM_UART8,
	// CAN COMM INTERFACE = 16 Accordignly to KU Leuven specification
	COM_CAN = 16,
	COM_SPI,
	COM_SOFTWARE,

	NUMBER_OF_COMMUNICATION_INTERFACE

} COMMUNICATION_INTERFACE;

typedef enum {
	COM_FAILURE_DO_NOTHING = 0,
	COM_FAILURE_STOP_SOFTWARE_INSTRUMENTS,
	COM_FAILURE_STOP_VISUALISATION,
	COM_FAILURE_STOP_MEASUREMENTS,
	COM_FAILURE__POWER_CUT_OFF,

	NUMBER_OF_COMMUNICATION_FAILURE_CONSEQUENCE

} COMMUNICATION_FAILURE_CONSEQUENCE;

typedef struct {

	uint16_t instrumentID;
	uint16_t numberOfParameters;

	// Coordinate for view
	int32_t 	x;
	int32_t 	y;
	int32_t 	r;

	// Communication Method
	uint32_t 			comMethod;
	uint32_t 			comMethodVersion;

	// Communication fail consequence
	uint32_t  	comFail;

	// Communication Address
	uint32_t comAddress;

	// SampleRate
	uint32_t sampleRate;

	// Datatype type input and output
	uint32_t dataTypeInput;
	uint32_t dataTypeOutput;

	// Data Input and Output

	uint32_t dataOutput;
	uint32_t dataInput;

	// Measurement status
	uint32_t measurementStatus;
	// Instrument Status
	uint32_t instrumentStatus;

	/* Maybe Add a tab of sensor, index of the tab sensor can be the ID of the sensors ?
	*int32_t Sensor_ID[NUM_MAX_OF_SENSOR];
	* For now just 4 variable for 4 sensor */
	uint32_t sensorID1;
	uint32_t sensorID2;
	uint32_t sensorID3;
	uint32_t sensorID4;
	uint32_t sensorID5;
	uint32_t sensorID6;
	uint32_t sensorID7;
	uint32_t sensorID8;

	// Information relative to the sensor
	uint32_t viewAngle;
	uint32_t interSensorDistance;

	uint32_t pollRank;

	uint16_t outputDataSize; //in bytes

	uint32_t joystickID;
	uint32_t profileNumber;
	uint32_t shortThrowTravel;
	uint32_t maximumForwardSpeed;
	uint32_t pwcMaximumSpeed;

	uint32_t pwcBoundaryCalibration;

	 // Data Bluetooth MAC address
	 uint32_t BTMACHigh;
	 uint32_t BTMACLow;

	//software function not implemented yet
	uint32_t softFuncToExec;
	void (*execFunction)(void);

	int (*parseBinary)(void * data, unsigned char *dest); /* Pointer to datatype to raw parsing function */
	int (*parseAscii)(unsigned int instrumentType, void * pData, unsigned char *dest, unsigned int lineReturn);
	int (*decodeData)(unsigned char *source, unsigned int size, void * pData);
	void *data; /* Void type, malloc is done once the config is decoded, have to cast everytime we want to access it */

} instrument_config_t;


typedef enum
{
	DECODE_SUCCESS = 0,
	EDATA		   = -1,
	NO_SOH 		   = -2,
	NO_STX 	       = -3,
	NO_ETX         = -4,
	NO_EOT 	       = -5

} decode_result;

typedef struct {

	uint16_t setupID;
	uint16_t version;
	uint16_t companyID;

	uint16_t numberOfInstruments;

	instrument_config_t *instruments;

	uint32_t cycleCounter;

} decoded_config_t;


/** Public API */
decode_result decode_config(const uint8_t *buffer, decoded_config_t *config);
int raw_config_test(void);
void print_config(decoded_config_t config);

#endif /* DECODED_H_ */
#endif /* CONFIG_RAW_H_ */
