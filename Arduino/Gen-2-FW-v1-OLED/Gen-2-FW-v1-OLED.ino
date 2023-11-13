////////////////////////////////////////////////////////////

/*
  Lite3DP Gen 2 v1.0 - OLED auxiliary display

  https://github.com/Lite3DP/Lite3DP-Gen-2

  MIT License

  Please visit www.lite3dp.com for more information, documentation and software.

  email: lite3dp@lite3dp.com

*/

////////////////////////////////////////////////////////////

/*
  LIBRARIES USED:

  Thank you very much to the following libraries creators!

  1) TFT_eSPI by Bodmer
     https://github.com/Bodmer/TFT_eSPI

  2) SDFat by Greiman
     https://github.com/greiman/SdFat

  3) PNGdec by bitbank2
     https://github.com/bitbank2/PNGdec

  4) Adafruit_SH110X by Adafruit
     https://github.com/adafruit/Adafruit_SH110x

  5) Adafruit_GFX by Adafruit
     https://github.com/adafruit/Adafruit-GFX-Library

  6) Adafruit_BusIO by Adafruit
     https://github.com/adafruit/Adafruit_BusIO

  7) Converter tool, file to C style array converter by notisrac
     https://notisrac.github.io/FileToCArray/
     https://github.com/notisrac/FileToCArray

*/

////////////////////////////////////////////////////////////

/*
  FIRMWARE UPLOAD:

  1) Install ESP32 board in Arduino IDE. Here is a good tutorial: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

  2) Install the 6 libraries. If you already have a library with the same name installed, delete it or temporarily move it from the libraries folder.

  2) Select Board: ESP32 Dev Module

  3) Connect the main board with USB to TTL board (CP2102, CH340, etc.):

  Main board / USB to TTL board
      RX----------TX
      TX----------RX
      VIN---------5V
      GND---------GND

  4) On the Gen 2 main board, press the RST button, and without releasing it, press the BOOT button. Release RST and then release BOOT.

  5) Upload firmware and reset.

  6) Please be careful! Disconnect the 4 wires from the USB to TTL board before plugging the 12V power supply.
  
*/

////////////////////////////////////////////////////////////

// **** PARAMETERS ****

int hUp;                     // Lift height (mm)

int hUpInitial;              // Inital layers lift height (mm)

int FirstLayers;             // Number of bottom layers

float expotime;              // Exposure time (s)

int iexpotime;               // Bottom exposure time (s)

float LiftSpeed;             // Lift speed (mm/s)

float LiftSpeedInit;         // Bottom lift speed (mm/s)

float RetractSpeed;          // Retract speed (mm/s)

int RestTime;                // Delay after retract (ms)

int PWMUV;                   // UV power (0-255)

int PWMBL;                   // White backlight brightness (0-255)

int Lguide;                  // Total length of linear guide

float pitch;                 // Pitch of leadscrew

int stepsMotorPerRev;        // Motor steps per revolution (including gear reduction, if any)

int microsteps;              // Driver microsteps

////////////////////////////////////////////////////////////

// Pin definition

#define PinDir 26
#define PinStep 27
#define PinEn 25

//#define sclk 18
//#define miso 19
//#define mosi 23
//#define cs   15
//#define dc   2
//#define rst  4

#define SDcs 5

#define BtnUp 12
#define BtnDown 14
#define BtnNext 13
#define BtnBack 32

#define PinEndStop 36

#define LightPin 16
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

#define BacklightPin 33
const int freq1 = 5000;
const int ledChannel1 = 1;
const int resolution1 = 8;

#define SdEnter 34

#define BtnPlay 39

#include "OLED-templates.h"
#include "OLED-test-print.h"
#include "Menu-templates.h"
#include "Keychain-test-print.h"


///////////////////////////////////////////////

// ****LIBRARIES****

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire, -1, 400000, 100000);

#include <SPI.h>

#include <PNGdec.h>
#include <SdFat.h>
SdFat SD;

#include <Preferences.h>
Preferences preferences;

#include "SPI.h"
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

PNG png;

int16_t xpos = 0;
int16_t ypos = 27;

///////////////////////////////////////////////

// ****GLOBAL VARIABLES****

// **Screen number**

int screen = 0;


//**Button/SD status**

int edoBtnUP;
int edoBtnDOWN;
int edoBtnNEXT;
int edoBtnBACK;
int edoBtnPLAY;
int edoSdEnter;


//**Button delay**

#define delaybutton 200


//**Preview number of sections**

const int lenpreview = 60;

// **EndStop Lecture**

bool LectEndStop;


// **Layer Height**

float hLayer = 0.05;
int hLayerx1000 = hLayer * 1000;


// **Ascendant movement Height (mm)**

int maxheight;


// **Retract movement during printing**

float hDown;

float hDownInitial;


// **Steps per mm**

int StepsPerMm;


// **Calibration aditional steps**

const int maxAddDesc = 3;     // Additional maximum descent (Default = 3 mm)

long maxAddSteps;

long stepsadditional;

int stepsadditionalx80 = 0;


// **Speed and time calculations**

int topdelay = 200;         // Movement delay between lift and retract

int delayLiftSpeed;

int delayLiftSpeedInit;

int delayRetractSpeed;

float updowntime;           // Light-off delay in seconds. Counted from turning off the UV light on one layer and turning on the UV light on the next one.

float updowntimeInit;       // For initial layers.


// **For the correct name of file**

String DirAndFile;

String FileName;

int number;

String slicermode;

int slicernumber;


//**For the layers counter**

int LayersCounter = 0;

int Layers;

String dirfoldersel;


// **For the name of the folder**

File root;

File myfile;

char foldersel[25];

int counter = 1;


// **OLED**

int AdvLay = 0;
int xx0 = 32;
int yy0 = 40;
int xx1;
int yy1;
int alfaDegTot = 0;
float alfaRad;
int radius = 30;

unsigned long compTimeMillis = 0;


// **Test prints**

int PrintMode; // 0:micro SD File, 1:Keychain test, 2:OLED cover test



///////////////////////////////////////////////



void setup(void) {

  ReadPref();
  recalcTime();

  //UV Light, PWM
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(LightPin, ledChannel);
  ledcWrite(ledChannel, 0);

  //White backlight, PWM
  ledcSetup(ledChannel1, freq1, resolution1);
  ledcAttachPin(BacklightPin, ledChannel1);
  ledcWrite(ledChannel1, 0);

  pinMode (PinDir, OUTPUT);
  pinMode (PinStep, OUTPUT);
  pinMode (PinEn, OUTPUT);
  pinMode (PinEndStop, INPUT);
  pinMode (SdEnter, INPUT_PULLUP);

  pinMode (BtnPlay, INPUT);

  digitalWrite (PinEn, HIGH);

  tft.begin();

  tft.setRotation(3);

  coordRot3();

  screen = 0;

  displaymenuintro();

  OLEDintro();

  displaymenuprint();

}

////////////////////////////////////////////////////

void loop() {

  edoBtnUP = touchRead (BtnUp);
  edoBtnDOWN = touchRead (BtnDown);
  edoBtnNEXT = touchRead (BtnNext);
  edoBtnBACK = touchRead (BtnBack);
  edoBtnPLAY = digitalRead (BtnPlay);


  if (edoBtnPLAY == LOW) {
    btnPLAYmenu();
  }

  if (edoBtnUP < 20) {
    btnUPmenu();
  }

  if (edoBtnDOWN < 20) {
    btnDOWNmenu();
  }

  if (edoBtnNEXT < 20) {
    btnNEXTmenu();
    delaybtn();
  }

  if (edoBtnBACK < 20) {
    btnBACKmenu();
    delaybtn();
  }

  if (screen == 0 && edoBtnPLAY == LOW && edoBtnUP < 20) {
    Testmode();
    edoBtnUP = touchRead (BtnUp);
    edoBtnDOWN = touchRead (BtnDown);
    edoBtnNEXT = touchRead (BtnNext);
    edoBtnBACK = touchRead (BtnBack);
    edoBtnPLAY = digitalRead (BtnPlay);
    displaymenuprint();
    screen = 0;
  }

}


////////////////////////////////////////////////////////////

//*** MENU FUNCTIONS ****

