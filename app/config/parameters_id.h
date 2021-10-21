/**
 * @file parameters_id.h
 * @brief Definition of all the parameters id
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */
/* TODO: Add subgroups to describe the three sections in the instrument parameters */

#define NUM_MAX_OF_SENSOR		62 // Bus CAN can have 63 nodes on the bus but we reserved the node 1 for the mainboard and the Address 0x00 => A refaire

#ifndef INSTRUMENTS_PARAMETERS_ID_H_
#define INSTRUMENTS_PARAMETERS_ID_H_

/* Coordinate for view ID */

#define SETUP_PRM_X			0x0001 // x coordinate for view
#define SETUP_PRM_Y			0x0002 // y coordinate for view
#define SETUP_PRM_R			0x0003 // rotation for view

/* Parameter ID */

#define SETUP_PRM_COMM_METHOD           0x0004 // Parameter ID for COMM method
#define SETUP_PRM_COMM_METHOD_VERSION   0x0005 // Version of the specific communication method
#define SETUP_PRM_COMM_ADDR             0x0006 // Communication Address within the Interface
#define SETUP_PRM_COMM_FAIL_CONSEQUENCE 0x0007 // What needs to happen if this instrument fails
#define SETUP_PRM_SAMPLERATE            0x000A // Samplerate (1/s)
#define SETUP_PRM_DATA_INPUT_BYTES      0x0010 // Number of Input Bytes sent to the instrument
#define SETUP_PRM_DATA_INPUT_DATATYPE   0x0011 // Datatype of the input bytes
#define SETUP_PRM_DATA_OUTPUT_BYTES     0x0020 // Number of Outbut Bytes received from the instrument (No datatype)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE  0x0021 // Datatype of the output bytes
#define SETUP_PRM_SOFTWARE_FUNCTION     0x0100 // Software function to be executed in case of software instrument (No datatype)
#define SETUP_PRM_VIEW_ANGLE								0x010D // View angle for the sensor node.
#define SETUP_PRM_INTER_SENSOR_DISTANCE 					0x010E // interdistance between the sensor in a sensor node
#define SETUP_PRM_POLL_RANK									0x0111	 /* Poll rank of the sensor distance on the CAN BUS */
#define SETUP_PRM_JOYSTICK_ID								0x0200	/* The Joystick ID which the driving progile belong */
#define SETUP_PRM_PROFILE_NUMBER							0x0201	/* Profile number for a specific joystick */
#define SETUP_PRM_SHORT_THROW_TRAVEL						0x0202	/* Short throw travel */
#define SETUP_PRM_FORWARD_MAXIMUM_SPEED						0x0203	/* Maximum forward speed for each profile */
#define SETUP_PRM_BT_MAC_HIGH           0x0407 // Bluetooth MAC High
#define SETUP_PRM_BT_MAC_LOW            0x0408 // Bluetooth MAC Low

#define SETUP_PRM_PG_PARAMETER								0x0300
#define SETUP_PRM_DX_PARAMETER								0x0301

#define SETUP_PRM_HEIGHT									0x0302
#define SETUP_PRM_WEIGHT									0x0303

#define SETUP_PRM_TEMPLATE									0x0400
#define SETUP_PRM_TEMPLATE_TYPE								0x0401

#define SETUP_PRM_PWC_MAXIMUM_SPEED							0x0402	/* The maximum speed of the PWC wheelchair */

#define SETUP_PRM_OAS_SLOPE_START							0x0403
#define SETUP_PRM_OAS_SLOPE_PERCENTAGE						0x0404
#define SETUP_PRM_OAS_SLOPE_END								0x0405

#define SETUP_PRM_PWC_BOUNDARY_DISTANCE_CALIBRATION			0x0406

/* INSTRUMENT ID */

//#define SETUP_PRM_INSTRUMENT_1_ID       0x0101 // Sensor ID for input 1 of software instrument
//#define SETUP_PRM_INSTRUMENT_2_ID       0x0102 // Sensor ID for input 2 of software instrument
//#define SETUP_PRM_INSTRUMENT_3_ID       0x0103 // Sensor ID for input 3 of software instrument
//#define SETUP_PRM_INSTRUMENT_4_ID       0x0104 // Sensor ID for input 4 of software instrument

#define SETUP_PRM_SENSOR_1_ID 		0x0101	/**< Sensor ID for input 1 of software instrument   */
#define SETUP_PRM_SENSOR_2_ID 		0x0102	/**< Sensor ID for input 2 of software instrument   */
#define SETUP_PRM_SENSOR_3_ID 		0x0103	/**< Sensor ID for input 3 of software instrument   */
#define SETUP_PRM_SENSOR_4_ID 		0x0104	/**< Sensor ID for input 4 of software instrument   */
#define SETUP_PRM_SENSOR_5_ID 		0x0105	/**< Sensor ID for input 5 of software instrument   */
#define SETUP_PRM_SENSOR_6_ID		0x0106	/**< Sensor ID for input 6 of software instrument   */
#define SETUP_PRM_SENSOR_7_ID 		0x0107	/**< Sensor ID for input 7 of software instrument   */
#define SETUP_PRM_SENSOR_8_ID		0x0108	/**< Sensor ID for input 8 of software instrument   */

