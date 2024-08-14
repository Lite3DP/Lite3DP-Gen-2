////////////////////////////////////////////////////////////

/*
  Lite3DP Gen 2 v1.0 - OLED auxiliary display

  https://github.com/Lite3DP/Lite3DP-Gen-2

  MIT License

  Please visit www.lite3dp.com for more information, documentation and software.

  email: support@lite3dp.com

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

  3) Connect the main board with the USB-C auxiliary board. Please be careful, an incorrect position could damage the main board.

  4) On the Gen 2 main board, press the RST button, and without releasing it, press the BOOT button. Release RST and then release BOOT.

  5) Upload firmware and reset.

*/

////////////////////////////////////////////////////////////

// **** PARAMETERS ****

float hLayer;              // Layer height (mm)

float hUp;                   // Lift height (mm)

float hUpInitial;            // Inital layers lift height (mm)

int FirstLayers;           //Number of bottom layers

int TraLayers;       //Number of transition layers

float expotime;            //Exposure time (s)

int iexpotime;             //Bottom exposure time (s)

float LiftSpeed;             // Lift speed (mm/s)

float LiftSpeedInit;         // Bottom lift speed (mm/s)

float RetractSpeed;          // Retract speed (mm/s)

int RestTime;             // Delay after retract (ms)

int PWMUV;                 // UV power (0-255)

int PWMBL = 255;                 // White backlight brightness (0-255)


int Lguide = 115;           //Total length of linear guide

float pitch = 2.44;              //Pitch of movement screw thread

int stepsMotorPerRev = 200;     //Motor steps per revolution (including gear reduction, if any)

int microsteps = 64;       //Driver microsteps

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
#include "keychain.h"
#include "menus.h"
#include "template12234.h"
#include "templatesDG.h"

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
#include <TFT_eSPI.h>              // Hardware-specific library
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

#define delaybutton 100


//**Preview number of sections**

const int lenpreview = 30;

// **EndStop Lecture**

bool LectEndStop;


// **Layer Height**

int hLayerx1000 = hLayer * 1000;


// **Ascendant movement Height (mm)**

int maxheight = Lguide - 35;


// **Retract movement during printing**

float hDown;

float hDownInitial;


// **Steps per mm**

int StepsPerMm = stepsMotorPerRev * microsteps / pitch;


// **Calibration aditional steps**

const int maxAddDesc = 3;     // Additional maximum descent (Default = 3 mm)

long maxAddSteps = maxAddDesc * StepsPerMm / 80;

long stepsadditional;

int stepsadditionalx80 = 0;


// **Speed and time calculations**

int topdelay = 200;  // Movement delay between lift and retract

int delayLiftSpeed;

int delayLiftSpeedInit;

int delayRetractSpeed;

float updowntime;          //Light-off delay in seconds. Counted from turning off the UV light on one layer and turning on the UV light on the next one.

float updowntimeInit;      //For the initial layers.

long int updowntimeM;

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


// **For the folder name**

File root;
File myfile;

char foldersel[25];

int counter = 1;


// **Progress**

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


// Clean resin vat

int cleanexp = 45;


//Preferences modes: 0-Current, 1-Memory A, 2-Memory B, 3-Memory C, 4-Memory D, 5-Memory E, 6-Memory F

int PrefMode = 0;

//Select preferences from: 0-Menu preferences, 1-Menu print