void btnUPmenu() {

  switch (screen) {

    case 1:
      displaymenuprint();
      screen = 0;
      break;

    case 11:
      folderUp(root);
      delaybtn();
      break;

    case 12:
      switch (hLayerx1000) {
        case 25:
          hLayer = 0.05;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 146, hLayerx1000);
          tft.drawCentreString("um", 270, 146, 4);
          adjuststeps();
          break;

        case 50:
          hLayer = 0.1;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 146, hLayerx1000);
          tft.drawCentreString("um", 270, 146, 4);
          adjuststeps();
          break;

      }
      delaybtn();
      break;


    case 13:
      if (expotime >= 0.2 && screen == 13) {
        expotime = expotime + 0.2;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        if (expotime <= 9.8) {
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawVariableFloat(207, 146, expotime);
        }
        else if (expotime > 99.8) {
          drawVariableFloat3(200, 146, expotime);
        }
        else {
          drawVariableFloat2(200, 146, expotime);
        }

        tft.drawCentreString("s", 280, 146, 4);
      }
      delay(80);
      break;

    case 14:
      if (iexpotime >= 1 && screen == 14) {
        iexpotime++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(235, 146, iexpotime);
        tft.drawCentreString("s", 280, 146, 4);
      }
      delay(80);
      break;


    case 21:                                   //screen 21 is when the platform is down during calibration
      if (stepsadditional > 0) {
        stepsadditional--;
        digitalWrite(PinDir, HIGH);
        digitalWrite (PinEn, LOW);
        for (int z = 0; z < 80; z++) {
          digitalWrite(PinStep, HIGH);
          digitalWrite(PinStep, LOW);
          delayMicroseconds(150);
        }
        digitalWrite (PinEn, HIGH);
      }
      break;

    case 32:
      screen = 31;
      screen31B();
      delaybtn();
      break;

    case 42:
      screen = 41;
      screen41B();
      delaybtn();
      break;

    case 52:
      screen = 51;
      screen51B();
      delaybtn();
      break;

    case 102:
      screen = 101;
      screen101B();
      delaybtn();
      break;

    case 103:
      screen = 102;
      screen102();
      delaybtn();
      break;

    case 104:
      screen = 103;
      screen104();
      delaybtn();
      break;

    case 105:
      screen = 104;
      screen104();
      delaybtn();
      break;

    case 106:
      screen = 105;
      screen105();
      delaybtn();
      break;

    case 107:
      screen = 106;
      screen106();
      delaybtn();
      break;

    case 108:
      screen = 107;
      screen107();
      delaybtn();
      break;

    case 109:
      screen = 108;
      screen108();
      delaybtn();
      break;

    case 110:
      screen = 109;
      screen109();
      delaybtn();
      break;

    case 111:
      screen = 110;
      screen110();
      delaybtn();
      break;

    case 112:
      screen = 111;
      screen111();
      delaybtn();
      break;

    case 113:
      screen = 112;
      screen112();
      delaybtn();
      break;

    case 114:
      screen = 113;
      screen113();
      delaybtn();
      break;

    case 1010:
      if (PWMUV < 255 && screen == 1010) {
        PWMUV++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, PWMUV);
      }
      delaybtn();
      break;

    case 1020:
      if (PWMBL < 210 && screen == 1020) {
        PWMBL++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, PWMBL);
        ledcWrite(ledChannel1, PWMBL);
      }
      delaybtn();
      break;

    case 1030:
      if (hUpInitial < 15 && screen == 1030) {
        hUpInitial++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, hUpInitial);
      }
      delaybtn();
      break;

    case 1040:
      if (LiftSpeedInit < 8.1 && screen == 1040) {
        LiftSpeedInit = LiftSpeedInit + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeedInit);
        recalcTime();
      }
      delaybtn();
      break;

    case 1050:
      if (hUp < 15 && screen == 1050) {
        hUp++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, hUp);
      }
      delaybtn();
      break;


    case 1060:
      if (LiftSpeed < 8.1 && screen == 1060) {
        LiftSpeed = LiftSpeed + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeed);
        recalcTime();
      }
      delaybtn();
      break;


    case 1070:
      if (FirstLayers < 20 && screen == 1070) {
        FirstLayers++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, FirstLayers);
      }
      delaybtn();
      break;

    case 1080:
      if (RetractSpeed < 8 && screen == 1080) {
        RetractSpeed = RetractSpeed + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, RetractSpeed);
        recalcTime();
      }
      delaybtn();
      break;

    case 1090:
      if (RestTime < 30000 && screen == 1090) {
        RestTime = RestTime + 100;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, RestTime);
      }
      delaybtn();
      break;

    case 1100:
      if (Lguide < 1001 && screen == 1100) {
        Lguide = Lguide + 1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, Lguide);
      }
      delaybtn();
      break;


    case 1110:
      if (pitch < 10.1 && screen == 1110) {
        pitch = pitch + 0.01;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableFloat(240, 150, pitch);
      }
      delaybtn();
      break;


    case 1120:
      if (stepsMotorPerRev < 1201 && screen == 1120) {
        stepsMotorPerRev = stepsMotorPerRev + 1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, stepsMotorPerRev);
      }
      delaybtn();
      break;


    case 1130:
      if (microsteps < 513 && screen == 1130) {
        microsteps = microsteps + 1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, microsteps);
      }
      delaybtn();
      break;


  }
}

void btnDOWNmenu() {

  switch (screen) {

    case 0:
      displaymenusettings();
      screen = 1;
      break;

    case 11:
      folderDown(root);
      delaybtn();
      break;

    case 12:
      switch (hLayerx1000) {
        case 50:
          hLayer = 0.025;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 146, hLayerx1000);
          tft.drawCentreString("um", 270, 146, 4);
          adjuststeps();
          break;

        case 100:
          hLayer = 0.05;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 146, hLayerx1000);
          tft.drawCentreString("um", 270, 146, 4);
          adjuststeps();
          break;
      }
      delaybtn();
      break;

    case 13:
      if (expotime > 0.4 && screen == 13) {
        expotime = expotime - 0.2;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        if (expotime <= 9.8) {
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawVariableFloat(207, 146, expotime);
        }
        else if (expotime > 99.8) {
          drawVariableFloat3(200, 146, expotime);
        }
        else {
          drawVariableFloat2(200, 146, expotime);
        }
        tft.drawCentreString("s", 280, 146, 4);
      }
      delay(80);
      break;

    case 14:
      if (iexpotime > 1 && screen == 14) {
        iexpotime--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(235, 146, iexpotime);
        tft.drawCentreString("s", 280, 146, 4);
      }
      delay(80);
      break;

    case 21:                                   //screen 21 is when the platform is down during calibration
      if (stepsadditional <= maxAddSteps) {
        stepsadditional++;
        digitalWrite(PinDir, LOW);
        digitalWrite (PinEn, LOW);
        for (int z = 0; z < 80; z++) {
          digitalWrite(PinStep, HIGH);
          digitalWrite(PinStep, LOW);
          delayMicroseconds(150);
        }
        digitalWrite (PinEn, HIGH);
      }
      break;

    case 31:
      screen = 32;
      screen32();
      delaybtn();
      break;

    case 41:
      screen = 42;
      screen42();
      delaybtn();
      break;

    case 51:
      screen = 52;
      screen52();
      delaybtn();
      break;

    case 101:
      screen = 102;
      screen102();
      delaybtn();
      break;

    case 102:
      screen = 103;
      screen103();
      delaybtn();
      break;

    case 103:
      screen = 104;
      screen104();
      delaybtn();
      break;

    case 104:
      screen = 105;
      screen105();
      delaybtn();
      break;

    case 105:
      screen = 106;
      screen106();
      delaybtn();
      break;

    case 106:
      screen = 107;
      screen107();
      delaybtn();
      break;

    case 107:
      screen = 108;
      screen108();
      delaybtn();
      break;

    case 108:
      screen = 109;
      screen109();
      delaybtn();
      break;

    case 109:
      screen = 110;
      screen110();
      delaybtn();
      break;

    case 110:
      screen = 111;
      screen111();
      delaybtn();
      break;

    case 111:
      screen = 112;
      screen112();
      delaybtn();
      break;

    case 112:
      screen = 113;
      screen113();
      delaybtn();
      break;

    case 113:
      screen = 114;
      screen114();
      delaybtn();
      break;

    case 1010:
      if (PWMUV > 0 && screen == 1010) {
        PWMUV--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, PWMUV);
      }
      delaybtn();
      break;

    case 1020:
      if (PWMBL > 5 && screen == 1020) {
        PWMBL--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, PWMBL);
        ledcWrite(ledChannel1, PWMBL);
      }
      delaybtn();
      break;

    case 1030:
      if (hUpInitial > 0 && screen == 1030) {
        hUpInitial--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, hUpInitial);
        recalcTime();
      }
      delaybtn();
      break;


    case 1040:
      if (LiftSpeedInit > 0.1 && screen == 1040) {
        LiftSpeedInit = LiftSpeedInit - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeedInit);
        recalcTime();
      }
      delaybtn();
      break;


    case 1050:
      if (hUp > 0 && screen == 1050) {
        hUp--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, hUp);
        recalcTime();
      }
      delaybtn();
      break;

    case 1060:
      if (LiftSpeed > 0.1 && screen == 1060) {
        LiftSpeed = LiftSpeed - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeed);
        recalcTime();
      }
      delaybtn();
      break;


    case 1070:
      if (FirstLayers > 1 && screen == 1070) {
        FirstLayers--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, FirstLayers);
        recalcTime();
      }
      delaybtn();
      break;


    case 1080:
      if (RetractSpeed > 0.2 && screen == 1080) {
        RetractSpeed = RetractSpeed - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, RetractSpeed);
        recalcTime();
      }
      delaybtn();
      break;

    case 1090:
      if (RestTime >= 100 && screen == 1090) {
        RestTime = RestTime - 100;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, RestTime);
        recalcTime();
      }
      delaybtn();
      break;

    case 1100:
      if (Lguide >= 37 && screen == 1100) {
        Lguide = Lguide - 1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, Lguide);
      }
      delaybtn();
      break;


    case 1110:
      if (pitch >= 0.2 && screen == 1110) {
        pitch = pitch - 0.01;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableFloat(240, 150, pitch);
      }
      delaybtn();
      break;


    case 1120:
      if (stepsMotorPerRev >= 2 && screen == 1120) {
        stepsMotorPerRev = stepsMotorPerRev - 1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, stepsMotorPerRev);
      }
      delaybtn();
      break;


    case 1130:
      if (microsteps >= 2 && screen == 1130) {
        microsteps = microsteps - 1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, microsteps);
      }
      delaybtn();
      break;

  }
}


void btnNEXTmenu() {

  switch (screen) {

    case 0:

      screen31();
      screen = 31;
      break;

    case 1:
      screen = 101;
      screen101();
      break;

    case 11:
      contarlayers();
      slicerselect();
      if (LayersCounter > maxheight * 40 + 5) {
        cleanscreen();
        hExceeded();
      }

      else {
        preview();
      }
      break;

    case 12:
      screen = 13;
      screen13();
      break;

    case 13:
      WritePref();
      screen = 14;
      screen14();
      break;

    case 14:
      WritePref();
      screen = 15;
      screen15();
      break;

    case 15:
      switch (hLayerx1000) {

        case 25:
          Layers = LayersCounter;
          break;

        case 50:
          Layers = LayersCounter / 2;
          break;

        case 100:
          Layers = LayersCounter / 4;

          int resto = Layers * 2;
          resto = LayersCounter - resto;

          if (resto > 0) {
            Layers++;
          }

          break;
      }
      recalcTime();
      screen = 16;
      screen16();
      break;

    case 21:
      WritePref();
      OLEDwait();
      movasc(50, delayRetractSpeed);                   // 50 mm lift to make room for reading the display and tray entry.
      screen41();
      screen = 41;
      break;

    case 22:
      if (!SD.begin(SDcs, SD_SCK_MHZ(16))) {
        TFT_BLACKscreen();
        edoSdEnter = digitalRead (SdEnter);
        if (edoSdEnter == LOW) {
          SDnotformat();
        }
        else {
          SDnotfound();
        }
      }
      else {
        root = SD.open("/");
        screen11();
        screen = 11;
      }
      break;

    case 31:
      screen310();
      screen = 310;
      break;

    case 310:
      TFT_BLACKscreen();
      OLEDwait();
      calibrate();
      OLEDcalibrate();
      screen = 21;
      break;

    case 32:
      screen41();
      screen = 41;
      break;

    case 41:
      PrintMode = 0;
      if (!SD.begin(SDcs, SD_SCK_MHZ(16))) {
        TFT_BLACKscreen();
        edoSdEnter = digitalRead (SdEnter);
        if (edoSdEnter == LOW) {
          SDnotformat();
        }
        else {
          SDnotfound();
        }
      }

      else {
        root = SD.open("/");
        screen11();
        screen = 11;
      }
      break;

    case 42:
      screen51();
      screen = 51;
      break;

    case 51:
      screen12();
      screen = 12;
      PrintMode = 1;
      break;

    case 52:
      screen12();
      screen = 12;
      PrintMode = 2;
      break;

    case 101:
      screen = 1010;
      screen1010();
      break;

    case 102:
      screen = 1020;
      screen1020();
      break;

    case 103:
      screen = 1030;
      screen1030();
      break;

    case 104:
      screen = 1040;
      screen1040();
      break;

    case 105:
      screen = 1050;
      screen1050();
      break;

    case 106:
      screen = 1060;
      screen1060();
      break;

    case 107:
      screen = 1070;
      screen1070();
      break;

    case 108:
      screen = 1080;
      screen1080();
      break;

    case 109:
      screen = 1090;
      screen1090();
      break;

    case 110:
      screen = 1100;
      screen1100();
      break;

    case 111:
      screen = 1110;
      screen1110();
      break;

    case 112:
      screen = 1120;
      screen1120();
      break;

    case 113:
      screen = 1130;
      screen1130();
      break;

    case 114:
      screen = 1140;
      screen1140();
      break;
  }
}

