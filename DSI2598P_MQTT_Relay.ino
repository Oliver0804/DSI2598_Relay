#include "BC26.h"

//AT+QRST=1
//Check +CPIN:READY
//Check +IP:
//建立連線
//AT+QMTOPEN=0,"oliver0804.ddns.net",1883
//wait OK
//AT+QMTOPEN=0,"oliver0804.ddns.net",1883
//wait OK
//AT+QMTCONN=0,"ID"  device number
//wait OK
//AT+QMTSUB=0,1,"nbiot/test",2//使用0不會重複
//+QMTRECV: 0,1,"nbiot/test","9999"

int relay[4] = {0};
int relaytime[4] = {0};
int keeptime = 0;
void setup() {
  pinMode(PB12, OUTPUT);
  pinMode(PB13, OUTPUT);
  pinMode(PB14, OUTPUT);
  pinMode(PB15, OUTPUT);
  digitalWrite(PB12, HIGH);
  digitalWrite(PB13, HIGH);
  digitalWrite(PB14, HIGH);
  digitalWrite(PB15, HIGH);
  // put your setup code here, to run once:
  Serial.begin(115200);//Serial
  Serial1.begin(115200);//BC26
  BC26_initail();
  Serial.println("connect server.");
  build_MQTT_connect();
  delay(30 * 1000);
  Serial.println("connect MQTT.");
  connect_MQTT("DSI2598");
  delay(30 * 1000);
    digitalWrite(PB12, LOW);
  Serial1.print("AT+QMTSUB=0,1,\"nbiot/relay/1\",0,\"nbiot/relay/2\",0,\"nbiot/relay/3\",0\n\r");
}

void loop() {
  if (keeptime >=10000) {
    
    Serial.println("keep alive");
    Serial1.print("AT+QMTSUB=0,1,\"nbiot/relay/1\",0,\"nbiot/relay/2\",0,\"nbiot/relay/3\",0\n\r");
    keeptime = 0;
  } else {
    keeptime++;
    delay(1);
  }
  //data_publish(topic, message(String(data_random_1), value_2), device_1);
  // delay(1000);
  if (Serial.available()) Serial1.write(Serial.read());
  // Serial.println(sta);
  serial_read();
}