int SelPrefMode = 0;
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

  displaymenuintro();

  //  delay(1200);

  screen = 0;
  displaymenuprint();

  OLEDintro();


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

    case 141:
      screen = 14;
      screen14B();
      delaybtn();
      break;

    case 142:
      screen = 141;
      screen141();
      delaybtn();
      break;

    case 143:
      screen = 142;
      screen142();
      delaybtn();
      break;

    case 144:
      screen = 143;
      screen143();
      delaybtn();
      break;

    case 145:
      screen = 144;
      screen144();
      delaybtn();
      break;

    case 146:
      screen = 145;
      screen145();
      delaybtn();
      break;

    case 147:
      screen = 146;
      screen146();
      delaybtn();
      break;

    case 2:
      displaymenusettings();
      screen = 1;
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

    case 2010:
      if (cleanexp < 360) {
        cleanexp++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, cleanexp);
      }
      delaybtn();
      break;


    case 202:
      screen = 201;
      screen201B();
      delaybtn();
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


    case 10011:
      switch (hLayerx1000) {
        case 25:
          hLayer = 0.05;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 150, hLayerx1000);
          tft.drawCentreString("um", 270, 150, 4);
          adjuststeps();
          break;

        case 50:
          hLayer = 0.1;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 150, hLayerx1000);
          tft.drawCentreString("um", 270, 150, 4);
          adjuststeps();
          break;

      }
      delaybtn();
      break;


    case 10012:
      if (iexpotime >= 1) {
        iexpotime++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(235, 150, iexpotime);
        tft.drawCentreString("s", 280, 150, 4);
      }
      delaybtn();
      break;


    case 10013:
      if (expotime >= 0.2) {
        expotime = expotime + 0.2;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        if (expotime <= 9.8) {
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawVariableFloat(207, 150, expotime);
        }
        else if (expotime > 99.8) {
          drawVariableFloat3(200, 150, expotime);
        }
        else {
          drawVariableFloat2(200, 150, expotime);
        }

        tft.drawCentreString("s", 280, 150, 4);
      }
      delaybtn();
      break;


    case 10014:
      if (PWMUV < 255) {
        PWMUV++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, PWMUV);
      }
      delaybtn();
      break;


    case 10015:
      if (FirstLayers < 20) {
        FirstLayers++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, FirstLayers);
      }
      delaybtn();
      break;


    case 10016:
      if (TraLayers < 11) {
        TraLayers++;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, TraLayers);
      }
      delaybtn();
      break;


    case 10017:
      if (hUpInitial < 14.9) {
        hUpInitial = hUpInitial + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, hUpInitial);
      }
      delaybtn();
      break;


    case 10018:
      if (hUp < 15) {
        hUp = hUp + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, hUp);
      }
      delaybtn();
      break;


    case 10019:
      if (LiftSpeedInit < 8.1) {
        LiftSpeedInit = LiftSpeedInit + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeedInit);
        recalcTime();
      }
      delaybtn();
      break;


    case 10020:
      if (LiftSpeed < 8.1) {
        LiftSpeed = LiftSpeed + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeed);
        recalcTime();
      }
      delaybtn();
      break;


    case 10021:
      if (RetractSpeed < 8) {
        RetractSpeed = RetractSpeed + 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, RetractSpeed);
        recalcTime();
      }
      delaybtn();
      break;


    case 10022:
      if (RestTime < 30000) {
        RestTime = RestTime + 100;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, RestTime);
      }
      delaybtn();
      break;

    case 100231:
      screen = 10023;
      screen10023B();
      delaybtn();
      break;

    case 100232:
      screen = 100231;
      screen100231();
      delaybtn();
      break;

    case 100233:
      screen = 100232;
      screen100232();
      delaybtn();
      break;

    case 100234:
      screen = 100233;
      screen100233();
      delaybtn();
      break;

    case 100235:
      screen = 100234;
      screen100234();
      delaybtn();
      break;

    case 100236:
      screen = 100235;
      screen100235();
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

    case 1:
      displaymenuutilities();
      screen = 2;
      break;

    case 11:
      folderDown(root);
      delaybtn();
      break;

    case 14:
      screen = 141;
      screen141();
      delaybtn();
      break;

    case 141:
      screen = 142;
      screen142();
      delaybtn();
      break;

    case 142:
      screen = 143;
      screen143();
      delaybtn();
      break;

    case 143:
      screen = 144;
      screen144();
      delaybtn();
      break;

    case 144:
      screen = 145;
      screen145();
      delaybtn();
      break;

    case 145:
      screen = 146;
      screen146();
      delaybtn();
      break;

    case 146:
      screen = 147;
      screen147();
      delaybtn();
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

    case 201:
      screen = 202;
      screen202();
      delaybtn();
      break;


    case 2010:
      if (cleanexp > 1) {
        cleanexp--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, cleanexp);
      }
      delay(80);
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


    case 10011:
      switch (hLayerx1000) {
        case 50:
          hLayer = 0.025;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 150, hLayerx1000);
          tft.drawCentreString("um", 270, 150, 4);
          adjuststeps();
          break;

        case 100:
          hLayer = 0.05;
          hLayerx1000 = hLayer * 1000;
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawCentreVariableInt(220, 150, hLayerx1000);
          tft.drawCentreString("um", 270, 150, 4);
          adjuststeps();
          break;
      }
      delaybtn();
      break;


    case 10012:
      if (iexpotime > 1) {
        iexpotime--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(235, 150, iexpotime);
        tft.drawCentreString("s", 280, 150, 4);
      }
      delaybtn();
      break;


    case 10013:
      if (expotime > 0.4) {
        expotime = expotime - 0.2;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        if (expotime <= 9.8) {
          tft.fillRect(120, 128, 240, 45, TFT_BLACK);
          drawVariableFloat(207, 150, expotime);
        }
        else if (expotime > 99.8) {
          drawVariableFloat3(200, 150, expotime);
        }
        else {
          drawVariableFloat2(200, 150, expotime);
        }
        tft.drawCentreString("s", 280, 150, 4);
      }
      delaybtn();
      break;


    case 10014:
      if (PWMUV > 0) {
        PWMUV--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, PWMUV);
      }
      delaybtn();
      break;


    case 10015:
      if (FirstLayers > 1) {
        FirstLayers--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, FirstLayers);
        recalcTime();
      }
      delaybtn();
      break;


    case 10016:
      if (TraLayers > 0) {
        TraLayers--;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        drawCentreVariableInt(240, 150, TraLayers);
      }
      delaybtn();
      break;


    case 10017:
      if (hUpInitial > 0.6) {
        hUpInitial = hUpInitial - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, hUpInitial);
        recalcTime();
      }
      delaybtn();
      break;


    case 10018:
      if (hUp > 0.6) {
        hUp = hUp - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, hUp);
        recalcTime();
      }
      delaybtn();
      break;


    case 10019:
      if (LiftSpeedInit > 0.1) {
        LiftSpeedInit = LiftSpeedInit - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeedInit);
        recalcTime();
      }
      delaybtn();
      break;


    case 10020:
      if (LiftSpeed > 0.1) {
        LiftSpeed = LiftSpeed - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, LiftSpeed);
        recalcTime();
      }
      delaybtn();
      break;


    case 10021:
      if (RetractSpeed > 0.2) {
        RetractSpeed = RetractSpeed - 0.1;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableFloat(240, 150, RetractSpeed);
        recalcTime();
      }
      delaybtn();
      break;


    case 10022:
      if (RestTime >= 100) {
        RestTime = RestTime - 100;
        tft.fillRect(120, 128, 240, 45, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        drawCentreVariableInt(240, 150, RestTime);
        recalcTime();
      }
      delaybtn();
      break;

    case 10023:
      screen = 100231;
      screen100231();
      delaybtn();
      break;

    case 100231:
      screen = 100232;
      screen100232();
      delaybtn();
      break;

    case 100232:
      screen = 100233;
      screen100233();
      delaybtn();
      break;

    case 100233:
      screen = 100234;
      screen100234();
      delaybtn();
      break;

    case 100234:
      screen = 100235;
      screen100235();
      delaybtn();
      break;

    case 100235:
      screen = 100236;
      screen100236();
      delaybtn();
      break;

  }
}