void btnBACKmenu() {

  switch (screen) {

    case 1:
      screen = 0;
      displaymenuprint();
      break;

    case 11:
      screen = 41;
      screen41();
      break;


    case 31:
      screen = 0;
      displaymenuprint();
      OLEDblack();
      break;

    case 32:
      screen = 0;
      displaymenuprint();
      OLEDblack();
      break;

    case 310:
      screen = 31;
      screen31();
      break;

    case 41:
      screen = 31;
      screen31();
      break;

    case 42:
      screen = 31;
      screen31();
      break;

    case 51:
      screen = 41;
      screen41();
      break;

    case 52:
      screen = 41;
      screen41();
      break;

    case 12:

      if (PrintMode != 0) {
        screen = 41;
        screen41();
      }
      else {
        screen = 11;
        screen11();
      }

      break;

    case 13:
      screen = 12;
      screen12();
      break;

    case 14:
      screen = 13;
      screen13();
      break;

    case 15:
      screen = 14;
      screen14();
      break;

    case 16:
      screen = 15;
      screen15();
      break;

    case 17:
      screen = 0;
      ledcWrite(ledChannel1, PWMBL);
      displaymenuprint();
      break;

    case 22:
      screen = 31;
      screen31();
      break;

    case 101:
      screen = 1;
      displaymenusettings();
      break;

    case 102:
      screen = 1;
      displaymenusettings();
      break;

    case 103:
      screen = 1;
      displaymenusettings();
      break;

    case 104:
      screen = 1;
      displaymenusettings();
      break;

    case 105:
      screen = 1;
      displaymenusettings();
      break;

    case 106:
      screen = 1;
      displaymenusettings();
      break;

    case 107:
      screen = 1;
      displaymenusettings();
      break;

    case 108:
      screen = 1;
      displaymenusettings();
      break;

    case 109:
      screen = 1;
      displaymenusettings();
      break;

    case 110:
      screen = 1;
      displaymenusettings();
      break;

    case 111:
      screen = 1;
      displaymenusettings();
      break;

    case 112:
      screen = 1;
      displaymenusettings();
      break;

    case 113:
      screen = 1;
      displaymenusettings();
      break;

    case 114:
      screen = 1;
      displaymenusettings();
      break;

    case 320:
      screen = 31;
      screen31();
      break;

    case 1010:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 101;
      screen101();
      WritePref();
      break;

    case 1020:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 102;
      screen102();
      WritePref();
      break;

    case 1030:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 103;
      screen103();
      WritePref();
      break;

    case 1040:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 104;
      screen104();
      WritePref();
      break;

    case 1050:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 105;
      screen105();
      WritePref();
      break;

    case 1060:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 106;
      screen106();
      WritePref();
      break;

    case 1070:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 107;
      screen107();
      WritePref();
      break;

    case 1080:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 108;
      screen108();
      WritePref();
      break;

    case 1090:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 109;
      screen109();
      WritePref();
      break;

    case 1100:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 110;
      screen110();
      WritePref();
      break;

    case 1110:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 111;
      screen111();
      WritePref();
      break;

    case 1120:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 112;
      screen112();
      WritePref();
      break;

    case 1130:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 113;
      screen113();
      WritePref();
      break;

    case 1140:
      displaytemplate1();
      tft.drawCentreString("SETTINGS", 240, 25, 4);
      screen = 114;
      screen114();
      WritePref();
      break;

  }
}

void btnPLAYmenu() {
  switch (screen) {

    case 0:
      movasc(0.05, delayRetractSpeed);
      break;

    case 16:
      TFT_BLACKscreen();
      OLEDprintstart();
      switch (PrintMode) {
        case 0:
          print();
          break;

        case 1:
          printKeychain();
          break;

        case 2:
          printOLEDcover();
          break;

      }
      break;
  }
}

//***** MAIN FUNCTIONS *****

//Test mode function

void Testmode() {

  TFT_BLACKscreen();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("TEST MODE STARTED", 240, 25, 4);
  tft.drawCentreString("OLED DISPLAY SHOWS HI!?", 240, 155, 4);
  delay(1500);
  tft.drawCentreString("PRESS PLAY/PAUSE", 240, 280, 4);
  edoBtnPLAY = digitalRead (BtnPlay);
  while (edoBtnPLAY != LOW) {
    edoBtnPLAY = digitalRead (BtnPlay);
  }

  TFT_BLACKscreen();
  tft.fillCircle(240, 140, 30, TFT_WHITE);
  tft.drawCentreString("UV LIGHT ON?", 240, 25, 4);
  ledcWrite(ledChannel, 255);
  delay(1500);
  ledcWrite(ledChannel, 0);
  tft.drawCentreString("PRESS BACK", 240, 280, 4);
  edoBtnBACK = touchRead (BtnBack);
  while (edoBtnBACK > 50) {
    edoBtnBACK = touchRead (BtnBack);
  }

  TFT_BLACKscreen();
  tft.drawCentreString("STEPPER MOTOR MOVE?", 240, 25, 4);
  movasc(10, delayRetractSpeed);
  movdesc(10, delayRetractSpeed);
  tft.drawCentreString("PRESS NEXT", 240, 155, 4);
  edoBtnNEXT = touchRead (BtnNext);
  while (edoBtnNEXT > 50) {
    edoBtnNEXT = touchRead (BtnNext);
  }

  TFT_BLACKscreen();

  if (!SD.begin(SDcs, SD_SCK_MHZ(16))) {
    TFT_BLACKscreen();
    tft.drawCentreString("Please insert", 240, 100, 4);
    tft.drawCentreString("SD card and start again", 240, 140, 4);
    screen = 22;
  }
  else {
    root = SD.open("/");
  }
  FileName = "example.png";
  char NameChar[40];
  FileName.toCharArray(NameChar, 40);
  tft.setRotation(2);
  coordRot2();
  int rc = 0;
  rc = png.open((const char *)NameChar, myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rc == PNG_SUCCESS) {
    rc = png.decode(NULL, 0);
    png.close();
  }
  tft.setTextColor(TFT_BLACK);
  tft.setRotation(3);
  coordRot3();
  tft.drawCentreString("SD IMAGE DISPLAYED?", 240, 25, 4);
  tft.drawCentreString("PRESS DOWN", 240, 280, 4);
  edoBtnDOWN = touchRead (BtnDown);
  while (edoBtnDOWN > 50) {
    edoBtnDOWN = touchRead (BtnDown);
  }

  TFT_BLACKscreen();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("INTERRUPT ENDSTOP", 240, 140, 4);
  LectEndStop = digitalRead(PinEndStop);
  while (LectEndStop != HIGH) {
    LectEndStop = digitalRead(PinEndStop);
  }

  tft.fillScreen(TFT_GREEN);
  tft.drawCentreString("TEST FINISHED!", 240, 140, 4);
  delay(2000);
}





// Functions to access a file on the SD card

void SDnotfound() {
  displaytemplate3();
  tft.drawCentreString("SD CARD FAILURE", 240, 25, 4);
  tft.drawCentreString("Insert micro SD card.", 240, 140, 4);
  screen = 22;
}

void SDnotformat() {
  displaytemplate3();
  tft.drawCentreString("SD CARD FAILURE", 240, 25, 4);
  tft.drawCentreString("SD card inserted but", 240, 120, 4);
  tft.drawCentreString("not recognized.", 240, 150, 4);
  tft.drawCentreString("Format to FAT32.", 240, 230, 4);
  screen = 22;
}

void hExceeded() {
  displaytemplate3();
  tft.drawCentreString("HEIGHT EXCEEDED", 240, 25, 4);
  tft.drawCentreString("This file exceeds the", 240, 120, 4);
  tft.drawCentreString("maximum print height.", 240, 150, 4);
  delay(2000);
  screen = 11;
  screen11();
}


void * myOpen(const char *filename, int32_t *size) {
  Serial.printf("Attempting to open %s\n", filename);
  myfile = SD.open(filename);
  *size = myfile.size();
  return &myfile;
}
void myClose(void *handle) {
  if (myfile) myfile.close();
}
int32_t myRead(PNGFILE * handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}
int32_t mySeek(PNGFILE * handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}

void PNGDraw(PNGDRAW * pDraw) {
  uint16_t lineBuffer[426];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

void TransPNGDraw(PNGDRAW * pDraw) {
  uint16_t lineBuffer[426];          // Line buffer for rendering
  uint8_t  maskBuffer[1 + 426 / 8];  // Mask buffer
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  if (png.getAlphaMask(pDraw, maskBuffer, 255)) {
    tft.pushMaskedImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer, maskBuffer);
  }
}

//Read-Write preferences

