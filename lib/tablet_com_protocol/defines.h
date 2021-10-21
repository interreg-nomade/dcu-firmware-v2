/**
 * @file defines.h
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef CLOUD_PROT_DEFINES_H_
#define CLOUD_PROT_DEFINES_H_

#define MAX_CPL_PAYLOAD_LENGTH 255
#define MAX_CPL_FRAME_LENGTH 261

#define CPL_START_DELIMITER 0x68
#define CPL_END_DELIMITER 0x16

/* Timeout accepted when the parser tries to build a frame */
#define CPL_TIMEOUT_REF 250 /* 5 Ticks */ // stond op 100

/* List of the messages stored in DSPA/SSAP :
 * - REQ stands for Request
 * - ASW stands for Answer = answer to a request
 * - Stream means that the message is emitted without a requests
 */
#define CPL_MSG_TABLET_CON_REQ 0x0001
#define CPL_MSG_TABLET_CON_ASW 0x0002

/* Stream of the current configuration ID */
#define CPL_MSG_CONFIG_ID_STREAM 0x0003;

/* Request which mode to boot on */
#define CPL_MSG_MODE_REQ 0x0004
/* Answer which mode to boot on */
#define CPL_MSG_MODE_ASW 0x0005

/* Request if a config modification is required */
#define CPL_MSG_CONFIG_STATE_REQ 0x0006
/* Answer if a config modification is required */
#define CPL_MSG_CONFIG_STATE_ASW 0x0007

/* Stream config : one packet per instrument */
#define CPL_MSG_CONFIG_STREAM_ONE_INSTR    0x0008

/* Stream config over : request the saving */
#define CPL_MSG_CONFIG_STREAM_OVER    0x0009

/* Start measurement data stream */
#define CPL_MSG_START_MEASUREMENT_STREAM    0x0010

/* Start measurement data stream */
#define CPL_MSG_STOP_MEASUREMENT_STREAM    0x0011

/* Contains stream of one instrument */
#define CPL_MSG_MEASUREMENT_STREAM    0x0012

/* Commands */
#define CPL_MODE_ENGINEER   0x01
#define CPL_MODE_USER       0x00


/** @defgroup Flow control function options
 *  @{
 */
/* Flow control function codes */
#define FLOW_CONTROL_FUNCTION_CODE_NOFLOW 0x00
#define FLOW_CONTROL_FUNCTION_CODE_INIT	  0x02
#define FLOW_CONTROL_FUNCTION_CODE_DATA	  0x04
#define FLOW_CONTROL_FUNCTION_CODE_ACK	  0x06
/** @} */ // end of Flow control function options

/** @defgroup Services table
 *  @{
 */
/* Flow control function codes */
//TODO: include the rest of the table, present @ https://docs.google.com/spreadsheets/d/1x6ykCb45KlGHnkvCNAetLhMO48olEdmooSA2Db9HxsA/edit#gid=1394394354
// in the panel "SAP_TABLE"SERVICE_REQUEST_CONNECTION_CHECK
#define SERVICE_REQUEST_CONNECTION_CHECK 		0x0180  /*!< Connection check (Android dev. and Mainboard initiative) */
#define SERVICE_RESPONSE_CONNECTION_CHECK 		0x8001	/*!< Connection check (Android dev. and Mainboard initiative) */

#define SET_RTC_TIME                            0x0181  /* Android sends current date and time in epoch_64 format */

#define SERVICE_REQUEST_SETUP_ID		 		0x0280	/*!< Main Board sends current Setup ID if any. (Mainboard init.) */
#define SERVICE_RESPONSE_SETUP_ID			 	0x8002	/*!< Main Board sends current Setup ID if any. (Mainboard init.) */

#define SERVICE_REQUEST_SETUP_ID_FORCED 		0x0281	/*!< Main Board sends current Setup ID if any. (Mainboard init.) */
#define SERVICE_RESPONSE_SETUP_ID_FORCED	 	0x8102	/*!< Main Board sends current Setup ID if any. (Mainboard init.) */