void btnNEXTmenu() {

  switch (screen) {

    case 0:
      OLEDblack();
      screen31();
      screen = 31;
      break;

    case 1:
      OLEDblack();
      SelPrefMode = 0;
      screen = 10011;
      screen10011();
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
        delay(800);
      }
      screen = 14;
      screen14();
      break;

    case 14:
      PrefMode = 0;
      ReadPref();
      screen = 15;
      screen15();
      break;

    case 141:
      SelPrefMode = 1;
      screen = 10011;
      screen10011();
      break;

    case 142:
      PrefMode = 1;
      ReadPref();
      screen = 15;
      screen15();
      break;

    case 143:
      PrefMode = 2;
      ReadPref();
      screen = 15;
      screen15();
      break;

    case 144:
      PrefMode = 3;
      ReadPref();
      screen = 15;
      screen15();
      break;

    case 145:
      PrefMode = 4;
      ReadPref();
      screen = 15;
      screen15();
      break;

    case 146:
      PrefMode = 5;
      ReadPref();
      screen = 15;
      screen15();
      break;

    case 147:
      PrefMode = 6;
      ReadPref();
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
      if (PrintMode != 0) {
        FirstLayers = 5;
        TraLayers = 0;
      }

      recalcTime();
      screen = 151;
      screen151();
      break;


    case 151:
      screen = 152;
      screen152();
      break;


    case 152:
      screen = 16;
      screen16();
      break;


    case 2:
      OLEDblack();
      screen = 201;
      screen201();
      break;


    case 21:
      WritePref();
      //OLEDwait();
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

    case 201:
      screen2010();
      screen = 2010;
      break;

    case 2010:
      screen20101();
      screen = 20101;
      break;

    case 202:
      screen2020();
      screen = 2020;
      break;

    case 31:
      screen310();
      screen = 310;
      break;

    case 310:
      TFT_BLACKscreen();
      //OLEDwait();
      screen21();
      calibrate();
      //OLEDcalibrate();
      screen = 21;
      screen22();
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
      screen14();
      screen = 14;
      PrintMode = 1;
      break;

    case 52:
      screen14();
      screen = 14;
      PrintMode = 2;
      break;


    case 10011:
      screen = 10012;
      screen10012();
      break;


    case 10012:
      screen = 10013;
      screen10013();
      break;


    case 10013:
      screen = 10014;
      screen10014();
      break;


    case 10014:
      screen = 10015;
      screen10015();
      break;


    case 10015:
      screen = 10016;
      screen10016();
      break;


    case 10016:
      screen = 10017;
      screen10017();
      break;


    case 10017:
      screen = 10018;
      screen10018();
      break;


    case 10018:
      screen = 10019;
      screen10019();
      break;


    case 10019:
      screen = 10020;
      screen10020();
      break;


    case 10020:
      screen = 10021;
      screen10021();
      break;


    case 10021:
      screen = 10022;
      screen10022();
      break;

    case 10022:
      screen = 10023;
      screen10023();
      break;

    case 10023:
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 0;
          displaymenuprint();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;

    case 100231:
      PrefMode = 1;
      WritePref();
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 0;
          displaymenuprint();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;

    case 100232:
      PrefMode = 2;
      WritePref();
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 0;
          displaymenuprint();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;

    case 100233:
      PrefMode = 3;
      WritePref();
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 0;
          displaymenuprint();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;

    case 100234:
      PrefMode = 4;
      WritePref();
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 0;
          displaymenuprint();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;

    case 100235:
      PrefMode = 5;
      WritePref();
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 0;
          displaymenuprint();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;


    case 100236:
      PrefMode = 6;
      WritePref();
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 0;
          displaymenuprint();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;

  }
  delaybtn();
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

    case 2:
      screen = 1;
      displaymenusettings();
      break;

    case 201:
      screen = 2;
      displaymenuutilities();
      break;


    case 2010:
      screen201();
      screen = 201;
      break;

    case 20101:
      screen2010();
      screen = 2010;
      break;

    case 202:
      screen = 2;
      displaymenuutilities();
      break;


    case 2020:
      screen = 201;
      screen201();
      break;


    case 31:
      screen = 0;
      displaymenuprint();
      break;

    case 32:
      screen = 0;
      displaymenuprint();
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

    case 14: case 141: case 142: case 143: case 144: case 145: case 146: case 147:

      if (PrintMode != 0) {
        screen = 41;
        screen41();
      }
      else {
        screen = 11;
        screen11();
      }

      break;

    case 15:
      screen = 14;
      screen14();
      break;

    case 151:
      screen = 15;
      screen15();
      break;


    case 152:
      screen = 151;
      screen151();
      break;

    case 16:
      screen = 152;
      screen152();
      break;

    case 17:
      screen = 0;
      OLEDblack();
      ledcWrite(ledChannel1, PWMBL);
      displaymenuprint();
      break;

    case 22:
      screen = 31;
      screen31();
      break;


    case 10011:
      PrefMode = 0;
      WritePref();
      switch (SelPrefMode) {
        case 0:
          screen = 1;
          displaymenusettings();
          break;
        case 1:
          screen = 15;
          screen15();
          break;
      }
      break;


    case 10012:
      screen = 10011;
      screen10011();
      break;


    case 10013:
      screen = 10012;
      screen10012();
      break;


    case 10014:
      screen = 10013;
      screen10013();
      break;


    case 10015:
      screen = 10014;
      screen10014();
      break;


    case 10016:
      screen = 10015;
      screen10015();
      break;


    case 10017:
      screen = 10016;
      screen10016();
      break;


    case 10018:
      screen = 10017;
      screen10017();
      break;


    case 10019:
      screen = 10018;
      screen10018();
      break;


    case 10020:
      screen = 10019;
      screen10019();
      break;


    case 10021:
      screen = 10020;
      screen10020();
      break;


    case 10022:
      screen = 10021;
      screen10021();
      break;


    case 10023: case 100231: case 100232: case 100233: case 100234: case 100235: case 100236:
      screen = 10022;
      screen10022();
      break;

    case 320:
      screen = 31;
      screen31();
      break;

  }
  delaybtn();
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

    case 20101:
      cleanresinvat();
      break;


    case 2020:
      Testmode();
      break;

  }
}