void ReadPref() {
  preferences.begin("Settings", false);
  hUp = preferences.getInt("hUp", 4);
  hUpInitial = preferences.getInt("hUpInitial", 6);
  FirstLayers = preferences.getInt("FirstLayers", 4);
  expotime = preferences.getFloat("expotime", 6.0);
  iexpotime = preferences.getInt("iexpotime", 45);
  LiftSpeed = preferences.getFloat("LiftSpeed", 4);
  LiftSpeedInit = preferences.getFloat("LiftSpeedInit", 2);
  RetractSpeed = preferences.getFloat("RetractSpeed", 6.5);
  RestTime = preferences.getInt("RestTime", 0);
  stepsadditional = preferences.getLong("stepsadditional", 0);
  PWMUV = preferences.getInt("PWMUV", 255);
  PWMBL = preferences.getInt("PWMBL", 210);

  Lguide = preferences.getInt("Lguide", 115);
  pitch = preferences.getFloat("pitch", 2.44);
  stepsMotorPerRev = preferences.getInt("stepsMotorPerRev", 200);
  microsteps = preferences.getInt("microsteps", 64);
  preferences.end();

  //Adjust related parameters:
  maxheight = Lguide - 35;          //(35 mm lost due to carriage, platform and top bearing)
  StepsPerMm = stepsMotorPerRev * microsteps / pitch;
  maxAddSteps = maxAddDesc * StepsPerMm / 80;
}


void WritePref() {

  preferences.begin("Settings", false);
  preferences.putInt("hUp", hUp);
  preferences.putInt("hUpInitial", hUpInitial);
  preferences.putInt("FirstLayers", FirstLayers);
  preferences.putFloat("expotime", expotime);
  preferences.putInt("iexpotime", iexpotime);
  preferences.putFloat("LiftSpeed", LiftSpeed);
  preferences.putFloat("LiftSpeedInit", LiftSpeedInit);
  preferences.putFloat("RetractSpeed", RetractSpeed);
  preferences.putInt("RestTime", RestTime);
  preferences.putLong("stepsadditional", stepsadditional);
  preferences.putInt("PWMUV", PWMUV);
  preferences.putInt("PWMBL", PWMBL);
  preferences.putInt("Lguide", Lguide);
  preferences.putFloat("pitch", pitch);
  preferences.putInt("stepsMotorPerRev", stepsMotorPerRev);
  preferences.putInt("microsteps", microsteps);
  preferences.end();
}

void RestoreSettings() {

  hUp = 4;
  hUpInitial = 4;
  FirstLayers = 4;
  expotime = 6.0;
  iexpotime = 45;
  LiftSpeed = 2;
  LiftSpeedInit = 2;
  RetractSpeed = 4;
  RestTime = 0;
  stepsadditional = 0;
  PWMUV = 255;
  PWMBL = 210;
  Lguide = 115;
  pitch = 2.44;
  stepsMotorPerRev = 200;
  microsteps = 64;

}

void recalcTime() {

  delayLiftSpeedInit = 1000000 / (StepsPerMm * LiftSpeedInit);

  int timeLiftInit = StepsPerMm * delayLiftSpeedInit * hUpInitial / 1000000;


  delayLiftSpeed = 1000000 / (StepsPerMm * LiftSpeed);

  int timeLift = StepsPerMm * delayLiftSpeed * hUp / 1000000;


  delayRetractSpeed = 1000000 / (StepsPerMm * RetractSpeed);

  int timeRetractInit = StepsPerMm * delayRetractSpeed * hDownInitial / 1000000;

  int timeRetract = StepsPerMm * delayRetractSpeed * hDown / 1000000;


  updowntimeInit = timeLiftInit + topdelay / 1000 + timeRetractInit + RestTime / 1000;

  updowntime = timeLift + topdelay / 1000 + timeRetract + RestTime / 1000;

}


// PRINT FUNCTION

void print() {

  compTimeMillis = - millis();
  adjuststeps();
  ledcWrite(ledChannel1, 0);

  switch (slicernumber) {

    case 4: //Chitubox
      number = 1;
      break;

    default: //Voxeldance, Lychee, Prusa
      number = 0;
      break;

  }


  buildfolder();
  calibrate();
  delay(500);
  movasc(hLayer, delayRetractSpeed);
  OLEDprint();
  delay(4000);


  // Printing the first layers (with initial exposure time)

  for (int l = 0; l < FirstLayers ; l++) {

    delay(RestTime);
    printname();
    ledcWrite(ledChannel, PWMUV);
    delay(iexpotime * 1000);
    ledcWrite(ledChannel, 0);
    TFT_BLACKscreen();
    pausing();                         //allows to enter the pausing function by holding down the Play/Pause button
    movasc(hUpInitial, delayLiftSpeedInit);
    delay(topdelay);
    movdesc(hDownInitial, delayRetractSpeed);
    OLEDprint();

  }

  // Printing transition layer and updwontime measure


  delay(RestTime);
  printname();
  ledcWrite(ledChannel, PWMUV);
  float transitionexpotime = (expotime + iexpotime) / 2;
  delay(transitionexpotime * 1000);
  ledcWrite(ledChannel, 0);
  TFT_BLACKscreen();
  pausing();                         //allows to enter the pausing function by holding down the Play/Pause button
  long int updowntimeM = - millis();
  movasc(hUpInitial, delayLiftSpeedInit);
  delay(topdelay);
  movdesc(hDownInitial, delayRetractSpeed);
  OLEDprint();
  updowntimeM = updowntimeM + millis();
  updowntime = updowntimeM / 1000;


  // Printing the rest

  for (int l = 0; l < Layers - FirstLayers - 1; l++) {

    delay(RestTime);
    printname();
    ledcWrite(ledChannel, PWMUV);
    delay(expotime * 1000);
    ledcWrite(ledChannel, 0);
    TFT_BLACKscreen();
    pausing();                         //allows to enter the pausing function by holding down the Play/Pause button
    movasc(hUp, delayLiftSpeed);
    delay(topdelay);
    movdesc(hDown, delayRetractSpeed);
    OLEDprint();

  }

  compTimeMillis = compTimeMillis + millis();
  OLEDfinished();
  AdvLay = 0;
  screen = 17;
  screen17();

}

///////////////////////

// CALIBRATION FUNCTION

void calibrate() {

  ReadPref();

  LectEndStop = digitalRead(PinEndStop);

  if (LectEndStop != HIGH) {

    desctoendstop();

    delay(600);

    stepsadditionalx80 = stepsadditional * 80;

    digitalWrite (PinEn, LOW);
    digitalWrite(PinDir, LOW);

    for (int z = 0; z < stepsadditionalx80; z++) {
      digitalWrite(PinStep, HIGH);
      digitalWrite(PinStep, LOW);
      delayMicroseconds(delayLiftSpeedInit);
    }

  }
}

/////////////////////////////////////////////////////////////

// CALIBRATION AND PRINT SUPPORT FUNCTIONS

void folderDown(File dir) {
  counter++;
  for (int i = 0; i < counter; i++) {
    while (true) {
      File entry =  dir.openNextFile();
      if (! entry) {
        break;
      }
      if (entry.isDirectory()) {
        entry.getName(foldersel, 25);
        break;
      }
      entry.close();
    }
  }
  //  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString(foldersel, 245, 146, 4);
  delay(200);

}

