#include <SwitecX12.h>

#define MAXRPM 7300
#define MINRPM 500
#define MINRPMPOSITION 3270

const int STEPS = 315 * 12;
const int A_STEP = 8;
const int A_DIR = 9;
const int RESET = 10;

SwitecX12 rpmmotor(STEPS, A_STEP, A_DIR);
#define RX3_PIN 14
#define TX3_PIN 15
static uint32_t oldtime = millis();
uint8_t speedyResponse[100]; //The data buffer for the Serial data. This is longer than needed, just in case
uint8_t byteNumber[2];  // pointer to which uint8_t number we are reading currently
uint8_t readclt; // clt doesn't need to be updated very ofter so
int clt;   // to store coolant temp
unsigned int rpm;  //rpm and PW from speeduino
float afr;
float mapData;
int8_t psi;
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

  digitalWrite(RESET, HIGH);
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
    rpmmotor.setPosition(calcposition(rpm));
  
  if (millis() > debug) {
  debug = millis() + (long unsigned)500;
  Serial.print("RPM-"); Serial.print(calcposition(rpm)); Serial.print("\n");
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


  }

void clearRX() {
  while(Serial3.available()) Serial3.read();
}