/* COMM METHOD value */

#define SETUP_PRM_COMM_METHOD_UART                     0x3F800000 // UART (Unused for now)
#define SETUP_PRM_COMM_METHOD_CAN                      0x40000000 // CAN (Unused for now)
#define SETUP_PRM_COMM_METHOD_SPI                      0x40400000 // SPI (Unused for now)
#define SETUP_PRM_COMM_METHOD_JOYSTICK_DYNAMIC_CONTROL 0x41800000 // Joystick Dynamic Control
#define SETUP_PRM_COMM_METHOD_JOYSTICK_PENNY_GILES     0x41880000 // Joystick Penny & Giles
#define SETUP_PRM_COMM_METHOD_JOYSTICK_LINX            0x41900000 // Joystick LINX
#define SETUP_PRM_COMM_METHOD_IMU                      0x42000000 // IMU
#define SETUP_PRM_COMM_METHOD_GPS                      0x42800000 // GPS
#define SETUP_PRM_COMM_METHOD_USBAD                    0x42A00000 // UPLINK communication Tablet to the mainboard during the running of the application
#define SETUP_PRM_COMM_METHOD_CAN_DISTANCE_SENSOR      0x42C00000 // CAN Distance Sensor
#define SETUP_PRM_COMM_METHOD_RTC                      0x43000000 // Real Time Clock (RTC)
#define SETUP_PRM_COMM_METHOD_BT                       0x40800000 // Bluetooth

/* COMM METHOD VERSION value */

// NP FOR NOW

/* COMM FAIL value */

#define SETUP_PRM_COMM_FAIL_CONSEQUENCE_DO_NOTHING                0x00000000 // Do nothing
#define SETUP_PRM_COMM_FAIL_CONSEQUENCE_STOP_SOFTWARE_INSTRUMENTS 0x3F800000 // Stop software instruments
#define SETUP_PRM_COMM_FAIL_CONSEQUENCE_STOP_VISUALISATION        0x40000000 // Stop visualisation
#define SETUP_PRM_COMM_FAIL_CONSEQUENCE_STOP_MEASUREMENTS         0x40400000 // Stop measurements
#define SETUP_PRM_COMM_FAIL_CONSEQUENCE_POWER_CUT_OFF             0x40800000 // Power cut off (if allowed by Ethical Commision)

/* INPUT DATATYPE value */

#define SETUP_PRM_DATA_INPUT_DATATYPE_NONE 0x00000000 // None

/* OUTPUT DATATYPE value */





















#define SETUP_PRM_DATA_OUTPUT_DATATYPE_NONE                   0x00000000 //   0 None
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKDX2OUTPUT      0x43210000 // 161 JOYSTICK_DX2_OUTPUT (0xA1)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKPGOUTPUT       0x43220000 // 162 JOYSTICK_PG_OUTPUT (0xA2)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKLINXOUTPUT     0x43230000 // 163 JOYSTICK_LINX_OUTPUT (0xA3)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU9AXISROTVEC         0x43310000 // 177 IMU_9AXIS_ROT_VEC (0xB1)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU6AXIS               0x43320000 // 178 IMU_6AXIS (0xB2)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT             0x43330000 // 179 IMU_QUAT_BATT (Quaternions + Batt only) (0xB3)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT                0x43340000 // 180 IMU_QUAT (Quaternions only) (0xB4)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC       0x43350000 // 181 IMU_QUAT_GYRO_ACC (Quaternions + Gyroscope + Accelerometer) (0xB5)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ 0x43360000 // 182 IMU_QUAT_GYRO_ACC_100HZ (Quaternions + Gyroscope + Accelerometer @ 100Hz) (0xB6)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSMINDATA             0x43410000 // 193 GPS_MIN_DATA (0xC1)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSSTATUS              0x43420000 // 194 GPS_STATUS (0xC2)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSDATASTATUS          0x43430000 // 195 GPS_DATA_STATUS (0xC3)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_MASK  0x43500000
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D1    0x43510000 // 209 CAN_DISTANCE_NODES D1 (0xD1) (US)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D2    0x43520000 // 210 CAN_DISTANCE_NODES D2 (0xD2) (IR)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D3    0x43530000 // 211 CAN_DISTANCE_NODES D3 (0xD3) (US+IR)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D4    0x43540000 // 212 CAN_DISTANCE_NODES D4 (0xD4) (US+2IR)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D5    0x43550000 // 213 CAN_DISTANCE_NODES D5 (0xD5) (US+3IR)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D6    0x43560000 // 214 CAN_DISTANCE_NODES D5 (0xD6) (4IR)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D7    0x43570000 // 215 CAN_DISTANCE_NODES D5 (0xD7) (4IR) Only calculated value
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D8	  0x43580000
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_RTC                    0x43610000 // 225 Real Time Clock (RTC) (0xE1)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD                  0x43710000 // 241 USB AD as Instrument (0xF1)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD_SENSOR           0x43720000 // 242 USB AD as Instrument + Sensor active bits (0xF2)
#define SETUP_PRM_DATA_OUTPUT_DATATYPE_AAMS                   0x44000000 // 512 AAMS Datatype