void folderUp(File dir) {
  if (counter > 2) {
    counter --;
    for (int i = 0; i < counter; i++) {
      while (true) {
        File entry =  dir.openNextFile();
        if (! entry) {
          break;
        }
        if (entry.isDirectory()) {
          entry.getName(foldersel, 25);
          break;
        }
        entry.close();
      }
    }
    //    tft.fillRect(120, 128, 240, 45, TFT_BLACK);
    tft.fillRect(77, 128, 326, 50, TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString(foldersel, 245, 146, 4);
    delay(200);
  }

}


void movasc (float Mm, int delaysteps) {

  long int stepsmotor = StepsPerMm * Mm;

  digitalWrite (PinEn, LOW);
  digitalWrite(PinDir, HIGH);

  for (long int x = 0; x < stepsmotor; x++) {
    //      edoBtnBACK = digitalRead (BtnCancel);
    digitalWrite(PinStep, HIGH);
    digitalWrite(PinStep, LOW);
    delayMicroseconds(delaysteps);
  }

  //  digitalWrite (PinEn, HIGH);
}


void movdesc (float Mm, int delaysteps) {

  long int stepsmotor = StepsPerMm * Mm;

  digitalWrite (PinEn, LOW);
  digitalWrite(PinDir, LOW);

  for (long int x = 0; x < stepsmotor; x++) {
    //      edoBtnBACK = digitalRead (BtnCancel);
    digitalWrite(PinStep, HIGH);
    digitalWrite(PinStep, LOW);
    delayMicroseconds(delaysteps);
  }
  //  digitalWrite (PinEn, HIGH);
}


void buildfolder() {

  DirAndFile = "";
  String folderselString = String(foldersel);
  String barra = "/";
  DirAndFile += barra;
  DirAndFile += folderselString;
  DirAndFile += barra;
  FileName = DirAndFile;

}

void contarlayers() {

  LayersCounter = 0;
  dirfoldersel = "";
  String folderselString2 = String(foldersel);
  number = 1;
  dirfoldersel += "/";
  dirfoldersel += folderselString2;
  dirfoldersel += "/";
  char dirfolderselChar[40];
  dirfoldersel.toCharArray(dirfolderselChar, 40);
  File dircarp = SD.open(dirfolderselChar);
  while (true) {
    File entry =  dircarp.openNextFile();
    if (! entry) {
      break;
    }
    if (entry.isDirectory()) {
    } else {
      LayersCounter ++;
    }
    entry.close();
  }
}

void slicerselect() {

  //Chitubox: 1.png, 2.png, ...
  //Voxeldance: 0.png, 1.png, ...
  //Lychee: lychee0000.png, lychee0001.png,... or lychee000.png, lychee001.png,
  //Prusa: foldersel00000.png, foldersel00001.png,....

  buildfolder(); // To get FileName = "/foldersel/"
  String folderselStr = String(foldersel);
  String testname = DirAndFile;
  char testnameChar[40];
  char testname2Char[40];

  // Test Prusa Slicer
  testname += folderselStr ;
  testname += "00000.png" ;
  testname.toCharArray(testnameChar, 40);
  if (SD.exists(testnameChar)) {
    slicermode = "Prusa";
    slicernumber = 1;
  }

  // Test Lychee Slicer 1
  testname = DirAndFile;
  testname += "lychee0000.png" ;
  testname.toCharArray(testnameChar, 40);
  if (SD.exists(testnameChar)) {
    slicermode = "Lychee";
    slicernumber = 2;
  }

  // Test Lychee Slicer 2
  testname = DirAndFile;
  testname += "lychee000.png" ;
  testname.toCharArray(testnameChar, 40);
  if (SD.exists(testnameChar)) {
    slicermode = "Lychee";
    slicernumber = 22;
  }

  // Test Voxeldance & Chitubox Slicer
  testname = DirAndFile;
  testname += "0.png" ;
  testname.toCharArray(testnameChar, 40);
  String testname2 = DirAndFile;
  testname2 += "1.png" ;
  testname2.toCharArray(testname2Char, 40);

  if (SD.exists(testnameChar)) {
    slicermode = "Voxeldance";
    slicernumber = 3;
  }

  if (!SD.exists(testnameChar) && SD.exists(testname2Char)) {
    slicermode = "Chitubox";
    slicernumber = 4;
  }

}

void printname() {

  //Chitubox: 1.png, 2.png, ...
  //Voxeldance: 0.png, 1.png, ...
  //Lychee: lychee0000.png, lychee0001.png,...
  //Prusa: foldersel00000.png, foldersel00001.png,....

  String folderselSt = String(foldersel);

  switch (slicernumber) {

    case 2: //Lychee 1
      FileName += "lychee";
      if (number < 1000 && number >= 100) {
        FileName += "0";
      }
      else if (number < 100 && number >= 10) {
        FileName += "00";
      }
      else if (number < 10) {
        FileName += "000";
      }
      break;

    case 22: //Lychee 2
      FileName += "lychee";
      if (number < 100 && number >= 10) {
        FileName += "0";
      }
      else if (number < 10) {
        FileName += "00";
      }
      break;

    case 1: //Prusa
      FileName += folderselSt;
      if (number < 10000 && number >= 1000) {
        FileName += "0";
      }
      else if (number < 1000 && number >= 100) {
        FileName += "00";
      }
      else if (number < 100 && number >= 10) {
        FileName += "000";
      }
      else if (number < 10) {
        FileName += "0000";
      }
      break;

    default:  //Chitubox && Voxeldance
      break;
  }
  FileName += number;
  FileName += ".png";
  char NameChar[40];
  FileName.toCharArray(NameChar, 40);
  tft.setRotation(2);
  coordRot2();

  int rc = 0;
  //  dir = SD.open("/");
  rc = png.open((const char *)NameChar, myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rc == PNG_SUCCESS) {
    rc = png.decode(NULL, 0);
    png.close();
  }


  switch (hLayerx1000) {
    case 25:
      number ++;
      break;

    case 50:
      number = number + 2;
      break;

    case 100:
      number = number + 4;
      break;
  }

  FileName = DirAndFile;

}






void pausing() {

  edoBtnPLAY = digitalRead (BtnPlay);

  if (edoBtnPLAY == LOW) {
    display.clearDisplay();
    display.setCursor(7, 64);
    display.print("Pause");
    display.display();

    float heightActual = number * 0.025;
    float heightAdd = maxheight - heightActual;
    movasc(heightAdd, delayLiftSpeed);

    while (digitalRead (BtnPlay) != LOW) {
    }
    movdesc(heightAdd, delayRetractSpeed);
    delaybtn();
  }

  //Aquí podrían ser dos velocidades, lento cerca de pantalla

}



void desctoendstop() {

  digitalWrite (PinEn, LOW);
  digitalWrite(PinDir, LOW);
  LectEndStop = digitalRead(PinEndStop);

  while (LectEndStop != HIGH) {
    LectEndStop = digitalRead(PinEndStop);
    digitalWrite(PinStep, HIGH);
    digitalWrite(PinStep, LOW);
    delayMicroseconds(delayRetractSpeed);
  }
  delay(300);
}


void delaybtn() {
  delay(delaybutton);
}


void adjuststeps() {

  hDown = hUp - hLayer;
  hDownInitial = hUpInitial - hLayer;

}

////////////////////////////////////////////////////////////

//OLED screens

void OLEDintro() {
  delay(500); // wait for the OLED to power up
  display.begin(0x3C, true);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();
  display.setRotation(4);
  display.drawBitmap(0, 0, hi, 64, 128, 1);
  display.display();
}

void OLEDwait() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 50);
  display.print("Please");
  display.setCursor(17, 80);
  display.print("wait");
  display.display();
}


void OLEDcalibrate() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 30);
  display.print("1.Lock");
  display.setCursor(0, 55);
  display.print("platform");
  display.setCursor(0, 95);
  display.print("2.Press");
  display.setCursor(16, 115);
  display.print("next");
  display.display();
}

void OLEDprintstart() {
  display.clearDisplay();
  display.setCursor(20, 35);
  display.print("3D");
  display.setCursor(0, 65);
  display.print("printing");
  display.setCursor(0, 95);
  display.print("started!");
  display.display();
}


void OLEDblack() {
  display.clearDisplay();
  display.display();
}


void OLEDlevel() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(8, 20);
  display.print("SKIP:");
  display.setCursor(10, 40);
  display.print("Level");
  display.setCursor(18, 60);
  display.print("and");
  display.setCursor(5, 80);
  display.print("locked");
  display.setCursor(0, 100);
  display.print("platform");
  display.setCursor(15, 120);
  display.print("only");
  display.display();
}



void OLEDprintset1() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Printing");
  display.setCursor(0, 70);
  display.print("settings");
  display.setCursor(18, 105);
  display.print("1/6");
  display.display();
}

void OLEDprintset2() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Printing");
  display.setCursor(0, 70);
  display.print("settings");
  display.setCursor(18, 105);
  display.print("2/6");
  display.display();
}

void OLEDprintset3() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Printing");
  display.setCursor(0, 70);
  display.print("settings");
  display.setCursor(18, 105);
  display.print("3/6");
  display.display();
}

void OLEDprintset4() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Printing");
  display.setCursor(0, 70);
  display.print("settings");
  display.setCursor(18, 105);
  display.print("4/6");
  display.display();
}

void OLEDprintset5() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Printing");
  display.setCursor(0, 70);
  display.print("settings");
  display.setCursor(18, 105);
  display.print("5/6");
  display.display();
}

void OLEDprintset6() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Printing");
  display.setCursor(0, 70);
  display.print("settings");
  display.setCursor(18, 105);
  display.print("6/6");
  display.display();
}

void OLEDprint() {

  AdvLay++;

  long int timelayersinitial = FirstLayers * iexpotime;
  long int quantitylayersresto = Layers - FirstLayers;
  long int timerestodelayers = quantitylayersresto * expotime;
  long int timesubebajatot = Layers * (updowntime + RestTime / 1000);

  int AdvLayNI = AdvLay - FirstLayers - 1;
  long int timetotalseg;

  if (AdvLay <= FirstLayers) {
    timetotalseg = timesubebajatot + timelayersinitial + timerestodelayers - AdvLay * iexpotime - AdvLay * updowntime;
  }
  else {
    timetotalseg = timesubebajatot + timelayersinitial + timerestodelayers - FirstLayers * iexpotime - AdvLayNI * expotime - AdvLayNI * updowntime;
  }

  int timetotalmin = timetotalseg / 60;
  int timetotalhours = timetotalmin / 60;
  int restominutes = timetotalmin - timetotalhours * 60;

  int percentAdv = 100 * AdvLay / Layers;
  alfaDegTot = AdvLay * 360 / Layers;

  display.clearDisplay();
  display.setTextSize(1);

  display.drawCircle(xx0, yy0, 30, SH110X_WHITE);

  for (int i = 0; i <= alfaDegTot; i++) {
    alfaRad = i * 0.01745329252; //2pi/360
    xx1 = xx0 + radius * sin(alfaRad);
    yy1 = yy0 - radius * cos(alfaRad);
    display.drawLine(xx0, yy0, xx1, yy1, SH110X_WHITE);
  }

  display.fillCircle(xx0, yy0, 24, SH110X_BLACK);

  if (percentAdv < 10) {
    display.setCursor(20, 45);
  }
  else if (percentAdv >= 10 && percentAdv < 100) {
    display.setCursor(13, 45);
  }
  display.print(percentAdv);

  if (percentAdv < 10) {
    display.setCursor(31, 45);
    display.print("%");
  }
  else if (percentAdv >= 10 || percentAdv <= 99) {
    display.setCursor(34, 45);
    display.print("%");
  }



  display.drawRoundRect(3, 80, 58, 48, 4, SH110X_WHITE);
  display.drawLine(3, 96, 3, 112, SH110X_BLACK);
  display.drawLine(60, 96, 60, 112, SH110X_BLACK);
  display.drawLine(21, 80, 40, 80, SH110X_BLACK);
  display.drawLine(21, 127, 40, 127, SH110X_BLACK);

  if (timetotalhours < 10) {
    display.setCursor(20, 100);
  }
  else {
    display.setCursor(13, 100);
  }
  display.print(timetotalhours);

  if (timetotalhours < 10) {
    display.setCursor(31, 100);
  }
  else {
    display.setCursor(34, 100);
  }
  display.print("h");

  if (restominutes < 10) {
    display.setCursor(20, 118);
  }
  else {
    display.setCursor(13, 118);
  }
  display.print(restominutes);
  if (restominutes < 10) {
    display.setCursor(31, 118);
  }
  else {
    display.setCursor(34, 118);
  }
  display.print("m");
  display.display();

}


void OLEDfinished() {

  display.clearDisplay();
  display.setTextSize(1);

  display.drawLine(0, 5, 64, 5, SH110X_WHITE);
  display.setCursor(4, 40);
  display.print("COMP.");
  display.setCursor(10, 60);
  display.print("TIME:");

  int timetotalmin = compTimeMillis / 60000;
  int timetotalhours = timetotalmin / 60;
  int restominutes = timetotalmin - timetotalhours * 60;

  display.drawRoundRect(3, 80, 58, 48, 4, SH110X_WHITE);
  display.drawLine(3, 96, 3, 112, SH110X_BLACK);
  display.drawLine(60, 96, 60, 112, SH110X_BLACK);
  display.drawLine(21, 80, 40, 80, SH110X_BLACK);
  display.drawLine(21, 127, 40, 127, SH110X_BLACK);

  if (timetotalhours < 10) {
    display.setCursor(20, 100);
  }
  else {
    display.setCursor(13, 100);
  }
  display.print(timetotalhours);

  if (timetotalhours < 10) {
    display.setCursor(31, 100);
  }
  else {
    display.setCursor(34, 100);
  }
  display.print("h");

  if (restominutes < 10) {
    display.setCursor(20, 118);
  }
  else {
    display.setCursor(13, 118);
  }
  display.print(restominutes);
  if (restominutes < 10) {
    display.setCursor(31, 118);
  }
  else {
    display.setCursor(34, 118);
  }
  display.print("m");
  display.display();





}


