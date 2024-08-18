#include <Arduino.h>
#include <SoftwareSerial.h>


// Pin definitions
const int CLK_PIN = 3;   // Interrupt pin
const int DATA_PIN = 2;  // Data pin
const int TX_PIN = 13;
const int RX_PIN = 12;
const int NUM_PACKETS = 7;
const int PACKET_LENGTH = NUM_PACKETS * sizeof(byte) * 8;
const int FRAME_DELAY = 12; // ms delay between packets

const int COUNTER_REMAIN=0x00;
const int COUNTER_DEW=0x01;
const int COUNTER_ELAPSED=0x02;
//const int COUNTER_ELAPSED=0x03;

SoftwareSerial softSerial (RX_PIN, TX_PIN, 1);
byte ssData = 0;
struct counter {
  byte sign_minus;
  int hours;
  int minutes;
  int seconds;
} t;
bool tapeReadOnly;

byte byteToSend = 0xFF;

//byte old_byte = 0;

// Buffer to store sampled data
byte data[NUM_PACKETS]; //buffer in byte form
byte data_old[NUM_PACKETS];
const int BUFFER_SIZE = 128; // Size of the buffer
volatile bool buffer[BUFFER_SIZE]; //buffer to store in coming bits
volatile int bufferIndex = 0;
unsigned long clkStartTime = 0; //How long since the last clock

//Frame 3  bits0-3 deck status
byte deckMode;
enum DECK_MODE {
  DCK_STOP=0,
  DCK_EJECT ,
  DCK_REW ,
  DCK_FF ,
  DCK_REV ,
  DCK_CUE ,
  DCK_SLOW ,
  DCK_OFF ,
  DCK_PLAY ,
  DCK_PAUSE ,
  DCK_REC ,
  DCK_REC_PAUSE ,
  DCK_AUDIO_DUB,
  DCK_A_D_PAU ,
  DCK_INSERT ,
  DCK_INS_PAUSE ,
};


/*
           VTR Commands
           ==========================
           Send on Byte 3             EC100 EC200 EC300

             $00     Stop                x    x    x
             $02     Rewind              x    x    x
             $03     Fast Forward        x    x    x
             $04     Review              x    x    x
             $05     Cue                 x    x    x
             $06     Still/pause         x    x    x
             $08     Record              x    x    x
             $09     A-Dub               x
             $0A     Play                x    x    x
             $0B     Reverse Play             x    x
             $0C     Frame Advance       x    x    x
             $0D     Reverse Frame Adv.       x    x
             $0F     Slow                x
             $2E     Slow Up             x
             $2F     Slow Down           x
             $33     Counter Reset       x    x    x
             $94     Direct Search       x
             $9D     Assemble Insert     x    x    x
             $9E     Assemble Record     x    x    x
             $9F     Assemble Play       x    x    x
             $A8     Preroll Wait                  x
             $A9     Assemble A-Dub           x    x
             $AA     Assemble A/V Insert      x    x
             $AB     Rehearsal A-Dub          x    x
             $AB     Rehearsal A/V Insert     x    x       ?? $AC ??
             $AB     Rehearsal Insert         x    x       ?? $AD ??
             $AB     Rehearsal Record         x    x       ?? $AE ??
             $AF     Frame Requeset           x    x
             $EF     NOP (Connect)       x
             $FE     NOP                      x    x
           ==========================
*/
/*
Arduino Control-M decoder

Frame 0   
bits    
2 & 3  0 0 TAPE REMAIN
       0 1 DEW
       1 0 INDEX
       1 1 Counter
Bits 5,4 and 1,0 must be the same for counter mode change

Frame 1 // Keypad and IR remote
Hex   cmd
0x00  stop
0x01  eject
0x02  rewind (Review)
0x03  fastfwd (Cue)
0x06  pause
0x08  record
0x09  Audio dubbing
0x0A  play
0x3D  power
0x53  counter mem on/off
0x54  counter reset
0xb0  digital tracking on
0xb1  tracking up
0xb2  tracking down
0xb3  insert
0x88  audio select
0xa0  edit in
0xa1  edit out
0xa2  edit set
0xa3  edit start
0x80  jog on 1
0xdf  DNR

frame 2
bits 0 - 3 Timer commands
0x0 NOOP
0x1 STANDBY
0x2 VPS SELECT
0x3 REC PAUSE
0x4 TIMER REC BY VPS
0x5 RECORD
0x6 TIMER REC BY TIME
0x7 OTR REC
0x8 INDEX REC
9 - E NOOP

bits 4-5 Tape speed
0 2h
1 4h
2 6h
3 N/A

bit 6? Tuner or Line
Bit 7 TV VTR

frame 3
Tape commands?
*/