/* SOFTWARE FUNCTION value */

#define SETUP_PRM_SOFTWARE_FUNCTION_NO_FUNCTION                               0x00000000 // No function
#define SETUP_PRM_SOFTWARE_FUNCTION_SIMPLE_FUNCTION_THAT_CALCULATES_MIN_VALUE 0x3F800000 // Simple function that calculates MIN value
#define SETUP_PRM_SOFTWARE_FUNCTION_SIMPLE_FUNCTION_THAT_CALCULATES_MAX_VALUE 0x40000000 // Simple function that calculates MAX value
#define SETUP_PRM_SOFTWARE_FUNCTION_OAS_WITH_4_SENSORS                        0x40400000 // OAS with 4 Sensors

/* Define the length of each information */

#define LENGTH_IN_BYTE_PARAMETER_ID				0x02
#define LENGTH_IN_BYTE_PARAMETER_VALUE			0x04

//TODO when remove the part below: use the doxygen template for the code we added on top
#if 0
/** @defgroup inst-param Instrument parameters
  * @brief Device specific parameters
  */

/** @addtogroup inst-param
 * @{
 */
/* HMI/Android device specific parameters */
#define HMI_PARAMETER_X 0x0001 /**< This ID describes a x coordinate for tablet view parameter */
#define HMI_PARAMETER_Y 0x0002 /**< This ID describes a y coordinate for tablet view parameter */
#define HMI_PARAMETER_Y 0x0003 /**< This ID describes a rotation for tablet view parameter    */
/**
 * @}
 * */

/** @addtogroup inst-param
 * @{
 */
/* Basic instrument parameters */
#define BYTE_INPUT_PARAMETER_ID 				0x0010  /**< Number of Input Bytes sent to the instrument				    		*/
#define BYTE_OUTPUT_PARAMETER_ID 				0x0020  /**< Number of Outbut Bytes received from the instrument		  		    */
#define SETUP_PRM_COMM_METHOD 					0x0004 	/**< Communication Interface: Number of the interface (See Table 3)         */
#define SETUP_PRM_COMM_METHOD_VERSION 			0x0005  /**< Communication Address for the Interface 						        */
#define SETUP_PRM_COMM_ADDR 					0x0006  /**< What needs to happen if this instrument fails (See Table 4)   		    */
#define SETUP_PRM_COMM_FAIL_CONSEQUENCE 		0x0007  /**< What needs to happen if this instrument fails (See Table 4)   		    */
#define SETUP_PRM_SAMPLERATE 					0x000A  /**< Samplerate (1/s), frequency in which the instrument should be polled   */
#define SETUP_PRM_DATA_INPUT_BYTES 				0x0010  /**< Number of Input Bytes sent to the instrument							*/
#define SETUP_PRM_DATA_INPUT_DATATYPE			0x0011  /**< Datatype of the input bytes											*/
#define SETUP_PRM_DATA_OUTPUT_BYTES 			0x0020  /**< Number of Outbut Bytes received from the instrument (No datatype)		*/
#define SETUP_PRM_DATA_OUTPUT_DATATYPE 			0x0021  /**< Datatype of the output bytes											*/
#define SETUP_PRM_SOFTWARE_FUNCTION 			0x0100  /**< Software function to be executed in case of software instrument (No datatype)*/


/**
 * @}
 * */

/** @addtogroup inst-param
 * @{
 */
/* Software instrument specific parameters */
#define SOFTWARE_FUNCTION_PARAMETER_ID 0x0100 	/**< Software function to be executed in case of software instrument (TABLE 5) */
#define SETUP_PRM_SENSOR_1_ID 		0x0101	/**< Sensor ID for input 1 of software instrument   */
#define SETUP_PRM_SENSOR_2_ID 		0x0102	/**< Sensor ID for input 2 of software instrument   */
#define SETUP_PRM_SENSOR_3_ID 		0x0103	/**< Sensor ID for input 3 of software instrument   */
#define SETUP_PRM_SENSOR_4_ID 		0x0104	/**< Sensor ID for input 4 of software instrument   */
#define SETUP_PRM_SENSOR_5_ID 		0x0105	/**< Sensor ID for input 5 of software instrument   */
#define SETUP_PRM_SENSOR_6_ID		0x0106	/**< Sensor ID for input 6 of software instrument   */
#define SETUP_PRM_SENSOR_7_ID 		0x0107	/**< Sensor ID for input 7 of software instrument   */
#define SETUP_PRM_SENSOR_8_ID		0x0108	/**< Sensor ID for input 8 of software instrument   */
/**
 * @}
 * */
#endif

#endif /* INSTRUMENTS_PARAMETERS_ID_H_ */