////////////////////////////////////////////////////////////

// LCD SCREENS


void displaymenuintro() {
  int16_t rc2 = png.openFLASH((uint8_t *)menuintro, sizeof(menuintro), PNGDraw);
  if (rc2 == PNG_SUCCESS) {
    tft.startWrite();
    rc2 = png.decode(NULL, 0);
    tft.endWrite();
  }
  for (int l = 0; l < 255; l++) {
    ledcWrite(ledChannel1, l);
    delay(8);
  }

}


void displaymenuprint() {
  ledcWrite(ledChannel1, PWMBL);
  int16_t rc3 = png.openFLASH((uint8_t *)menuprint, sizeof(menuprint), PNGDraw);
  if (rc3 == PNG_SUCCESS) {
    tft.startWrite();
    rc3 = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void displaymenusettings() {
  int16_t rc4 = png.openFLASH((uint8_t *)menusettings, sizeof(menusettings), PNGDraw);
  if (rc4 == PNG_SUCCESS) {
    tft.startWrite();
    rc4 = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void displaytemplate1() {
  int16_t rc5 = png.openFLASH((uint8_t *)template1, sizeof(template1), PNGDraw);
  if (rc5 == PNG_SUCCESS) {
    tft.startWrite();
    rc5 = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void displaytemplate2A() {
  int16_t rc6 = png.openFLASH((uint8_t *)template2A, sizeof(template2A), TransPNGDraw);
  if (rc6 == PNG_SUCCESS) {
    tft.startWrite();
    rc6 = png.decode(NULL, 0);
    tft.endWrite();
  }
}


void displaytemplate2B() {
  int16_t rc7 = png.openFLASH((uint8_t *)template2B, sizeof(template2B), TransPNGDraw);
  if (rc7 == PNG_SUCCESS) {
    tft.startWrite();
    rc7 = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void displaytemplate3() {
  int16_t rc7 = png.openFLASH((uint8_t *)template3, sizeof(template3), PNGDraw);
  if (rc7 == PNG_SUCCESS) {
    tft.startWrite();
    rc7 = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void preview() {

  TFT_BLACKscreen();
  Layers = LayersCounter;
  String folderselSt = String(foldersel);

  switch (slicernumber) {

    case 4: //Chitubox
      number = 1;
      break;

    default: //Voxeldance, Lychee, Prusa
      number = 0;
      break;

  }

  buildfolder();

  int divlayers = Layers / lenpreview;
  int backornext = 0;

  for (int k = 0; k < lenpreview; k++) {
    edoBtnNEXT = touchRead (BtnNext);
    if (edoBtnNEXT < 11) {
      break;
    }
    tft.setRotation(3);
    coordRot3();
    tft.setTextColor(TFT_YELLOW);
    tft.drawCentreString("Preview", 240, 295, 4);

    switch (slicernumber) {

      case 2: //Lychee 1
        FileName += "lychee";
        if (number < 1000 && number >= 100) {
          FileName += "0";
        }
        else if (number < 100 && number >= 10) {
          FileName += "00";
        }
        else if (number < 10) {
          FileName += "000";
        }
        break;

      case 22: //Lychee 2
        FileName += "lychee";
        if (number < 100 && number >= 10) {
          FileName += "0";
        }
        else if (number < 10) {
          FileName += "00";
        }
        break;

      case 1: //Prusa
        FileName += folderselSt;
        if (number < 10000 && number >= 1000) {
          FileName += "0";
        }
        else if (number < 1000 && number >= 100) {
          FileName += "00";
        }
        else if (number < 100 && number >= 10) {
          FileName += "000";
        }
        else if (number < 10) {
          FileName += "0000";
        }
        break;

      default:  //Chitubox && Voxeldance
        break;
    }

    FileName += number;
    FileName += ".png";
    char NameChar[40];
    FileName.toCharArray(NameChar, 40);
    tft.setRotation(2);
    coordRot2();
    int rc = 0;
    rc = png.open((const char *)NameChar, myOpen, myClose, myRead, mySeek, PNGDraw);
    if (rc == PNG_SUCCESS) {
      rc = png.decode(NULL, 0);
      png.close();
    }
    number = number + divlayers;
    FileName = DirAndFile;
  }
  tft.setTextColor(TFT_WHITE);
  tft.setRotation(3);
  coordRot3();

  screen = 12;
  screen12();

}


void screen11() {

  ReadPref();
  OLEDprintset1();
  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("SELECT FILE", 240, 25, 4);
  tft.drawCentreString(foldersel, 245, 146, 4);
}


void screen12() {

  OLEDprintset2();
  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("LAYER HEIGHT", 240, 25, 4);
  drawCentreVariableInt(220, 146, hLayerx1000);
  tft.drawCentreString("um", 270, 146, 4);

}


void screen13() {

  OLEDprintset3();
  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("EXPOSURE TIME", 240, 25, 4);
  if (expotime <= 9.8) {
    tft.fillRect(120, 128, 240, 45, TFT_BLACK);
    drawVariableFloat(207, 146, expotime);
  }
  else if (expotime >= 100) {
    drawVariableFloat3(200, 146, expotime);
  }
  else {
    drawVariableFloat2(200, 146, expotime);
  }
  tft.drawCentreString("s", 280, 146, 4);

}


void screen14() {

  OLEDprintset4();
  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("INITIAL EXPOSURE", 240, 25, 4);
  drawCentreVariableInt(235, 146, iexpotime);
  tft.drawCentreString("s", 280, 146, 4);

}


void screen15() {

  OLEDprintset5();
  displaytemplate3();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("CONFIRMATION", 240, 25, 4);

  tft.drawString("FOLDER:", 88, 110, 4);

  switch (PrintMode) {

    case 0:
      tft.drawString(foldersel, 230, 110, 4);
      break;

    case 1:
      tft.drawString("Keychain", 230, 110, 4);
      break;

    case 2:
      tft.drawString("OLED cover", 230, 110, 4);
      break;

  }

  tft.drawString("LAYER HEIGHT(um):", 88, 154, 4);
  drawVariableInt(330, 154, hLayerx1000);

  tft.drawString("EXPOSURE TIME(s):", 88, 198, 4);
  if (expotime < 10) {
    drawVariableFloat(330, 198, expotime);
  }
  if (expotime >= 100) {
    drawVariableFloat3(330, 198, expotime);
  }
  else {
    drawVariableFloat2(330, 198, expotime);
  }

  tft.drawString("INITIAL EXPOSURE(s):", 88, 242, 4);
  drawVariableInt(345, 242, iexpotime);

}


void screen16() {

  OLEDprintset6();

  displaytemplate3();
  maskRight();
  tft.setRotation(3);
  coordRot3();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("DETAILS", 240, 25, 4);
  tft.drawString("TOTAL LAYERS:", 90, 110, 4);

  switch (PrintMode) {

    case 0:
      break;

    case 1:
      switch (hLayerx1000) {

        case 25:
          Layers = 144;
          break;

        case 50:
          Layers = 72;
          break;

        case 100:
          Layers = 36;
          break;
      }

      break;

    case 2:

      switch (hLayerx1000) {

        case 25:
          Layers = 1412;
          break;

        case 50:
          Layers = 706;
          break;

        case 100:
          Layers = 353;
          break;
      }
      break;
  }

  drawVariableInt(310, 110, Layers);


  long int timelayersinitial = FirstLayers * iexpotime;
  long int quantitylayersresto = Layers - FirstLayers;
  long int timerestodelayers = quantitylayersresto * expotime;
  long int timesubebajatot = Layers * updowntime;
  long int timetotalseg = timesubebajatot + timelayersinitial + timerestodelayers;
  long int timetotalmin = timetotalseg / 60;
  int timetotalhours = timetotalmin / 60;
  int restominutes = timetotalmin - timetotalhours * 60;

  tft.drawString("DURATION:", 90, 155, 4);
  if (timetotalhours < 10) {
    drawVariableInt(257, 155, timetotalhours);
    tft.drawString("h", 272, 155, 4);
  }
  else {
    drawVariableInt(242, 155, timetotalhours);
    tft.drawString("h", 272, 155, 4);
  }

  if (restominutes < 10) {
    drawVariableInt(320, 155, restominutes);
    tft.drawString("min", 335, 155, 4);
  }
  else {
    drawVariableInt(305, 155, restominutes);
    tft.drawString("min", 335, 155, 4);
  }

  tft.drawCentreString("Insert tray and press START!", 240, 230, 4);
}


void screen17() {

  tft.setRotation(3);
  coordRot3();
  TFT_BLACKscreen();

  int heightup = Layers * hLayer;
  int heightremain = maxheight - heightup;

  movasc(heightremain, delayLiftSpeed);
}


void screen31() {

  OLEDlevel();

  tft.setRotation(3);
  coordRot3();
  TFT_BLACKscreen();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("LEVEL PLATFORM", 240, 25, 4);

  tft.drawCentreString("LEVEL", 240, 117, 4);
  tft.drawCentreString("SKIP", 240, 232, 4);
  displaytemplate2A();
}

void screen31B() {
  displaytemplate2A();
}

void screen310() {

  displaytemplate3();
  tft.drawCentreString("LEVEL PLATFORM", 240, 25, 4);
  tft.drawCentreString("PLACE PLATFORM", 240, 140, 4);
  tft.drawCentreString("AND SPACER", 240, 180, 4);
}

void screen32() {

  displaytemplate2B();
}


void screen41() {

  tft.setRotation(3);
  coordRot3();
  TFT_BLACKscreen();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("FILE", 240, 25, 4);

  tft.drawCentreString("MICRO SD", 240, 117, 4);
  tft.drawCentreString("TEST PRINT", 240, 232, 4);
  displaytemplate2A();
}

void screen41B() {
  displaytemplate2A();
}


void screen42() {

  displaytemplate2B();
}

void screen51() {

  tft.setRotation(3);
  coordRot3();
  TFT_BLACKscreen();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("TEST PRINT", 240, 25, 4);
  tft.drawCentreString("KEYCHAIN", 240, 117, 4);
  tft.drawCentreString("OLED COVER", 240, 232, 4);
  displaytemplate2A();
}

void screen51B() {
  displaytemplate2A();
}

void screen52() {
  displaytemplate2B();
}

void screen101() {

  tft.setRotation(3);
  coordRot3();
  TFT_BLACKscreen();
  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("SETTINGS", 240, 25, 4);
  tft.drawCentreString("UV POWER", 240, 150, 4);
}


void screen101B() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("UV POWER", 240, 150, 4);
}

void screen1010() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("UV POWER", 240, 25, 4);
  drawCentreVariableInt(240, 150, PWMUV);
  tft.drawCentreString("(0-255)", 240, 212, 4);
}


void screen102() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("WHITE BACKLIGHT", 240, 150, 4);
}


void screen1020() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("WHITE BACKLIGHT", 240, 25, 4);
  drawCentreVariableInt(240, 150, PWMBL);
  tft.drawCentreString("(0-210)", 240, 212, 4);
}


void screen103() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("BOTTOM LIFT HEIGHT", 240, 150, 4);
}


void screen1030() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("BOTTOM LIFT HEIGHT", 240, 25, 4);
  drawCentreVariableInt(240, 150, hUpInitial);
  tft.drawCentreString("(mm)", 240, 212, 4);
}


void screen104() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("BOTTOM LIFT SPEED", 240, 150, 4);
}


void screen1040() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("BOTTOM LIFT SPEED", 240, 25, 4);
  drawCentreVariableFloat(240, 150, LiftSpeedInit);
  tft.drawCentreString("(mm/s)", 240, 212, 4);
}


void screen105() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("LIFT HEIGHT", 240, 150, 4);
}


void screen1050() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("LIFT HEIGHT", 240, 25, 4);
  drawCentreVariableInt(240, 150, hUp);
  tft.drawCentreString("(mm)", 240, 212, 4);
}


