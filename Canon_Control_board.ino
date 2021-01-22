/* Canon Control Board or Canon EOS Time-lapse maker
 * I made it because the Canon EOS 2000D does not have a "Time-Lapse" function.
 * I made the first version with Micro:bit and Pimoroni (automation:bit) card.
 * 
 * v2.21 - After 5 sec exit to Main Menu if you don't do nothing
 * v2.2  - Add refresh displays with Functions
 * v2.1  - Add (sht * nop) time to alltime and timeleft calculation
 * v2.0  - Test with 0.96" Oled Display
 * v1.0  - Test With Serial Communication
 */

// Program Current Version
const float version = 2.21;

// ------------------ Pins and Variables ------------------
// Rotary encoder HW-040 pin numbers
const int pinA = 2;  // Connected to CLK on KY-040
const int pinB = 3;  // Connected to DT on KY-040
const int pinSW = 4; // Connected to SW on KY-040
int pinALast;
boolean pinSWLast;
int aVal;
int swVal;

// Star - Stop Button
const int staShoot = 8;          // Start Time-lapse
const int stoShoot = 9;          // Stop or Single Shoot
unsigned int staShootState;      // Variable to enter 'Time-lapse' sequence

// Output to 
const int Shoot = 10; // Shoot port
const int sht = 250;  // Shoot time in ms   <- CHANGE THIS VARIABLE WHEN CAMERA DOESN'T TAKE PICTURE, BECAUSE THE TIME IS TO SHORT
const int bulb = 40;  // 40 * 1000ms        <- This variable is for long exposure
const int bulbmin = 40;   // 40 * 1000ms    <- Max exposure
const int bulbmax = 300;  // 300 * 1000ms   <- Max exposure

// Menu State 0 = Main Menu ; 1 = Set Delay Time ; 2 = Number of pictures
unsigned int menu = 0;
unsigned int menuState = 0;

// Delay time
unsigned int dt = 10;     // Initial DELAY TIME.          Minimum 1 sec - maximum 60 sec
unsigned int dtmin = 1;   // DELAY TIME Min
unsigned int dtmax = 60;  // DELAY TIME Mxn

// Number of pictures
unsigned int nop = 100;     // Initial NUMBER OF PICTURES.  Minimum 10 frame - maximum 300
unsigned int nopmin = 10;  // NUMBER OF PICTURES Min
unsigned int nopmax = 300;  // NUMBER OF PICTURES Max

// All Time
float alltime ;
float leftime ;
unsigned long startM ;
unsigned long endM ;
unsigned long Interval = 5000;    // After X sec exit to Main Menu if you don't do nothing

// Void
void DisplayMainMenu();
void DispalyDelayTime();
void DisplayNumberOfPictures();
// void exitMenu();                  // If don't set anything, exit the menu after 10 sec

// ------------------ I2C Oled ------------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ------------------ Setup ------------------
void setup() {
  // Serial Begin
  Serial.begin(115200);
  
  // Oled
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D or 0x3C for 128x64 
    Serial.println(F("-- SSD1306 allocation failed --"));
    for(;;);
  }  
  
  // Rotary encoder HW-040
  pinMode (pinA, INPUT);
  pinMode (pinB, INPUT);
  pinMode (pinSW, INPUT_PULLUP); // PUSH BUTTON
  pinSWLast = true;
  pinALast = digitalRead(pinA);
  
  // Star - Stop Button
  pinMode (staShoot, INPUT_PULLUP);
  pinMode (stoShoot, INPUT_PULLUP);

  // Output 
  pinMode(Shoot, OUTPUT);

  //Serial.println("-- Canon Control board --");
  //Serial.println("--   " + String(version));
  //Serial.println("*************************");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(32, 0);
  display.println("Canon");
  display.setCursor(22, 19);
  display.println("Control");
  display.setCursor(32, 38);
  display.println("Board");
  display.setTextSize(1);
  display.setCursor(52, 56);
  display.println(version);
  display.display();
  delay(2000);

  DisplayMainMenu();
}