#define SERVICE_REQUEST_NO_SETUP_ID		 		0x0281	/*!< At first boot, the mainboard has no configuration and will
															send 0 as Setup ID as a Forced service */
#define SERVICE_RESPONSE_NO_SETUP_ID			0x8102	/*!< At first boot, the mainboard has no configuration and will send 0 as Setup ID as a Forced service */

#define SERVICE_REQUEST_JSON_SETUP		 		0x0380	/*!< Send JSON Setup format request  (Android dev. initiative) */
#define SERVICE_RESPONSE_JSON_SETUP			 	0x8003	/*!< Send JSON Setup format request  (Android dev. initiative) */

#define SERVICE_REQUEST_RAW_SETUP		 		0x0480	/*!< Send Raw Setup format request (Android dev. initiative) */
#define SERVICE_RESPONSE_RAW_SETUP			 	0x8004	/*!< Send Raw Setup format request (Android dev. initiative) */

#define SERVICE_REQUEST_GET_JSON_SETUP   		0x0580	/*!< Get JSON Setup format */
#define SERVICE_RESPONSE_GET_JSON_SETUP  	 	0x8005	/*!< Get JSON Setup format */

#define SERVICE_REQUEST_SHUTDOWN		 		0xFE80	/*!< Main Board sends current Setup ID if any. (Mainboard init.) */
#define SERVICE_RESPONSE_SHUTDOWN			 	0x80FE	/*!< Main Board sends current Setup ID if any. (Mainboard init.) */

/* Data stream */
#define SERVICE_REQUEST_START_DATA_STREAM	 	0x1080	/*!< Android device requests for the start of data streaming   */
#define SERVICE_RESPONSE_START_DATA_STREAM		0x8010	/*!< Android device requests for the start of data streaming  */

#define SERVICE_REQUEST_PACKET_DATA_STREAM		0x1180	/*!< Mainboard streaming data packets */
#define SERVICE_RESPONSE_PACKET_DATA_STREAM		0x8011	/*!< Mainboard streaming data packets */

#define SERVICE_REQUEST_STOP_DATA_STREAM	 	0x1280	/*!< Android device requests to stop streaming data */
#define SERVICE_RESPONSE_STOP_DATA_STREAM		0x8012	/*!< Android device requests to stop streaming data */

#define SERVICE_INFORMATION_USBAD_INSTRUMENT	0x9280 /*!< Android device as instrument send information about the feedback from the android device */
//todo is a request and response needed no information about this part in the documentation.

/* Live Stream (updated on 2019-03-12) */
#define SERVICE_REQUEST_START_LIVE_STREAM	 	0x1880	/*!< Android device requests for the start of live streaming to bluetooth devices */
#define SERVICE_RESPONSE_START_LIVE_STREAM		0x8018	/*!< Android device requests for the start of live streaming to bluetooth devices */

#define SERVICE_REQUEST_PACKET_LIVE_STREAM	 	0x1980	/*!< Mainboard live streaming  packets to bluetooth devices */
#define SERVICE_RESPONSE_PACKET_LIVE_STREAM		0x8019	/*!< Mainboard live streaming  packets to bluetooth devices */

#define SERVICE_REQUEST_STOP_LIVE_STREAM	 	0x1A80	/*!< Android device requests to stop live streaming to bluetooth devices  */
#define SERVICE_RESPONSE_STOP_LIVE_STREAM		0x801A	/*!< Android device requests to stop live streaming to bluetooth devices */


/* Starting and stopping a measurement */
#define SERVICE_REQUEST_START_MEASUREMENT		0x2080	/*!< Start a measurement (this will include a measurement ID for storage) */
#define SERVICE_RESPONSE_START_MEASUREMENT		0x8020	/*!< Start a measurement (this will include a measurement ID for storage) */

#define SERVICE_REQUEST_STOP_MEASUREMENT	 	0x2180	/*!< Stop a measurement  */
#define SERVICE_RESPONSE_STOP_MEASUREMENT		0x8021	/*!< Stop a measurement */