void screen106() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("LIFT SPEED", 240, 150, 4);
}


void screen1060() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("LIFT SPEED", 240, 25, 4);
  drawCentreVariableFloat(240, 150, LiftSpeed);
  tft.drawCentreString("(mm/s)", 240, 212, 4);
}


void screen107() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("BOTTOM LAYER COUNT", 240, 150, 4);
}


void screen1070() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("BOTTOM LAYER COUNT", 240, 25, 4);
  drawCentreVariableInt(240, 150, FirstLayers);
}


void screen108() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("RETRACT SPEED", 240, 150, 4);
}


void screen1080() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("RETRACT SPEED", 240, 25, 4);
  drawCentreVariableFloat(240, 150, RetractSpeed);
  tft.drawCentreString("(mm/s)", 240, 212, 4);
}


void screen109() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("REST TIME AFTER RETRACT", 240, 150, 4);

}


void screen1090() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("REST TIME AFTER RETRACT", 240, 25, 4);
  drawCentreVariableInt(240, 150, RestTime);
  tft.drawCentreString("(ms)", 240, 212, 4);

}


void screen110() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("GUIDE LENGTH (NOT GEN2)", 240, 150, 4);

}


void screen1100() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("GUIDE LENGTH", 240, 25, 4);
  drawCentreVariableInt(240, 150, Lguide);
  tft.drawCentreString("(mm)", 240, 212, 4);
}


void screen111() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("THR. PITCH (NOT GEN2)", 240, 150, 4);
}


void screen1110() {
  displaytemplate1();
  maskRight();
  tft.drawCentreString("THREAD ROD PITCH", 240, 25, 4);
  drawCentreVariableFloat(240, 150, pitch);
  tft.drawCentreString("(mm)", 240, 212, 4);

}


void screen112() {
  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("STEPS/REV. (NOT GEN2)", 240, 150, 4);
}


void screen1120() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("STEPS PER REV.", 240, 25, 4);
  drawCentreVariableInt(240, 150, stepsMotorPerRev);

}


void screen113() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("MICROSTEPS (NOT GEN2)", 240, 150, 4);
}


void screen1130() {

  displaytemplate1();
  maskRight();
  tft.drawCentreString("MICROSTEPS", 240, 25, 4);
  drawCentreVariableInt(240, 150, microsteps);
}



void screen114() {

  tft.fillRect(77, 128, 326, 50, TFT_BLACK);
  tft.drawCentreString("RESTORE SETTINGS", 240, 150, 4);

}


void screen1140() {

  RestoreSettings();
  WritePref();
  displaytemplate3();
  maskRight();
  tft.drawCentreString("RESTORE SETTINGS", 240, 25, 4);
  tft.drawCentreString("SETTINGS RESTORED!", 240, 160, 4);

}


////////////////////////////////////////////////////////////

// SCREEN SUPPORT FUNCTIONS

void maskLeft() {

  tft.fillRect(0, 0, 77, 56, TFT_BLACK);
}

void maskRight() {

  tft.fillRect(379, 0, 60, 56, TFT_BLACK);
}

void maskDown() {

  tft.fillRect(100, 231, 95, 60, TFT_BLACK);
}

void maskUp() {

  tft.fillRect(285, 231, 95, 60, TFT_BLACK);
}

void coordRot3() {
  xpos = 27;
  ypos = 0;
}

void coordRot2() {
  xpos = 0;
  ypos = 27;
}

void drawCentreVariableInt(int xPos, int yPos, int vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[7];
  vartoprint.toCharArray(vartodrawChar, 7);
  tft.drawCentreString(vartodrawChar, xPos, yPos, 4);
}

void drawVariableInt(int xPos, int yPos, int vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[7];
  vartoprint.toCharArray(vartodrawChar, 7);
  tft.drawString(vartodrawChar, xPos, yPos, 4);
}

void drawCentreVariableFloat(int xPos, int yPos, float vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[6];
  vartoprint.toCharArray(vartodrawChar, 6);
  tft.drawCentreString(vartodrawChar, xPos, yPos, 4);

}

void drawVariableFloat(int xPos, int yPos, float vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[4];
  vartoprint.toCharArray(vartodrawChar, 4);
  tft.drawString(vartodrawChar, xPos, yPos, 4);

}

void drawVariableFloat2(int xPos, int yPos, float vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[5];
  vartoprint.toCharArray(vartodrawChar, 5);
  tft.drawString(vartodrawChar, xPos, yPos, 4);

}

void drawVariableFloat3(int xPos, int yPos, float vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[6];
  vartoprint.toCharArray(vartodrawChar, 6);
  tft.drawString(vartodrawChar, xPos, yPos, 4);

}


void arrows() {

  tft.fillTriangle(260, 135, 280, 135, 270, 120, TFT_WHITE );  //up arrow
  tft.fillTriangle(260, 165, 280, 165, 270, 180, TFT_WHITE );  //down arrow
}


void rectscreen() {

  tft.drawRect(120, 110, 80, 40, TFT_WHITE);
}


void TFT_BLACKscreen() {

  tft.fillScreen(TFT_BLACK);
}


void cleanscreen() {
  tft.fillRect(0, 40, 426, 280, TFT_BLACK);
}



void rectTFT_BLUE () {

  tft.fillRect(0, 0, 320, 40, TFT_BLUE);
}



void bannerprint() {

  rectTFT_BLUE();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("PRINT", 160, 6, 4);

}


void bannerpreparation() {

  rectTFT_BLUE ();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Lite3DP.com", 160, 6, 4);

}