//***** MAIN FUNCTIONS *****

//Read-Write preferences

void ReadPref() {

  switch (PrefMode) {

    case 0:
      preferences.begin("Settings", false);
      break;

    case 1:
      preferences.begin("MemoryA", false);
      break;

    case 2:
      preferences.begin("MemoryB", false);
      break;

    case 3:
      preferences.begin("MemoryC", false);
      break;

    case 4:
      preferences.begin("MemoryD", false);
      break;

    case 5:
      preferences.begin("MemoryE", false);
      break;

    case 6:
      preferences.begin("MemoryF", false);
      break;
  }

  hLayer = preferences.getFloat("hLayer", 0.1);
  hUp = preferences.getFloat("hUp", 4.0);
  hUpInitial = preferences.getFloat("hUpInitial", 6.0);
  FirstLayers = preferences.getInt("FirstLayers", 4);
  TraLayers = preferences.getInt("TraLayers", 0);
  expotime = preferences.getFloat("expotime", 6.0);
  iexpotime = preferences.getInt("iexpotime", 40);
  LiftSpeed = preferences.getFloat("LiftSpeed", 1.7);
  LiftSpeedInit = preferences.getFloat("LiftSpeedInit", 0.7);
  RetractSpeed = preferences.getFloat("RetractSpeed", 2.9);
  RestTime = preferences.getInt("RestTime", 0);
  stepsadditional = preferences.getLong("stepsadditional", 0);
  PWMUV = preferences.getInt("PWMUV", 255);
  preferences.end();

  hLayerx1000 = hLayer * 1000;
  recalcTime();
}


void WritePref() {

  switch (PrefMode) {
    case 0:
      preferences.begin("Settings", false);
      break;

    case 1:
      preferences.begin("MemoryA", false);
      break;

    case 2:
      preferences.begin("MemoryB", false);
      break;

    case 3:
      preferences.begin("MemoryC", false);
      break;

    case 4:
      preferences.begin("MemoryD", false);
      break;

    case 5:
      preferences.begin("MemoryE", false);
      break;

    case 6:
      preferences.begin("MemoryF", false);
      break;
  }

  preferences.putFloat("hLayer", hLayer);
  preferences.putFloat("hUp", hUp);
  preferences.putFloat("hUpInitial", hUpInitial);
  preferences.putInt("FirstLayers", FirstLayers);
  preferences.putInt("TraLayers", TraLayers);
  preferences.putFloat("expotime", expotime);
  preferences.putInt("iexpotime", iexpotime);
  preferences.putFloat("LiftSpeed", LiftSpeed);
  preferences.putFloat("LiftSpeedInit", LiftSpeedInit);
  preferences.putFloat("RetractSpeed", RetractSpeed);
  preferences.putInt("RestTime", RestTime);
  preferences.putLong("stepsadditional", stepsadditional);
  preferences.putInt("PWMUV", PWMUV);
  preferences.end();
}


//Test mode function

void Testmode() {

  TFT_BLACKscreen();
  tft.setTextColor(TFT_WHITE);
  tft.fillCircle(240, 140, 30, TFT_WHITE);
  tft.drawCentreString("UV LIGHT ON?", 240, 25, 4);
  ledcWrite(ledChannel, 255);
  delay(1500);
  ledcWrite(ledChannel, 0);
  tft.drawCentreString("PRESS PLAY", 240, 280, 4);
  edoBtnPLAY = digitalRead (BtnPlay);
  while (edoBtnPLAY != LOW) {
    edoBtnPLAY = digitalRead (BtnPlay);
  }

  TFT_BLACKscreen();
  tft.drawCentreString("STEPPER MOTOR TEST", 240, 25, 4);
  LectEndStop = digitalRead(PinEndStop);
  if (LectEndStop != HIGH) {
    desctoendstop();
    delay(500);
  }
  movasc(50, delayRetractSpeed);
  digitalWrite (PinEn, HIGH);
  TFT_BLACKscreen();
  tft.drawCentreString("STEPPER MOTOR MOVE?", 240, 25, 4);

  tft.drawCentreString("PRESS PLAY", 240, 280, 4);
  edoBtnPLAY = digitalRead (BtnPlay);
  while (edoBtnPLAY != LOW) {
    edoBtnPLAY = digitalRead (BtnPlay);
  }

  TFT_BLACKscreen();
  tft.drawCentreString("SD CARD TEST", 240, 25, 4);
  delay(500);
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
  tft.setTextColor(TFT_YELLOW);
  tft.setRotation(3);
  coordRot3();
  tft.drawCentreString("SD IMAGE DISPLAYED?", 240, 25, 4);

  tft.drawCentreString("PRESS PLAY", 240, 280, 4);
  edoBtnPLAY = digitalRead (BtnPlay);
  while (edoBtnPLAY != LOW) {
    edoBtnPLAY = digitalRead (BtnPlay);
  }
  tft.setTextColor(TFT_BLACK);
  tft.fillScreen(TFT_GREEN);
  tft.drawCentreString("TEST FINISHED!", 240, 140, 4);
  delay(2000);
  screen = 0;
  displaymenuprint();
}