#define SERVICE_REQUEST_SEND_MEASUREMENT_LIST	0x2280	/*!< Start a measurement (this will include a measurement ID for storage) */
#define SERVICE_RESPONSE_SEND_MEASUREMENT_LIST	0x8022	/*!< Start a measurement (this will include a measurement ID for storage) */

#define SERVICE_REQUEST_SET_MEASUREMENT_ID   	0x2380	/*!< Start a measurement (this will include a measurement ID for storage) */
#define SERVICE_RESPONSE_SET_MEASUREMENT_ID  	0x8023	/*!< Start a measurement (this will include a measurement ID for storage) */



/* Acyclic Buffered Data Stream (updated on 2019-03-12) */
#define SERVICE_REQUEST_START_ACYCLIC_DATA_STREAM	 	0x3080	/*!< Android device requests for the start of Buffered Data Stream */
#define SERVICE_RESPONSE_START_ACYCLIC_DATA__STREAM		0x8030	/*!< Android device requests for the start of Buffered Data Stream */

#define SERVICE_REQUEST_PACKET_ACYCLIC_DATA_STREAM	 	0x3180	/*!< Mainboard buffered data packets (As a Flow) */
#define SERVICE_RESPONSE_PACKET_ACYCLIC_DATA_STREAM		0x8031	/*!< Mainboard buffered data packets (As a Flow) */

#define SERVICE_REQUEST_STOP_ACYCLIC_DATA_STREAM	 	0x3280	/*!< Android device requests to stop the Buffered Data Stream  */
#define SERVICE_RESPONSE_STOP_ACYCLIC_DATA_STREAM		0x8032	/*!< Android device requests to stop the Buffered Data Stream */

/* Parameters */
//#define SERVICE_REQUEST_START_ACYCLIC_DATA_STREAM	 	0x4080	/*!< Set parameters (Mainboard initiative) */
//#define SERVICE_RESPONSE_START_ACYCLIC_DATA__STREAM		0x8040	/*!< Set parameters (Mainboard initiative) */

//#define SERVICE_REQUEST_PACKET_ACYCLIC_DATA_STREAM	 	0x4081	/*!< Forced Set parameters (Android Device initiative) */
//#define SERVICE_RESPONSE_PACKET_ACYCLIC_DATA_STREAM		0x8140	/*!< Forced Set parameters (Android Device initiative) */

//#define SERVICE_REQUEST_STOP_ACYCLIC_DATA_STREAM	 	0x4180	/*!< Get parameters (Android Device initiative)  */
//#define SERVICE_RESPONSE_STOP_ACYCLIC_DATA_STREAM		0x8041	/*!< Get parameters (Android Device initiative) */

#define SERVICE_SOFTWARE_WATCHDOG 0x0080
/** @} */ // end of Services table

#define SERVICE_DSAP_TABLET_REQUEST_ADDRESS 0x01
#define SERVICE_DSAP_TABLET_ACK_ 0x01

typedef enum {
    CPL_LINK_IDLE = 0,              //!< CPL_LINK_IDLE
    CPL_LINK_CONNECTION_ESTABLISHED,//!< CPL_LINK_CONNECTION_ESTABLISHED
    CPL_LINK_ENGINEER_MODE,         //!< CPL_LINK_ENGINEER_MODE
    CPL_LINK_USER_MODE,             //!< CPL_LINK_USER_MODE
    CPL_LINK_CONFIG_CHANGE,         //!< CPL_LINK_CONFIG_CHANGE
    CPL_LINK_DATA_STREAM,           //!< CPL_LINK_DATA_STREAM
    CPL_LINK_ERROR                  //!< CPL_LINK_ERROR
} CPL_CON_STATE;

#define FLOW_CONTROL_NACK_VALUE 0xFE
#define FLOW_CONTROL_ACK_VALUE  0x80

#endif /* CLOUD_PROT_DEFINES_H_ */
