#include <SwitecX12.h>
#include <OneBitDisplay.h>

#define MAXRPM 7300
#define MINRPM 500
#define MINRPMPOSITION 3270

OBDISP obd;
#define SDA_PIN 43
#define SCL_PIN 44
// no reset pin needed
#define RESET_PIN -1
// let OneBitDisplay find the address of our display
#define OLED_ADDR -1
#define FLIP180 0
#define INVERT 0
// Use the default Wire library
#define USE_HW_I2C 0

const int STEPS = 315 * 12;
const int A_STEP = 8;
const int A_DIR = 9;
const int RESETPIN = 10;
//HardwareSerial Serial3(PB_11, PB_10);

SwitecX12 rpmmotor(STEPS, A_STEP, A_DIR);
#define RX3_PIN 14
#define TX3_PIN 15
static uint32_t oldtime = millis();
static uint32_t lastScreenUpdate = millis();
uint8_t speedyResponse[100]; //The data buffer for the Serial data. This is longer than needed, just in case
uint8_t byteNumber[2];  // pointer to which uint8_t number we are reading currently
uint8_t readclt; // clt doesn't need to be updated very ofter so
int clt;   // to store coolant temp
unsigned int rpm;  //rpm and PW from speeduino
unsigned int rpmBefore;
int iat;
float afr;
float afrConv;
uint8_t cmdAdata[40] ; 
uint8_t test;
 
#define BYTES_TO_READ 74
#define SERIAL_TIMEOUT 300
float rps;
boolean sent = false;
boolean received = false;
uint32_t sendTimestamp;

long unsigned debug = millis();


void setup() {
  obdI2CInit(&obd, OLED_128x32, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L);
  obdFill(&obd, 0, 0xff);
  obdWriteString(&obd, 0, 0, 0,(char *)"RPM 0000", FONT_SMALL, 0, 1);
  obdWriteString(&obd, 0, 56, 0,(char *)"IAT 0c ", FONT_SMALL, 0, 1);
  obdWriteString(&obd, 0, 56, 1,(char *)"AFR 14.7", FONT_SMALL, 0, 1);
  obdWriteString(&obd, 0, 0, 1,(char *)"Aqua 0c", FONT_SMALL, 0, 1);
  obdWriteString(&obd, 0, 56, 2,(char *)"TPS 0%", FONT_SMALL, 0, 1);
  obdWriteString(&obd, 0, 0, 2,(char *)"Adv 0", FONT_SMALL, 0, 1);
  obdWriteString(&obd, 0, 0, 3,(char *)"Olio 0c, 0 Bar", FONT_SMALL, 0, 1);


  digitalWrite(RESETPIN, HIGH);
  Serial.begin(9600);
  rpmmotor.zero();
  rpmmotor.setPosition(MINRPMPOSITION);
  rpmmotor.update();
  Serial3.begin(115200);
}

void loop() {
    while (!rpmmotor.stopped) rpmmotor.update();
    requestData();
  if(received) {
    received = false;
  }

if (isRPMDeviationBiggerThan(20))
  {
    rpmmotor.setPosition(calcposition(rpm));
   if (screenupdateDue()) {
    updaterpm(rpm); 
   }
    rpmBefore = rpm;
    }
  
  if (millis() > debug) {
    debug = millis() + (long unsigned)1000;
    Serial.print("RPM-"); Serial.print(rpm); Serial.print("\n");
   }
    

}


void requestData() {
  if(sent && Serial3.available() > 0) {
    if(Serial3.read() == 'A') {
      uint8_t bytesRead = Serial3.readBytes(speedyResponse, BYTES_TO_READ);
      if(bytesRead != BYTES_TO_READ) {
        processData();
        for(uint8_t i = 0; i < bytesRead; i++) {
        }
        received = true;
        clearRX();
      } else {
        processData();
        received = true;
        rps = 1000.0/(millis() - sendTimestamp);
      }
      sent = false;
    } else Serial3.read();
  } else if(!sent) {
    Serial3.write('A');
    sent = true;
    sendTimestamp = millis();
  } else if(sent && millis() - sendTimestamp > SERIAL_TIMEOUT) {
    sent = false;
  }
}

int calcposition(int currentRPM) {
   if (currentRPM < MINRPM) return MINRPMPOSITION;
   else if (currentRPM > MAXRPM) return 1;
   else  { 
    int rpmPosition = MINRPMPOSITION - ( rpm - 500 ) * 0.44;
    return rpmPosition; }
  }

void processData() {
    rpm = ((speedyResponse [15] << 8) | (speedyResponse [14]));
    //clt = (speedyResponse [15] << 8);
    //iat = (speedyResponse [15] << 8);
  }

boolean isRPMDeviationBiggerThan(int number) {
   int diff = abs((int)rpm - (int)rpmBefore);
   return diff > number;
}

boolean screenupdateDue() {
   Serial.print("RPM-"); Serial.print(millis() - lastScreenUpdate); Serial.print("\n");
   return (millis() - lastScreenUpdate) > 5000;
}


void updaterpm(int rpm) {

  }

void clearRX() {
  while(Serial3.available()) Serial3.read();
}
