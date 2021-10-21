/**
 * @file structures.h
 */
#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#include <stdbool.h>
#include <time.h>

#define MAX_NUMBER_OF_DISTANCE_SENSORS 32
#define DEFAULT_DISTANCE_NODE_

/*! PWC's type enumeration */
typedef enum {
	PWC_TYPE_DX 	= 0,  	/**< Powered wheelchair type DynamicControl's DX2 */
	PWC_TYPE_PG 	= 1,  	/**< Powered wheelchair type Penny & Giles R-NET  */
	PWC_TYPE_LINX 	= 2,	/**< Powered wheelchair type DynamicControl's DX2 */

	PWC_TYPE_TOTAL     		/**< Number of PWC enumeration entries 			  */
} pwc_type_e;

/**
 * Structure that contains PWC Data
 */
typedef struct {
	pwc_type_e 		pwcType;		/**< Type of Powered Wheelchair (see pwc_type_e enumeration)  */

	unsigned int	timestamp;		/**< Timestamp as milliseconds elapsed since t0 (bootup time) */

	uint16_t 			estimatedSpeed; 	/**< Calculated value for the actual speed (m/s) */

	int8_t 			turn; 			/**< Joystick position (cartesian): X */
	int8_t 			speed;			/**< Joystick position (cartesian): Y */
	unsigned int 	mode;			/**< Mode selected */
	uint8_t			profile;		/**< Profile selected */

	uint16_t		trueSpeed;		/**< True speed in m/s (only for Penny and Giles) */
	bool			actuatorMode;	/**< Actuator mode selected */

	unsigned int    isOnline; 		/**< 0: PWC turned off; 1: PWC turned ON */

} pwc_data_t;

typedef struct
{
	uint32_t maximumForwardSpeed;
	uint32_t shortThrowTravel;
	/* Add maximum reverse speed */
}pwc_profile_info_t;


typedef struct {
	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	float			lon;
	float			lat;
	float			hMSL; 	  /* Height above sea level */
	float			gSpeed;   /* Ground speed */

	bool 			gnssFixOK; /* 1 = valid fix (i.e within DOP & accuracy masks) */

} gps_data_t;



/* Generic structure */
typedef struct {
	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
//	unsigned int	timestamp; // removed to optimize data size to sensor queue

	float x;
	float y;
	float z;
} three_axis_data_t;

typedef enum  {
	NODE_D0 = 0,
	NODE_D1,
	NODE_D2,
	NODE_D3,
	NODE_D4,
	NODE_D5

}EDUCAT_DISTANCE_NODE_VERSION;

typedef struct {
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	unsigned int distance;
	void *pData;
	EDUCAT_DISTANCE_NODE_VERSION version;
	unsigned int position;
	unsigned int angle;

} distance_node_data_t;

typedef struct {
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	unsigned int distance;
	unsigned int position;
	unsigned int angle;

} can_distance_node_d1; /* 1 US */

typedef struct {
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	unsigned int distance;
	unsigned int position;
	unsigned int angle;

} can_distance_node_d2; /* 1 IR */

typedef struct {
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	unsigned int distanceUS;
	unsigned int distanceIR;

	unsigned int calculatedDistance;

	unsigned int position;
	unsigned int angle;

} can_distance_node_d3; /* 1 US + 1 IR */

typedef struct {
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	unsigned int distanceUS;
	unsigned int distanceIR1;
	unsigned int distanceIR2;

	unsigned int calculatedDistance;

	unsigned int position;
	unsigned int angle;

} can_distance_node_d4; /* 1 US + 2 IR */


typedef struct {
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	unsigned int distanceUS;
	unsigned int distanceIR1;
	unsigned int distanceIR2;
	unsigned int distanceIR3;

	unsigned int calculatedDistance;

	unsigned int position;
	unsigned int angle;

} can_distance_node_d5; /* 1 US + 3 IR */



typedef struct {
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;

	unsigned int distanceIR1;
	unsigned int distanceIR2;
	unsigned int distanceIR3;
	unsigned int distanceIR4;

	unsigned int calculatedDistance;

	unsigned int position;
	unsigned int angle;

} can_distance_node_d6; /* 1 US + 3 IR */

typedef struct{
	unsigned int ID;

	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	timestamp;
	unsigned int calculatedDistance;

	unsigned int position;
	unsigned int angle;

}can_distance_node_d7; /* 4IR sensor with only the calculated Distance */


/* Generic structure */
typedef struct {
	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
//	unsigned int	timestamp; // removed to minimize data to sensor queue

	float real;
	float i;
	float j;
	float k;

} rot_vector_data_t;

typedef struct {
	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	    timestamp;

	three_axis_data_t	accelerometer;
	three_axis_data_t	gyroscope;
	three_axis_data_t	magnetometer;
	rot_vector_data_t	rotVectors;

} imu_data_t;

typedef struct {
	/* Timestamp as milliseconds elapsed since t0 (bootup time) */
	unsigned int	    timestamp;

	three_axis_data_t	accelerometer1;
	three_axis_data_t	gyroscope1;
	rot_vector_data_t	rotVectors1;
	three_axis_data_t	accelerometer2;
	three_axis_data_t	gyroscope2;
	rot_vector_data_t	rotVectors2;

} imu_100Hz_data_t;

typedef struct {
	unsigned int nDistNodes;
	distance_node_data_t distanceNodes[MAX_NUMBER_OF_DISTANCE_SENSORS];

} distance_node_list_t;

typedef struct {
	unsigned int     timestamp;

	pwc_data_t pwc;
	imu_data_t imu;

	gps_data_t gps;

	distance_node_list_t distList;

	/* Also save the last 4 values */
	imu_data_t imuDataHistory[4];
	/* Allows to increment the pointer to be incremented with an overflow @3++ */
	unsigned int imuDataHistoryPointer : 2;


} data_container_t;
/**
 * @struct usbad_data_t structure.h
 * @brief Android Device as instrument dataType
 *
 * Data struct used to contain the data return by the EDUCAT Android
 * device
 */
typedef struct
{
	uint32_t cycleCounter;

	uint8_t OAScalculatedValue; /*!< Calculated value return by the OAS */

	union
	{
		uint8_t feedbackStatue; /*!< Statue of the feedback enable for the OAS */
		struct {
			uint8_t buzzer : 1;
			uint8_t haptic : 1;
			uint8_t visual : 1;
			uint8_t padding : 5;
		};
	};

	union
	{
		uint8_t sensorActivate; /*!< Give the information of which sensor has been activated or disactivated */
		struct{
			uint8_t sensor1ActivateBit : 1;
			uint8_t sensor2ActivateBit : 1;
			uint8_t sensor3ActivateBit : 1;
			uint8_t sensor4ActivateBit : 1;
			uint8_t sensor5ActivateBit : 1;
			uint8_t sensor6ActivateBit : 1;
			uint8_t sensor7ActivateBit : 1;
			uint8_t sensor8ActivateBit : 1;
		};
	};
}usbad_data_t;


#endif /* DATA_STRUCTURES_H_ */