void cleanresinvat() {

  tft.fillScreen(TFT_WHITE);
  ledcWrite(ledChannel, PWMUV);
  delay(cleanexp * 1000);
  ledcWrite(ledChannel, 0);
  delay(1000);
  screen201();
  screen = 201;
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

  started();
  AdvLay = 0;
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
    printprogress();
    movasc(hUpInitial, delayLiftSpeedInit);
    delay(topdelay);
    movdesc(hDownInitial, delayRetractSpeed);
    OLEDprint();
  }

  // Printing transition layers

  for (int l = 1; l <= TraLayers ; l++) {
    delay(RestTime);
    printname();
    ledcWrite(ledChannel, PWMUV);
    float TransitionLapse = (iexpotime - expotime) / (TraLayers + 1);
    float TransitionExpo = iexpotime - (TransitionLapse * l);
    delay(TransitionExpo * 1000);
    ledcWrite(ledChannel, 0);
    TFT_BLACKscreen();
    pausing();                         //allows to enter the pausing function by holding down the Play/Pause button
    printprogress();
    movasc(hUpInitial, delayLiftSpeedInit);
    delay(topdelay);
    movdesc(hDownInitial, delayRetractSpeed);
    OLEDprint();
  }

  // Printing the rest

  for (int l = 0; l < Layers - FirstLayers - TraLayers; l++) {

    delay(RestTime);
    printname();
    ledcWrite(ledChannel, PWMUV);
    delay(expotime * 1000);
    ledcWrite(ledChannel, 0);
    TFT_BLACKscreen();
    pausing();                         //allows to enter the pausing function by holding down the Play/Pause button
    updowntimeM = - millis();
    printprogress();
    movasc(hUp, delayLiftSpeed);
    delay(topdelay);
    movdesc(hDown, delayRetractSpeed);
    OLEDprint();
    updowntimeM = updowntimeM + millis();
    updowntime = updowntimeM / 1000;
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
  tft.drawCentreString(foldersel, 245, 150, 4);
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
    tft.drawCentreString(foldersel, 245, 150, 4);
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


//    tft.setRotation(3);
//    coordRot3();
//    displaytemplate3();
//    maskRight();
//    maskLeft();
//    tft.drawCentreString("PAUSE", 240, 25, 4);
//    tft.drawCentreString("PRESS PLAY/PAUSE", 240, 150, 4);
//    tft.drawCentreString("TO RESUME", 240, 180, 4);

    float heightActual = number * 0.025;
    float heightAdd = maxheight - heightActual;
    movasc(heightAdd, delayLiftSpeed);

    while (digitalRead (BtnPlay) != LOW) {
    }
    printprogress();
    movdesc(heightAdd, delayRetractSpeed);
    delaybtn();
  }

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

//////////////////////////////
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

//void OLEDwait() {
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setCursor(5, 50);
//  display.print("Please");
//  display.setCursor(17, 80);
//  display.print("wait");
//  display.display();
//}

//void OLEDcalibrate() {
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setCursor(5, 30);
//  display.print("1.Lock");
//  display.setCursor(0, 55);
//  display.print("platform");
//  display.setCursor(0, 95);
//  display.print("2.Press");
//  display.setCursor(16, 115);
//  display.print("next");
//  display.display();
//}

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

//void OLEDlevel() {
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setCursor(8, 20);
//  display.print("SKIP:");
//  display.setCursor(10, 40);
//  display.print("Level");
//  display.setCursor(18, 60);
//  display.print("and");
//  display.setCursor(5, 80);
//  display.print("locked");
//  display.setCursor(0, 100);
//  display.print("platform");
//  display.setCursor(15, 120);
//  display.print("only");
//  display.display();
//}

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
  tft.fillScreen(TFT_BLACK);
  int16_t rc2 = png.openFLASH((uint8_t *)menuintro, sizeof(menuintro), PNGDraw);
  if (rc2 == PNG_SUCCESS) {
    tft.startWrite();
    rc2 = png.decode(NULL, 0);
    tft.endWrite();
  }
  tft.drawCentreString("V1.0", 240, 270, 4);

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


void displaymenuutilities() {
  int16_t rc40 = png.openFLASH((uint8_t *)menuutilities, sizeof(menuutilities), PNGDraw);
  if (rc40 == PNG_SUCCESS) {
    tft.startWrite();
    rc40 = png.decode(NULL, 0);
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


void displaytemplate4() {
  int16_t rc8 = png.openFLASH((uint8_t *)template4, sizeof(template4), PNGDraw);
  if (rc8 == PNG_SUCCESS) {
    tft.startWrite();
    rc8 = png.decode(NULL, 0);
    tft.endWrite();
  }
}


void screen11() {

  ReadPref();
  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("SELECT FILE", 240, 25, 4);
  tft.drawCentreString(foldersel, 245, 150, 4);
}


void screen14() {

  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("PARAMETERS", 240, 25, 4);
  tft.drawCentreString("Current parameters", 245, 150, 4);
}


void screen14B() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Current parameters", 245, 150, 4);
}



void screen141() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Change parameters", 245, 150, 4);
}