void printKeychain() {

  /*
     0,025: (A:1-38), (B:39-76), (C:77-96), (D:97-144), TL = 144
     0,050: (A:1-19), (B:20-38), (C:39-48), (D:49-72), TL = 72
     0,100: (A:1-10), (B:11-19), (C:20-24), (D:25-36), TL = 36
  */
  tft.setRotation(2);
  coordRot2();
  FirstLayers = 5;
  compTimeMillis = - millis();
  adjuststeps();
  ledcWrite(ledChannel1, 0);
  calibrate();
  delay(500);
  movasc(hLayer, delayRetractSpeed);
  OLEDprint();
  delay(4000);
  number = 0;

  switch (hLayerx1000) {

    case 25:
      Layers = 144;

      //1-5
      for (int l = 1; l <= FirstLayers ; l++) {
        delay(RestTime);
        int16_t rc8 = png.openFLASH((uint8_t *)AA, sizeof(AA), PNGDraw);
        if (rc8 == PNG_SUCCESS) {
          tft.startWrite();
          rc8 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(iexpotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUpInitial, delayLiftSpeedInit);
        delay(topdelay);
        movdesc(hDownInitial, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //6-38
      for (int l = 6; l <= 38; l++) {
        delay(RestTime);
        int16_t rc9 = png.openFLASH((uint8_t *)AA, sizeof(AA), PNGDraw);
        if (rc9 == PNG_SUCCESS) {
          tft.startWrite();
          rc9 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //39-76
      for (int l = 39; l <= 76; l++) {
        delay(RestTime);
        int16_t rc10 = png.openFLASH((uint8_t *)AB, sizeof(AB), PNGDraw);
        if (rc10 == PNG_SUCCESS) {
          tft.startWrite();
          rc10 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //77-96
      for (int l = 77; l <= 96; l++) {
        delay(RestTime);
        int16_t rc11 = png.openFLASH((uint8_t *)AC, sizeof(AC), PNGDraw);
        if (rc11 == PNG_SUCCESS) {
          tft.startWrite();
          rc11 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //97-144
      for (int l = 97; l <= 144; l++) {
        delay(RestTime);
        int16_t rc12 = png.openFLASH((uint8_t *)AD, sizeof(AD), PNGDraw);
        if (rc12 == PNG_SUCCESS) {
          tft.startWrite();
          rc12 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }
      break;


    case 50:
      Layers = 72;

      //1-5
      for (int l = 1; l <= FirstLayers ; l++) {
        delay(RestTime);
        int16_t rc8 = png.openFLASH((uint8_t *)AA, sizeof(AA), PNGDraw);
        if (rc8 == PNG_SUCCESS) {
          tft.startWrite();
          rc8 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(iexpotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUpInitial, delayLiftSpeedInit);
        delay(topdelay);
        movdesc(hDownInitial, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //6-19
      for (int l = 6; l <= 19; l++) {
        delay(RestTime);
        int16_t rc9 = png.openFLASH((uint8_t *)AA, sizeof(AA), PNGDraw);
        if (rc9 == PNG_SUCCESS) {
          tft.startWrite();
          rc9 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //20-38
      for (int l = 20; l <= 38; l++) {
        delay(RestTime);
        int16_t rc10 = png.openFLASH((uint8_t *)AB, sizeof(AB), PNGDraw);
        if (rc10 == PNG_SUCCESS) {
          tft.startWrite();
          rc10 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //39-48
      for (int l = 39; l <= 48; l++) {
        delay(RestTime);
        int16_t rc11 = png.openFLASH((uint8_t *)AC, sizeof(AC), PNGDraw);
        if (rc11 == PNG_SUCCESS) {
          tft.startWrite();
          rc11 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //49-72
      for (int l = 49; l <= 72; l++) {
        delay(RestTime);
        int16_t rc12 = png.openFLASH((uint8_t *)AD, sizeof(AD), PNGDraw);
        if (rc12 == PNG_SUCCESS) {
          tft.startWrite();
          rc12 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }
      break;

    case 100:
      Layers = 36;

      //1-5
      for (int l = 1; l <= FirstLayers ; l++) {
        delay(RestTime);
        int16_t rc8 = png.openFLASH((uint8_t *)AA, sizeof(AA), PNGDraw);
        if (rc8 == PNG_SUCCESS) {
          tft.startWrite();
          rc8 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(iexpotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUpInitial, delayLiftSpeedInit);
        delay(topdelay);
        movdesc(hDownInitial, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //6-10
      for (int l = 6; l <= 10; l++) {
        delay(RestTime);
        int16_t rc9 = png.openFLASH((uint8_t *)AA, sizeof(AA), PNGDraw);
        if (rc9 == PNG_SUCCESS) {
          tft.startWrite();
          rc9 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //11-19
      for (int l = 11; l <= 19; l++) {
        delay(RestTime);
        int16_t rc10 = png.openFLASH((uint8_t *)AB, sizeof(AB), PNGDraw);
        if (rc10 == PNG_SUCCESS) {
          tft.startWrite();
          rc10 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //20-24
      for (int l = 20; l <= 24; l++) {
        delay(RestTime);
        int16_t rc11 = png.openFLASH((uint8_t *)AC, sizeof(AC), PNGDraw);
        if (rc11 == PNG_SUCCESS) {
          tft.startWrite();
          rc11 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //25-36
      for (int l = 25; l <= 36; l++) {
        delay(RestTime);
        int16_t rc12 = png.openFLASH((uint8_t *)AD, sizeof(AD), PNGDraw);
        if (rc12 == PNG_SUCCESS) {
          tft.startWrite();
          rc12 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }
      break;

  }
  compTimeMillis = compTimeMillis + millis();
  OLEDfinished();
  AdvLay = 0;
  screen = 17;
  screen17();
}



/////////////////////////////////////////



void printOLEDcover() {

  /*
     0,025: (A:1-20), (B:21-282), (C:283-480), (D:481-560), (C:561-1232), (B:1232-1412), TL = 1412
     0,050: (A:1-10), (B:11-141), (C:142-240), (D:241-280), (C:281-616), (B:617-706), TL = 706
     0,100: (A:1-5), (B:6-71), (C:72-120), (D:121-140), (C:141-308), (B:309-353), TL = 353
  */
  tft.setRotation(2);
  coordRot2();
  FirstLayers = 5;
  compTimeMillis = - millis();
  adjuststeps();
  ledcWrite(ledChannel1, 0);
  calibrate();
  delay(500);
  movasc(hLayer, delayRetractSpeed);
  OLEDprint();
  delay(4000);
  number = 0;

  switch (hLayerx1000) {

    case 25:
      Layers = 1412;

      //1-5
      for (int l = 1; l <= FirstLayers ; l++) {
        delay(RestTime);
        int16_t rc8 = png.openFLASH((uint8_t *)BA, sizeof(BA), PNGDraw);
        if (rc8 == PNG_SUCCESS) {
          tft.startWrite();
          rc8 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(iexpotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUpInitial, delayLiftSpeedInit);
        delay(topdelay);
        movdesc(hDownInitial, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //6-20
      for (int l = 6; l <= 20; l++) {
        delay(RestTime);
        int16_t rc9 = png.openFLASH((uint8_t *)BA, sizeof(BA), PNGDraw);
        if (rc9 == PNG_SUCCESS) {
          tft.startWrite();
          rc9 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //21-282
      for (int l = 21; l <= 282; l++) {
        delay(RestTime);
        int16_t rc10 = png.openFLASH((uint8_t *)BB, sizeof(BB), PNGDraw);
        if (rc10 == PNG_SUCCESS) {
          tft.startWrite();
          rc10 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //283-480
      for (int l = 283; l <= 480; l++) {
        delay(RestTime);
        int16_t rc11 = png.openFLASH((uint8_t *)BC, sizeof(BC), PNGDraw);
        if (rc11 == PNG_SUCCESS) {
          tft.startWrite();
          rc11 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //481-560
      for (int l = 481; l <= 560; l++) {
        delay(RestTime);
        int16_t rc12 = png.openFLASH((uint8_t *)BD, sizeof(BD), PNGDraw);
        if (rc12 == PNG_SUCCESS) {
          tft.startWrite();
          rc12 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //561-1232
      for (int l = 561; l <= 1232; l++) {
        delay(RestTime);
        int16_t rc13 = png.openFLASH((uint8_t *)BC, sizeof(BC), PNGDraw);
        if (rc13 == PNG_SUCCESS) {
          tft.startWrite();
          rc13 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      //1233-1412
      for (int l = 1233; l <= 1412; l++) {
        delay(RestTime);
        int16_t rc14 = png.openFLASH((uint8_t *)BB, sizeof(BB), PNGDraw);
        if (rc14 == PNG_SUCCESS) {
          tft.startWrite();
          rc14 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number ++;
      }

      break;


    case 50:
      Layers = 706;

      //1-5
      for (int l = 1; l <= FirstLayers ; l++) {
        delay(RestTime);
        int16_t rc8 = png.openFLASH((uint8_t *)BA, sizeof(BA), PNGDraw);
        if (rc8 == PNG_SUCCESS) {
          tft.startWrite();
          rc8 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(iexpotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUpInitial, delayLiftSpeedInit);
        delay(topdelay);
        movdesc(hDownInitial, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //6-10
      for (int l = 6; l <= 10; l++) {
        delay(RestTime);
        int16_t rc9 = png.openFLASH((uint8_t *)BA, sizeof(BA), PNGDraw);
        if (rc9 == PNG_SUCCESS) {
          tft.startWrite();
          rc9 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //11-141
      for (int l = 11; l <= 141; l++) {
        delay(RestTime);
        int16_t rc10 = png.openFLASH((uint8_t *)BB, sizeof(BB), PNGDraw);
        if (rc10 == PNG_SUCCESS) {
          tft.startWrite();
          rc10 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //142-240
      for (int l = 142; l <= 240; l++) {
        delay(RestTime);
        int16_t rc11 = png.openFLASH((uint8_t *)BC, sizeof(BC), PNGDraw);
        if (rc11 == PNG_SUCCESS) {
          tft.startWrite();
          rc11 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //241-280
      for (int l = 241; l <= 280; l++) {
        delay(RestTime);
        int16_t rc12 = png.openFLASH((uint8_t *)BD, sizeof(BD), PNGDraw);
        if (rc12 == PNG_SUCCESS) {
          tft.startWrite();
          rc12 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //281-616
      for (int l = 281; l <= 616; l++) {
        delay(RestTime);
        int16_t rc13 = png.openFLASH((uint8_t *)BC, sizeof(BC), PNGDraw);
        if (rc13 == PNG_SUCCESS) {
          tft.startWrite();
          rc13 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }

      //617-706
      for (int l = 617; l <= 706; l++) {
        delay(RestTime);
        int16_t rc14 = png.openFLASH((uint8_t *)BB, sizeof(BB), PNGDraw);
        if (rc14 == PNG_SUCCESS) {
          tft.startWrite();
          rc14 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 2;
      }
      break;

    case 100:
      Layers = 353;

      //1-5
      for (int l = 1; l <= FirstLayers ; l++) {
        delay(RestTime);
        int16_t rc8 = png.openFLASH((uint8_t *)BA, sizeof(BA), PNGDraw);
        if (rc8 == PNG_SUCCESS) {
          tft.startWrite();
          rc8 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(iexpotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUpInitial, delayLiftSpeedInit);
        delay(topdelay);
        movdesc(hDownInitial, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //6-71
      for (int l = 6; l <= 71; l++) {
        delay(RestTime);
        int16_t rc10 = png.openFLASH((uint8_t *)BB, sizeof(BB), PNGDraw);
        if (rc10 == PNG_SUCCESS) {
          tft.startWrite();
          rc10 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //72-120
      for (int l = 72; l <= 120; l++) {
        delay(RestTime);
        int16_t rc11 = png.openFLASH((uint8_t *)BC, sizeof(BC), PNGDraw);
        if (rc11 == PNG_SUCCESS) {
          tft.startWrite();
          rc11 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //121-140
      for (int l = 121; l <= 140; l++) {
        delay(RestTime);
        int16_t rc12 = png.openFLASH((uint8_t *)BD, sizeof(BD), PNGDraw);
        if (rc12 == PNG_SUCCESS) {
          tft.startWrite();
          rc12 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //141-308
      for (int l = 141; l <= 308; l++) {
        delay(RestTime);
        int16_t rc13 = png.openFLASH((uint8_t *)BC, sizeof(BC), PNGDraw);
        if (rc13 == PNG_SUCCESS) {
          tft.startWrite();
          rc13 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }

      //309-353
      for (int l = 309; l <= 353; l++) {
        delay(RestTime);
        int16_t rc14 = png.openFLASH((uint8_t *)BB, sizeof(BB), PNGDraw);
        if (rc14 == PNG_SUCCESS) {
          tft.startWrite();
          rc14 = png.decode(NULL, 0);
          tft.endWrite();
        }
        ledcWrite(ledChannel, PWMUV);
        delay(expotime * 1000);
        ledcWrite(ledChannel, 0);
        TFT_BLACKscreen();
        pausing();
        movasc(hUp, delayLiftSpeed);
        delay(topdelay);
        movdesc(hDown, delayRetractSpeed);
        OLEDprint();
        number = number + 4;
      }
      break;


  }
  compTimeMillis = compTimeMillis + millis();
  OLEDfinished();
  AdvLay = 0;
  screen = 17;
  screen17();
}