// ------------------ Loop -------------------
void loop() {
  // --------------- Main Menu ---------------
  while (menuState == 0){
    /*
    Serial.println("**************************");
    Serial.println("Main Menu: ");
    Serial.println("");
    Serial.print("Delay Time: ");
    Serial.println(dt);
    Serial.print("Number of Pictures: ");
    Serial.println(nop);
    Serial.println("");
    Serial.println("**************************");
    */

    menuState = senseSW();

    while (digitalRead(staShoot) == HIGH && digitalRead(stoShoot) == HIGH && digitalRead(pinSW) == HIGH){

    }

    if (!digitalRead(staShoot)){
      staShootState = true;
      delay(1);
    }
    // ----- Create Single shoot if Stop button pressed ----- 
    if (!digitalRead(stoShoot)){
      digitalWrite(Shoot, HIGH);
      delay(sht);               // Delay 250ms
      digitalWrite(Shoot, LOW);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(25, 0);
      display.println("Single");
      display.setCursor(29, 22);
      display.println("Shoot");
      display.setCursor(22, 44);
      display.println("Created");
      display.display();
      delay(2000);
      DisplayMainMenu();
    }
    
    // ----- Start Time-Lapse Sequence ----- 
    while (staShootState == true ){
      // Serial.println("Start Time-Lapse sequence");
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(30, 0);
      display.println("Start");
      display.setCursor(30, 22);
      display.println("Time");
      display.setCursor(30, 44);
      display.println("Lapse");
      display.display();
      delay(1000);
      for (int y = 3; y != 0; y--){
        delay(1000);
        //Serial.print("Start in ");
        //Serial.print(y);
        //Serial.println(" second");
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(10, 10);
        display.println("Start in:");
        display.setCursor(50, 40);
        display.println(y);
        display.display();
      }
      //Serial.println("**************************");
      delay(1000);
      for (int i = 1; i != nop+1; i++){
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(15, 10);
        display.println("Pictures:");
        display.setCursor(50, 34);
        display.println(i);

        // Left Time
        //                     (2*(60 - 59))/60                   +          (250/1000)            *       (60-59)              /  60
        leftime = ( ( (float)dt*((float)nop - (float)i)) / (float)60) + ( ( (float)sht / (float)1000 ) * ((float)nop - (float)i) ) / (float)60;
        display.setTextSize(1);
        display.setCursor(0, 56);
        display.print("Time left:  ");
        display.setCursor(70, 56);
        display.print(leftime, 2);
        display.setCursor(110, 56);
        display.println("min");
        display.display();
        
        // Shoot
        digitalWrite(Shoot, HIGH);
        delay(sht);               // Delay 250ms
        digitalWrite(Shoot, LOW);
        delay(dt*1000);           // Delay dt * 1000ms = secundum
     
        if (!digitalRead(stoShoot)){
          staShootState = false;
          break;
        }
        //Serial.print("Picture: ");
        //Serial.println(i+1);
      }
      //Serial.println("End of sequence.");
      delay(1000);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(30, 15);
      display.println("End Of");
      display.setCursor(15, 35);
      display.println("Sequence");
      display.display();
      delay(3000);
      staShootState = false;
      DisplayMainMenu();
    }
  }
  
  // --------------- Menu ---------------
  switch (menuState) {
    case 1:
      /*
      Serial.println("-- Edit Delay Time: 1-60 sec");
      Serial.print("-- Current Delay Time: ");
      Serial.println(dt);
      */
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Edit Delay Time: 1-60");
      display.setCursor(5, 18);
      display.println("Current Delay Time");
      display.setCursor(50, 40);
      display.setTextSize(2);
      display.println(dt);
      display.display();
      startM = millis();
      
      while (menuState == 1 ) {
          aVal = digitalRead(pinA);
      if (aVal != pinALast) { // Means the knob is rotating
      // if the knob is rotating, we need to determine direction
      startM = endM;      
        if (digitalRead(pinB) != aVal) {       // Means pin A Changed first
          // dt = dt + 1;
          ++dt;
          dt = min(dt, dtmax),
          // Serial.print("Delay time: ");
          // Serial.println(dt);
          DispalyDelayTime();
  
        } else {                               // Otherwise B changed first
          // dt = dt - 1;
          --dt;
          dt = max(dtmin, dt);
          // Serial.print("Delay time: ");
          // Serial.println(dt);
          DispalyDelayTime();
        }
      }
        pinALast = aVal;
        menuState = senseSW();
        endM = millis();
        if ( (endM - startM) >= Interval ){
          menuState = 3;
          }
      }
      break;
    case 2:
      //Serial.println("-- Edit Number of Pictures: 10-300 frame");
      //Serial.print("-- Current Number of Pictures: ");
      //Serial.println(nop);
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("EditNumOfPics: 10-300");
      display.setCursor(5, 18);
      display.println("Current Num of Pics");
      display.setCursor(50, 40);
      display.setTextSize(2);
      display.println(nop);
      display.display();
      startM = millis();
      
      while (menuState == 2) {
        aVal = digitalRead(pinA);
      if (aVal != pinALast) { // Means the knob is rotating
      // if the knob is rotating, we need to determine direction
      startM = endM;
        if (digitalRead(pinB) != aVal) {       // Means pin A Changed first
          //++nop;
          nop += 5;
          nop = min(nop, nopmax);
          //Serial.print("Number of Pictures: ");
          //Serial.println(nop);
          DisplayNumberOfPictures();
  
        } else {                               // Otherwise B changed first
          //--nop;
          nop -= 5;
          nop = max(nopmin, nop);
          //Serial.print("Number of Pictures: ");
          //Serial.println(nop);
         DisplayNumberOfPictures();
        }
      }
        pinALast = aVal;
        menuState = senseSW();
        endM = millis();
        if ( (endM - startM) >= Interval ){
          menuState = 3;
          }        
      }
      break;
  }
  if (menuState > 2) {
    menuState = 0;
    menu = 0;
    DisplayMainMenu();
  }

}