void screen142() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Memory A", 245, 150, 4);
}


void screen143() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Memory B", 245, 150, 4);
}


void screen144() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Memory C", 245, 150, 4);
}


void screen145() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Memory D", 245, 150, 4);
}


void screen146() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Memory E", 245, 150, 4);
}

void screen147() {
  tft.fillRect(120, 128, 240, 50, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Memory F", 245, 150, 4);
}


void screen15() {

  displaytemplate4();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("CONFIRMATION 1", 240, 25, 4);

  hLayerx1000 = hLayer * 1000;
  tft.drawString("LAYER HEIGHT (um)", 70, 100, 4);
  drawVariableInt(343, 100, hLayerx1000);

  tft.drawString("INITIAL EXPOS. (s)", 70, 154, 4);
  drawVariableInt(343, 154, iexpotime);

  tft.drawString("EXPOSURE TIME (s)", 70, 208, 4);
  if (expotime < 10) {
    drawVariableFloat(343, 208, expotime);
  }
  if (expotime >= 100) {
    drawVariableFloat3(343, 208, expotime);
  }
  else {
    drawVariableFloat2(343, 208, expotime);
  }

  tft.drawString("UV POWER", 70, 262, 4);
  drawVariableInt(343, 262, PWMUV);

}

void screen151() {

  displaytemplate4();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("CONFIRMATION 2", 240, 25, 4);

  tft.drawString("BOTTOM LAYERS", 70, 100, 4);
  drawVariableInt(343, 100, FirstLayers);
  if (PrintMode != 0) {
    tft.drawString("*", 370, 100, 4);
  }

  tft.drawString("TRANS. LAYERS", 70, 154, 4);
  drawVariableInt(343, 154, TraLayers);
  if (PrintMode != 0) {
    tft.drawString("*", 370, 154, 4);
  }

  tft.drawString("B. LIFT HEIGHT(mm)", 70, 208, 4);
  drawVariableFloat(343, 208, hUpInitial);

  tft.drawString("LIFT HEIGHT (mm)", 70, 262, 4);
  drawVariableFloat(343, 262, hUp);
}


void screen152() {

  displaytemplate4();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("CONFIRMATION 3", 240, 25, 4);

  tft.drawString("LIFT SPEED (mm/s)", 70, 100, 4);
  if (LiftSpeed < 10) {
    drawVariableFloat(343, 100, LiftSpeed);
  }
  if (expotime >= 100) {
    drawVariableFloat3(343, 100, LiftSpeed);
  }
  else {
    drawVariableFloat2(343, 100, LiftSpeed);
  }

  tft.drawString("RETR. SPEED (mm/s)", 70, 154, 4);
  if (RetractSpeed < 10) {
    drawVariableFloat(343, 154, RetractSpeed);
  }
  if (expotime >= 100) {
    drawVariableFloat3(343, 154, RetractSpeed);
  }
  else {
    drawVariableFloat2(343, 154, RetractSpeed);
  }


  tft.drawString("B. LIFT SPEED (mm/s)", 70, 208, 4);
  if (LiftSpeedInit < 10) {
    drawVariableFloat(343, 208, LiftSpeedInit);
  }
  if (expotime >= 100) {
    drawVariableFloat3(343, 208, LiftSpeedInit);
  }
  else {
    drawVariableFloat2(343, 208, LiftSpeedInit);
  }

  tft.drawString("REST TIME (s)", 70, 262, 4);
  drawVariableInt(343, 262, RestTime);
}


void screen16() {

  displaytemplate3();
  maskRight();
  tft.setRotation(3);
  coordRot3();
  tft.setTextColor(TFT_WHITE);

  tft.drawCentreString("DETAILS", 240, 25, 4);

  tft.drawString("FOLDER:", 88, 110, 4);

  char shortfolder[11];
  for (int c = 0; c < 9 ; c++) {
    shortfolder[c] = foldersel[c];
  }
  shortfolder[9] = '.';
  shortfolder[10] = '.';

  switch (PrintMode) {

    case 0:
      tft.drawString(shortfolder, 215, 110, 4);
      break;

    case 1:
      tft.drawString("Keychain", 230, 110, 4);
      break;

    case 2:
      tft.drawString("OLED Cover", 230, 110, 4);
      break;

  }



  tft.drawString("TOTAL LAYERS:", 90, 155, 4);

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
          Layers = 2480;
          break;

        case 50:
          Layers = 1240;
          break;

        case 100:
          Layers = 620;
          break;
      }
      break;
  }

  drawVariableInt(310, 155, Layers);

  recalcTime();
  long int timelayersinitial = FirstLayers * iexpotime;
  long int quantitylayersresto = Layers - FirstLayers;
  long int timerestodelayers = quantitylayersresto * expotime;
  long int timesubebajatot = Layers * updowntime;
  long int timetotalseg = timesubebajatot + timelayersinitial + timerestodelayers;
  long int timetotalmin = timetotalseg / 60;
  int timetotalhours = timetotalmin / 60;
  int restominutes = timetotalmin - timetotalhours * 60;

  tft.drawString("EST. TIME:", 90, 200, 4);
  if (timetotalhours < 10) {
    drawVariableInt(257, 200, timetotalhours);
    tft.drawString("h", 272, 200, 4);
  }
  else {
    drawVariableInt(242, 200, timetotalhours);
    tft.drawString("h", 272, 200, 4);
  }

  if (restominutes < 10) {
    drawVariableInt(320, 200, restominutes);
    tft.drawString("min", 335, 200, 4);
  }
  else {
    drawVariableInt(305, 200, restominutes);
    tft.drawString("min", 335, 200, 4);
  }
  tft.drawCentreString("PRESS START!", 240, 260, 4);
}


