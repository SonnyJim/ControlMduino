
#include "controlm.h"


void printDeckMode();
void clockOutByte(byte data);
int byteSendCount;

// Interrupt Service Routine (ISR) for the falling edge on CLK_PIN
// Reset the buffer and timestamp if CLK is held high for FRAME_DELAY
void sampleData() {
  if (clkStartTime + FRAME_DELAY < millis())
  {
    clkStartTime = millis();
    bufferIndex = 0;
  }
  // Sample data from DATA_PIN
  if (bufferIndex < BUFFER_SIZE) {
    buffer[bufferIndex] = digitalRead(DATA_PIN);
    bufferIndex++;
  }
  
  if (bufferIndex == 16 && byteToSend != 0xFF) //Send a command if we have one
  {
    //Serial.print ("Sending command: ");
    //Serial.println (byteToSend, HEX);
    clockOutByte(byteToSend);
    //clockOutByte (0x0A);
    byteSendCount++;
    if (byteSendCount > 2)
      byteToSend = 0xFF;
    bufferIndex += 8;
  }
  
}

void setup() {
  // Initialize pins
  pinMode(CLK_PIN, INPUT);
  pinMode(DATA_PIN, INPUT);

  // Attach interrupt to CLK_PIN
  attachInterrupt(digitalPinToInterrupt(CLK_PIN), sampleData, RISING);
  // Start serial communication for debugging
  Serial.begin(115200);
  delay(200);
  Serial.println("*************************************************************");
  Serial.println("******************BOOOOOOTIIIIIIING**************************");
  Serial.println("*************************************************************");
}

void printByteAsBinary(byte value) {
  for (int i = 7; i >= 0; i--) {
    Serial.print(bitRead(value, i));
  }
  Serial.println();
}

byte bcdToInt(byte val)
{
  return( (val/16*10) + (val%16) );
}

void printData()
{
  Serial.print("{");
  for (int i = 0; i < NUM_PACKETS; i++)
  {
    Serial.print (data[i], HEX);
    if (i < (NUM_PACKETS - 1))
      Serial.print (":");
  }
  Serial.print ("} ");
}
// Function to clock out a byte on DATA_PIN with 12 �s pulses on CLK_PIN
void clockOutByte(byte data) {
  noInterrupts();
    digitalWrite(CLK_PIN, HIGH);
  digitalWrite(DATA_PIN, HIGH);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  delayMicroseconds(500);
  for (int i = 7; i >= 0; i--) {  // Iterate over each bit from MSB to LSB
    // Set DATA_PIN to the current bit of the data byte
    digitalWrite(DATA_PIN, (data >> i) & 0x01);

    // Generate a 12 �s pulse on CLK_PIN
    digitalWrite(CLK_PIN, LOW);
    delayMicroseconds(6);  // High for 12 �s
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(6);  // Low for 12 �s
  }

    // Initialize pins
  pinMode(CLK_PIN, INPUT);
  pinMode(DATA_PIN, INPUT);
  interrupts();
}

void process_serial(){
  if (Serial.available() > 0) {
    // Read the incoming byte
    byteToSend = Serial.read();
    Serial.print ("byteToSend: ");
    Serial.println (byteToSend, HEX);
    byteSendCount = 0;
  }
}

void process_buffer(){
  for (int i = 0; i < NUM_PACKETS; i++) {
        byte currentByte = 0;
        // Construct the byte from 8 bits in the buffer
        for (int bit = 0; bit < 8; bit++) {
          currentByte |= (buffer[i * 8 + bit] << (7 - bit));
        }
        data[i] = currentByte; // Store the constructed byte into data array
  }

  if (memcmp(data, data_old, NUM_PACKETS) != 0)
  {
    //printData();
    memcpy (data_old, data, NUM_PACKETS);
  }
  else
    return;

  if ((data[0] & 0b00000011) != 0b00000011)
    return; //TODO Only handling Counter frames at this point in time
  t.sign_minus = (data[4] >> 4) & 1; //1 when minus counter sign is active
  t.hours = bcdToInt(data[4] & 0x0F); //First 4 bits of frame 4
  t.minutes = bcdToInt(data[5]);
  t.seconds = bcdToInt(data[6]);
  deckMode = data[3] & 0x0F; //Take the first 4 bits of Frame 3 for the deck mode
  tapeReadOnly = (data[3] >> 7) & 1;
  
  if (t.sign_minus)
    Serial.print ("-");
  Serial.print (t.hours);
  Serial.print (":");
  Serial.print (t.minutes);
  Serial.print (":");
  Serial.print (t.seconds);
  if (tapeReadOnly)
    Serial.print (" RO ");
  else
    Serial.print (" RW ");
  printDeckMode();
  //printByteAsBinary(data[0]);
      //Serial.println();
      
      /*
      if (old_byte != data[6])
      {

        Serial.println (data[6], HEX);
        printByteAsBinary(data[6]);
        old_byte = data[6];
        
      }*/
      /*
      if (data[0] & 0b00110000)
        Serial.println ("Display: Elapsed");
      else
        Serial.println ("Display: Remaining");
  
      if (data[2] & 0x01)
      Serial.println ("Edit deck not connected");
    else
      Serial.println ("Edit deck connected");
    printDeckMode(); //data[3] is deck mode 
*/
}
void loop() {
  if (bufferIndex >= PACKET_LENGTH) {
    process_buffer();
  }
  process_serial();
}


void printDeckMode()
{
  // Compare with DECK_MODE values
  switch (deckMode) {
    case DCK_STOP:
      Serial.println("STOP");
      break;
    case DCK_PAUSE:
      Serial.println("PAUSE");
      break;
    case DCK_REW:
      Serial.println("REW");
      break;
    case DCK_FF:
      Serial.println("FF");
      break;
    case DCK_REV:
      Serial.println("REV");
      break;
    case DCK_CUE:
      Serial.println("CUE");
      break;
    case DCK_SLOW:
      Serial.println("SLOW");
      break;
    case DCK_OFF:
      Serial.println("OFF");
      break;
    case DCK_PLAY:
      Serial.println("PLAY");
      break;
    case DCK_EJECT:
      Serial.println("EJECT");
      break;
    case DCK_REC:
      Serial.println("REC");
      break;
    case DCK_REC_PAUSE:
      Serial.println("REC PAUSE");
      break;
    case DCK_AUDIO_DUB:
      Serial.println("AUDIO DUB");
      break;
    case DCK_A_D_PAU:
      Serial.println("A_D_PAU");
      break;
    case DCK_INSERT:
      Serial.println("INSERT");
      break;
    case DCK_INS_PAUSE:
      Serial.println("INS PAUSE");
      break;
    default:
      Serial.println("Unknown Deck Mode");
      break;
  }

}