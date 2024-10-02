/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 19/03/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

#define NSS 10                                  //select on LoRa device
#define NRESET 9                                //reset on LoRa device
#define RFBUSY 7                                //RFBUSY pin on LoRa device
#define DIO1 3                                  //DIO1 on LoRa device, used for RX and TX done 
#define LED1 8                                  //On board LED, high for on
#define SDCS 30                                 //select pin for SD card 
#define OV2640CS 22                             //select for SPI on camera

#define LORA_DEVICE DEVICE_SX1280               //this is the device we are using

#define Monitorport Serial                      //Port where serial prints go

const uint32_t Frequency = 2445000000;          //frequency of transmissions
const uint32_t Offset = 0;                      //offset frequency for calibration purposes
const int8_t  TXpower = 10;                     //LoRa transmit power

//*******  Setup LoRa modem parameters here ! ***************
const uint8_t Bandwidth = LORA_BW_1600;         //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF5;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate

//*******  Setup FLRC modem parameters here ! ***************
const uint8_t BandwidthBitRate = FLRC_BR_1_300_BW_1_2;     //FLRC bandwidth and bit rate, 1.3Mbs
//const uint8_t BandwidthBitRate = FLRC_BR_0_260_BW_0_3;   //FLRC 260kbps
const uint8_t CodingRate = FLRC_CR_1_0;                    //FLRC coding rate
const uint8_t BT = RADIO_MOD_SHAPING_BT_1_0;               //FLRC BT
const uint32_t Syncword = 0x01234567;                      //FLRC uses syncword


const uint32_t TXtimeoutmS = 5000;              //mS to wait for TX to complete
const uint32_t RXtimeoutmS = 60000;             //mS to wait for receiving a packet

const uint32_t ACKdelaymS = 0;                  //ms delay after packet actioned and ack sent
const uint32_t ACKsegtimeoutmS = 75;            //mS to wait for receiving an ACK before re-trying transmit segment
const uint32_t ACKopentimeoutmS = 750;          //mS to wait for receiving an ACK before re-trying transmit file open
const uint32_t ACKclosetimeoutmS = 750;         //mS to wait for receiving an ACK before re-trying transmit file close
const uint32_t DuplicatedelaymS = 10;           //ms delay if there has been an duplicate segment or command receipt
const uint32_t NoAckCountLimit = 250;           //if no NoAckCount exceeds this value - restart transfer
const uint8_t HeaderSizeMax = 12;               //max size of header in bytes, minimum size is 7 bytes
const uint8_t DataSizeMax = 245;                //max size of data array in bytes
const uint8_t DTSendAttempts = 10;              //number of attempts sending a packet before a restart
const uint8_t StartAttempts = 2;                //number of attempts to start transfer before a fail
const uint8_t SendAttempts = 5;                 //number of attempts carrying out a process before a restart
const uint8_t Maxfilenamesize = 32;             //size of DTfilename buffer
const uint32_t FunctionDelaymS = 0;             //delay between functions such as open file, send segments etc
const uint32_t PacketDelaymS = 1000;            //mS delay between transmitted packets such as DTInfo etc
const uint16_t NetworkID = 0x3210;              //a unique identifier to go out with packet

#ifdef USELORA
const uint8_t SegmentSize = 245;              //number of bytes in each segment, 245 is maximum value for LoRa
#endif

#ifdef USEFLRC
const uint8_t SegmentSize = 117;              //number of bytes in each segment, 117 is maximum value for FLRC
#endif


/******************************************************************************
  Setup the resolution OV2640 required, the choices are;

  OV2640_160x120,OV2640_176x144,OV2640_320x240,OV2640_352x288,OV2640_640x480,
  OV2640_800x600, OV2640_1024x768,OV2640_1280x1024,OV2640_1600x1200
*******************************************************************************/

const uint8_t OV2640resolution = OV2640_640x480;