void screen17() {

  tft.setRotation(3);
  coordRot3();
  TFT_BLACKscreen();

  int heightup = Layers * hLayer;
  int heightremain = maxheight - heightup;

  movasc(heightremain, delayLiftSpeed);

  displayfinished();

  int timetotalmin = compTimeMillis / 60000;
  int timetotalhours = timetotalmin / 60;
  int restominutes = timetotalmin - timetotalhours * 60;

  tft.drawCentreString("FINISHED IN:", 240, 220, 4);

  if (timetotalhours < 10) {
    drawVariableInt(187, 270, timetotalhours);
    tft.drawString("h", 202, 270, 4);
  }
  else {
    drawVariableInt(172, 270, timetotalhours);
    tft.drawString("h", 202, 270, 4);
  }

  if (restominutes < 10) {
    drawVariableInt(250, 270, restominutes);
    tft.drawString("min", 265, 270, 4);
  }
  else {
    drawVariableInt(235, 270, restominutes);
    tft.drawString("min", 265, 270, 4);
  }
}


void screen21() {

  displaytemplate3();
  maskLeft();
  maskRight();
  tft.drawCentreString("LEVELING...", 240, 25, 4);
  tft.drawCentreString("PLEASE WAIT", 240, 160, 4);
}


void screen22() {

  displaylevel();
}

void started() {

  displaytemplate3();
  maskLeft();
  maskRight();
  tft.drawCentreString("3D PRINT", 240, 25, 4);
  tft.drawCentreString("STARTED!", 240, 160, 4);
}


void printprogress() {

  tft.setRotation(3);
  coordRot3();

  AdvLay++;

  long int timelayersinitial = FirstLayers * iexpotime;
  long int quantitylayersresto = Layers - FirstLayers;
  long int timerestodelayers = quantitylayersresto * expotime;
  long int timesubebajatot = Layers * (updowntime + RestTime / 1000);

  int AdvLayNI = AdvLay - FirstLayers - 1;
  long int timetotalseg;
  int PromTra = (iexpotime + expotime) / 2;

  if (AdvLay <= FirstLayers) {
    timetotalseg = timesubebajatot + timelayersinitial + timerestodelayers - AdvLay * iexpotime - AdvLay * updowntime;
  }
  else if (AdvLay < (FirstLayers + TraLayers)) {
    timetotalseg = timesubebajatot + timelayersinitial + timerestodelayers - FirstLayers * iexpotime - AdvLay * PromTra - AdvLay * updowntime;
  }
  else {
    timetotalseg = timesubebajatot + timelayersinitial + timerestodelayers - FirstLayers * iexpotime - TraLayers * PromTra - AdvLayNI * expotime - AdvLayNI * updowntime;
  }

  int timetotalmin = timetotalseg / 60;
  int timetotalhours = timetotalmin / 60;
  int restominutes = timetotalmin - timetotalhours * 60;

  int percentAdv = 100 * AdvLay / Layers;
  alfaDegTot = AdvLay * 360 / Layers;

  displayprogress();

  //tft.drawArc(x, y, r, r - thickness, val_angle, last_angle, TFT_BLACK, DARKER_GREY);

  if (alfaDegTot <= 180) {
    tft.drawArc(244, 160, 120, 100, 180, 180 + alfaDegTot, TFT_WHITE, TFT_WHITE);
  }
  else if (alfaDegTot > 180) {
    tft.drawArc(244, 160, 120, 100, 180, alfaDegTot - 180, TFT_WHITE, TFT_WHITE);
  }

  if (percentAdv <= 99) {
    drawCentreVariableInt6(244, 85, percentAdv);
    tft.setTextColor(TFT_YELLOW);
    tft.drawCentreString("%", 244, 130, 4);
    tft.setTextColor(TFT_WHITE);
  }

  tft.drawCentreString("TO FINISH:", 246, 175, 4);

  if (timetotalhours < 10) {
    drawVariableInt(187, 205, timetotalhours);
    tft.drawString("h", 202, 205, 4);
  }
  else {
    drawVariableInt(172, 205, timetotalhours);
    tft.drawString("h", 202, 205, 4);
  }
  if (restominutes < 10) {
    drawVariableInt(250, 205, restominutes);
    tft.drawString("min", 265, 205, 4);
  }
  else {
    drawVariableInt(235, 205, restominutes);
    tft.drawString("min", 265, 205, 4);
  }
}


void screen201() {

  tft.setRotation(3);
  coordRot3();
  TFT_BLACKscreen();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("TOOLS", 240, 25, 4);
  tft.drawCentreString("CLEAN RESIN VAT", 240, 117, 4);
  tft.drawCentreString("FACTORY TEST", 240, 232, 4);
  displaytemplate2A();
}


void screen201B() {
  displaytemplate2A();
}


void screen202() {

  displaytemplate2B();
}


void screen2010() {

  displaytemplate1();
  tft.drawCentreString("EXPOSURE TIME", 240, 25, 4);
  drawCentreVariableInt(240, 150, cleanexp);
}


void screen20101() {

  displaytemplate3();
  maskRight();
  tft.drawCentreString("CLEAN RESIN VAT", 240, 25, 4);
  tft.drawString("EXPOSURE TIME (s):", 90, 125, 4);
  drawVariableInt(340, 125, cleanexp);
  tft.drawCentreString("PRESS START!", 240, 200, 4);
}


