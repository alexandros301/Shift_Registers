// Sketch using the 74HC165 parallel in - serial out and the 74HC595 serial in - parallel out shift registers
// Code based on Nick Gammon's tutorials and modified by Alexandros Drymonitis
// Used to send data to Pure Data
// Specs for 74HC165 http://www.ti.com/lit/ds/symlink/cd74hc165.pdf
// Specs for 74HC595 http://www.ti.com/lit/ds/symlink/cd74hc595.pdf
// http://drymonitis.me/

// Pin connections for Uno and similar:

// Button chip pin 1  (/PL) goes to LATCH (D9)
// Button chip pin 2  (CP)  goes to SCK   (D13)
// Button chip pin 9  (Q7)  goes to MISO  (D12)
// Button chip pin 15 (/CE) goes to ground
// To daisy chain more chips, connect pin 9 (Q7) of the second  button chip to pin 10 (DS) of the first button chip
// The /PL and CP pins of the second chip connect straight to the same ones on the first chip
// You can repeat this process for as many chips as you like
// Connect each pin from D0 to D7 to ground with a 10k resistor and to one leg of the push button (or toggle switch)
// The other leg of each push button goes to 5V

// LED chip pin 11 (SRCLK) goes to SCK  (D13) 
// LED chip pin 12 (RCLK)  goes to SS   (D10)
// LED chip pin 14 (SER)   goes to MOSI (D11)
// LED chip pin 10 (/MR)   goes to +5v
// LED chip pin 13 (/OE)   goes to ground
// To daisy chain more chips, connect pin 9 (Q7') of the first LED chip to pin 14 (DS) of the second LED chip
// The RCLK and SRCLK pins of the second chip connect straight to the same ones on the first chip
// You can repeat this process for as many chips as you like
// To avoid clocking in bits while the main processor is booting, connect pin 12 of all chips to ground with a 10k resistor
// Connect each pin from QA to QH to the anode of the LED with a 220Ohm resistor
// The cathode of the LEDs goes to ground



// First include the SPI library
// More on SPI http://arduino.cc/en/Reference/SPI

#include <SPI.h>

const byte buttonLatch = 9;
const byte numOfButChips = 2; // This is the only modification needed if number of button chips used is changed
const int numOfButPins = numOfButChips * 8;

byte buttonData[numOfButChips] = { 0 };
byte oldButtonData[numOfButChips];

byte butArray[numOfButPins + 1];

const byte ledLatch = 10;
const byte numOfLedChips = 2; // This is the only modification needed if number of LED chips used is changed

byte ledData[numOfLedChips] = { 0 };

void refreshLEDs() // function to set the LEDs
{
  digitalWrite(ledLatch, LOW);
  for(int i = numOfLedChips - 1; i >=0; i--)
    SPI.transfer(ledData[i]);
  digitalWrite(ledLatch, HIGH);
}

void setup()
{
  SPI.begin();
  Serial.begin(115200);
  pinMode(buttonLatch, OUTPUT);
  digitalWrite(buttonLatch, HIGH);
  refreshLEDs();
}

void loop ()
{
  digitalWrite (buttonLatch, LOW);    // pulse the parallel load latch
  digitalWrite (buttonLatch, HIGH);
  for(int i = 0; i < numOfButChips; i++)
    buttonData[i] = SPI.transfer(0);
  
  butArray[0] = 0xc0; // denote the beginning of data (192, DEC)
  for(int i = 0; i < numOfButChips; i++){ // variable to count through the chips
    int chip = i * 8; // variable to set the correct index of the Serial.write array
    for(int j = 0; j < 8; j++){ // variable to count through the bits of each chip
      if(bitRead(buttonData[i], j) != bitRead(oldButtonData[i], j)){
        butArray[j + 1 + chip] = bitRead(buttonData[i], j);
        Serial.write(butArray, numOfButPins + 1);
        if(bitRead(buttonData[i], j)){ // if a push button is held down; if using toggle switches, don't use this if test, use only the code inside the test's curly braces
          // enclose the following two lines of code in a while(i <= numOfLedChips) loop if numOfButChips > numOfLedChips
          bitWrite(ledData[i], j, !bitRead(ledData[i], j)); //change the state of the respective LED
          // the line of code above is used with push buttons; when a button is pressed, the LED will change its state
          // if using toggle switches use the line of code below instead
          // bitWrite(ledData[i], j, bitRead(buttonData[i], j));
          refreshLEDs(); // call the function to set the LEDs
        }
      }
    }
    oldButtonData[i] = buttonData[i];
  }
  
  delay (10);   // debounce
}  // end of loop