// If press button increase menu with one
int senseSW() {
  swVal = digitalRead(pinSW);
  if (pinSWLast == true ) {
    if (swVal == LOW) {
      ++menu;
      pinSWLast = false;
    }
  }
  if (swVal == HIGH) {
    pinSWLast = true;
  }
  return menu;
}

// Display Main menu
void DisplayMainMenu(){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(12, 0);
    display.println("Main Menu");

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print("Delay Time:     ");
    display.println(dt);
    
    display.setCursor(0, 38);
    display.print("Number of Pics: ");
    display.println(nop);
    display.display();
    
    // Time-Lapse Required Time Calculation 
    alltime = ( (float)dt*nop / (float)60) + ( ( (float)sht / (float)1000 ) * (float)nop ) / (float)60;
    
    display.setCursor(0, 52);
    display.print("Time req: ");
    display.setCursor(60, 52);
    display.print(alltime, 2);
    display.setCursor(110, 52);
    display.println("min");
    display.display();
}

// Display Delay Time
void DispalyDelayTime(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(30, 18);
  display.println("Delay Time: ");
  display.setCursor(50, 40);
  display.setTextSize(2);          
  display.println(dt);
  display.display();
}

// Display Number Of Pictures
void DisplayNumberOfPictures(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(25, 18);
  display.println("Number of Pics: ");
  display.setCursor(50, 40);
  display.setTextSize(2);
  display.println(nop);
  display.display();
}

// Display BulbMode
void DisplayBulbMode(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(25, 18);
  display.println("Bulb exp. Time: ");
  display.setCursor(50, 40);
  display.setTextSize(2);
  display.println(bulb);
  display.display();
}