void screen2020() {

  displaytemplate3();
  maskRight();
  tft.drawCentreString("FACTORY TEST", 240, 25, 4);
  tft.drawCentreString("REMOVE TRAY", 240, 110, 4);
  tft.drawCentreString("AND PLATFORM", 240, 150, 4);
  tft.drawCentreString("PRESS START!", 240, 230, 4);
}


void screen31() {

  //OLEDlevel();
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

////////////////////////////////////////


void screen10011() {

  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("1.LAYER HEIGHT", 240, 25, 4);
  drawCentreVariableInt(220, 150, hLayerx1000);
  tft.drawCentreString("um", 270, 150, 4);
}


void screen10012() {

  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("2.INITIAL EXPOSURE", 240, 25, 4);
  drawCentreVariableInt(235, 150, iexpotime);
  tft.drawCentreString("s", 280, 150, 4);
}


void screen10013() {

  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("3.EXPOSURE TIME", 240, 25, 4);
  if (expotime <= 9.8) {
    tft.fillRect(120, 128, 240, 45, TFT_BLACK);
    drawVariableFloat(207, 150, expotime);
  }
  else if (expotime >= 100) {
    drawVariableFloat3(200, 150, expotime);
  }
  else {
    drawVariableFloat2(200, 150, expotime);
  }
  tft.drawCentreString("s", 280, 150, 4);
}


void screen10014() {

  displaytemplate1();
  tft.drawCentreString("4.UV POWER", 240, 25, 4);
  drawCentreVariableInt(240, 150, PWMUV);
  tft.drawCentreString("(0-255)", 240, 212, 4);
}


void screen10015() {

  displaytemplate1();
  tft.drawCentreString("5.BOTTOM LAYER COUNT", 240, 25, 4);
  drawCentreVariableInt(240, 150, FirstLayers);
}


void screen10016() {

  displaytemplate1();
  tft.drawCentreString("6.TRANSITION LAYERS", 240, 25, 4);
  drawCentreVariableInt(240, 150, TraLayers);
}


void screen10017() {

  displaytemplate1();
  tft.drawCentreString("7.BOTTOM LIFT HEIGHT", 240, 25, 4);
  drawCentreVariableFloat(240, 150, hUpInitial);
  tft.drawCentreString("(mm)", 240, 212, 4);
}


void screen10018() {

  displaytemplate1();
  tft.drawCentreString("8.LIFT HEIGHT", 240, 25, 4);
  drawCentreVariableFloat(240, 150, hUp);
  tft.drawCentreString("(mm)", 240, 212, 4);
}


void screen10019() {

  displaytemplate1();
  tft.drawCentreString("9.BOTTOM LIFT SPEED", 240, 25, 4);
  drawCentreVariableFloat(240, 150, LiftSpeedInit);
  tft.drawCentreString("(mm/s)", 240, 212, 4);
}


void screen10020() {

  displaytemplate1();
  tft.drawCentreString("10.LIFT SPEED", 240, 25, 4);
  drawCentreVariableFloat(240, 150, LiftSpeed);
  tft.drawCentreString("(mm/s)", 240, 212, 4);
}


void screen10021() {

  displaytemplate1();
  tft.drawCentreString("11.RETRACT SPEED", 240, 25, 4);
  drawCentreVariableFloat(240, 150, RetractSpeed);
  tft.drawCentreString("(mm/s)", 240, 212, 4);
}


void screen10022() {

  displaytemplate1();
  tft.drawCentreString("12.REST TIME", 240, 25, 4);
  drawCentreVariableInt(240, 150, RestTime);
  tft.drawCentreString("(ms)", 240, 212, 4);
}


void screen10023() {

  displaytemplate1();
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("13.STORE PARAMETERS", 240, 25, 4);
  tft.drawCentreString("DO NOT STORE", 240, 150, 4);
}


void screen10023B() {

  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("DO NOT STORE", 240, 150, 4);
}

void screen100231() {

  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("MEMORY A", 240, 150, 4);
}


void screen100232() {

  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("MEMORY B", 240, 150, 4);
}


void screen100233() {

  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("MEMORY C", 240, 150, 4);
}


void screen100234() {

  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("MEMORY D", 240, 150, 4);
}


void screen100235() {

  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("MEMORY E", 240, 150, 4);
}


void screen100236() {

  tft.fillRect(120, 128, 240, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("MEMORY F", 240, 150, 4);
}


void displayfinished() {
  int16_t rc5 = png.openFLASH((uint8_t *)finished, sizeof(finished), PNGDraw);
  if (rc5 == PNG_SUCCESS) {
    tft.startWrite();
    rc5 = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void displayprogress() {
  int16_t rc5 = png.openFLASH((uint8_t *)progress, sizeof(progress), PNGDraw);
  if (rc5 == PNG_SUCCESS) {
    tft.startWrite();
    rc5 = png.decode(NULL, 0);
    tft.endWrite();
  }
}


void displaylevel() {
  int16_t rc5 = png.openFLASH((uint8_t *)level, sizeof(level), PNGDraw);
  if (rc5 == PNG_SUCCESS) {
    tft.startWrite();
    rc5 = png.decode(NULL, 0);
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
    edoBtnPLAY = digitalRead (BtnPlay);
    if (edoBtnPLAY == LOW) {
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

void drawCentreVariableInt6(int xPos, int yPos, int vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[7];
  vartoprint.toCharArray(vartodrawChar, 7);
  tft.drawCentreString(vartodrawChar, xPos, yPos, 6);
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

void drawVariableFloat4(int xPos, int yPos, float vartodraw) {

  String vartoprint = String(vartodraw);
  char vartodrawChar[7];
  vartoprint.toCharArray(vartodrawChar, 7);
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

////////////////////////////////////////


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
