/*
 * packet.h
 *
 *  Created on: 18.03.2013
 *      Author: fritz
 */







// TODO: add Packet-Type-Identifiers





#ifndef PACKET_H_
#define PACKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define COMMAND_INVALID_RECEIVER  0xFFFF
#define COMMAND_INVALID_CMD 0xFF
// Commands, that can be sent to the devices of the RF-Network
enum Radio_Command {RCmd_RESET=0xab, RCmd_INVALID = COMMAND_INVALID_CMD};

enum Packet_Type{PACKET_TYPE_COMMAND = 1, PACKET_TYPE_MATRIX, PACKET_TYPE_ALIVE};

typedef uint32_t TIME_T; // 32 Bit value
	// this lasts with 100us time steps for 119.3 hours or 4.95 days

/*
 * Packet formats:
 *
 * All Packets have this header:
 * Name					Bytes	Type	Example		Description
 * Length				1		uint8	7			It holds the length of the packet without the lenght-byte itself
 * 													including the source and the destination address
 * 													excluding CRC16 and the in the receiver appended RSSI and LQI bytes.
 * Source-Address		1		uint8	3			Address of the transmitter. This belongs to a special time-slot.
 * Destination-Address	1		uint8	0			Address of the receiver. 0xFF = Broadcast, 0xFE = Invalid Address
 *
 *
 * Sync Packet:
 * Name					Bytes	Type	Example		Description
 * Timestamp			4		TIME_T	25000=2.5s	Base-Time in 100us-steps. Its the time of triggering the transmission.
 */

/*
 * Boat-Box to Time-Base Packet:
 * Name					Bytes	Type	Example		Description
 * Battery-Status TS	2		uint16	3700=3.7V	Voltage in mV, 0xFFFF = invalid
 * TimeStamp Trig TS	4		uint32	25000=2.5s	Time stamp of trigger from the light sensor on the Trigger-Station
 * TriggerEdge			1		uint8	1			0 = falling, 1 = rising edge. 0xFF = invalid
 * Boat-Box-ID			1		uint8	2			Is the RF-Address of the Boat-Box (2.4GHz)
 * 													This is needed for data forwarded by the Trigger-Station from the Boat-Box
 * Battery-Status BB	2		uint16	3700=3.7V	Voltage in mV, 0xFFFF = invalid
 * TimeStamp Trig BB	4		uint32	25000=2.5s	Time stamp of an trigger-event from a Boat-Box.
 * 													This is detected with the Photodetector
 */

// split the magic number into 2 bytes, so endianness doesn't matter:
#define MAGIC_NUMBER1	0x42
#define MAGIC_NUMBER2	0x24

#define PACKET_VERSION	2

#define TIME_INVALID				0
#define BOX_PACKET_ID_INVALID		0xFF		// is used for autopackets

#define ALGE_TEXT_LEN	48

// measures of KIBOKO-MATRIX
#define MATRIX_MODULES		12	// total number of modules
#define MATRIX_COLS_PER_MOD 7	// number of pixels in one row of module
#define MATRIX_ROWS			28  // number of rows per module
#define MATRIX_DATA_SIZE	(MATRIX_MODULES*MATRIX_ROWS)
#define MATRIX_BLOCK_SIZE	(MATRIX_ROWS)
#define MATRIX_N_DATABLOCKS (MATRIX_DATA_SIZE/MATRIX_BLOCK_SIZE)

/*
 * WARNING:
 * The order of these structs is important for the data alignment!
 */
// CAUTION: always use blocks of 8 bytes in this structures
// to make settings.flashCRC is on address (&settings + sizeof(SETTINGS)-1)
// and to be compatible with 64-bit processors on the PC

typedef struct
{
	TIME_T triggerTimeBB;	// send the last trigger time
	TIME_T triggerTimeTS_L; // recent trigger time of left light sensor (or at TS1)
	TIME_T triggerTimeTS_R; // recent trigger time of right light sensor (just in the case of TS2)

	uint16_t batteryVoltageTS;	// battery voltage of Trigger-Station in mV
	uint16_t batteryVoltageBB;	// battery voltage of Boat-Box in mV

	uint8_t boatBoxID; // 1...N_BOATBOXES
	uint8_t binRssiBB; // binary rssi-value of a packet coming from a Boat-Box
		// is set at the Trigger-Station
	
	uint8_t stationTriggeredAt; // where the power-led has been detected (Start=1/Goal_left=2/Goal_right=3)
	
	
	uint8_t dummy1;		// for alignment. this field is never set and has undefined value

	uint16_t packetType; // refer to enum Packet_Type
		// not used until now (23.8.2013) // TODO
	uint16_t dummy2;
}ALIVE_PACKET;



// Time-Base to Personal-Computer Packet
typedef struct
{
	uint8_t magicNumber1;		// a unique number that identifies this packet
	uint8_t magicNumber2;
	uint16_t packetVersion;		// the packet version

	uint16_t batteryVoltageTB;	// battery voltage of Time-Base in mV
	uint8_t triggerStationID;	// the ID of the Trigger-Station, the tsPacket was received from. (START: 1, GOAL: 2)
	uint8_t binRssiTS;			// binary rssi-value of a packet coming from a Trigger-Station (is set at the Time-Base)

	ALIVE_PACKET tsPacket;		// the packet from TS

	uint32_t baseTime_100us;	// Time of the Time-Base in 100us
	uint16_t dummyShort;		// is needed, that the size of the structure is a multiple of 8 bytes.
	uint8_t dummyByte;

	uint8_t crc;				// for crc error check
}TB2PC_PACKET;


// Personal-Computer to PacketTime-Base
typedef struct
{
    uint8_t magicNumber1;		// a unique number that identifies this packet
    uint8_t magicNumber2;
    uint16_t packetVersion;		// the packet version

    uint32_t pcTime_100us;      // Time of the Time-Base in 100us
	
	char algeText[ALGE_TEXT_LEN];	// Text for ALGE display
	
	uint8_t matrixData1[MATRIX_DATA_SIZE]; // data for MATRIX display 1
	uint8_t matrixData2[MATRIX_DATA_SIZE]; // data for MATRIX display 2
	uint8_t matrixDataValid; // if 0, not valid, else valid
	uint8_t dummy1;
	uint16_t dummy2;

	// Command Bytes (Commands can be sent from the Kiboko-Manager to the RF-Devices)
    uint16_t cmdReceiver; // MSB=0...433MHz, MSB=1...2.4GHz devices
    uint8_t command; // Command Byte (Refer to enum Radio_Command)

    uint8_t crc;				// for crc error check
}PC2TB_PACKET;


// packet for MATRIX display from Time-Base to the Matrix-Boards
typedef struct
{
	uint16_t packetType; // refer to enum Packet_Type
	uint8_t imageID; // it is counted from 0 to 255 (then runs over),
		// and is needed by the Matrix-Board, to reassamble the
		// sub-images into the whole image.
	uint8_t blocknumber; 					//(0 ... MATRIX_N_DATABLOCKS-1)
	uint8_t datablock[MATRIX_BLOCK_SIZE];
}MATRIX_PACKET;


// packet to transmit commands to the devices in the RF-network
typedef struct
{
	uint16_t packetType; // refer to enum Packet_Type
    uint16_t cmdReceiver; // MSByte=0...433MHz, MSByte=1...2.4GHz devices, LSByte=network address

    uint8_t command; // Command Byte (Refer to enum Radio_Command)
    uint8_t dummy1;
    uint16_t dummy2;
}COMMAND_PACKET;



#ifdef __cplusplus
}
#endif

#endif /* PACKET_H_ */


