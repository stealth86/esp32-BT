#include "BluetoothSerial.h"

BluetoothSerial serialBT;
char cmd;

void setup() {
  // put your setup code here, to run once:
  serialBT.begin("ESP32-BT");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(serialBT.available()){
    cmd = serialBT.read();
  }
  delay(20);
}
