//#include <SoftwareSerial.h>                         // for 2598, use pin8 & 9 as mySerial
//SoftwareSerial mySerial(8, 9);

String user = "oliver";                    //  使用者(登入Mail)
String device_1 = "oYTiodvm4WRU6HkUXWCr";             //  裝置的存取權仗 1

String value_1 = "millis";                            //  資料類型 1 (顯示於雲端"鍵"的分類)
String value_2 = "random_1";                          //  資料類型 2 (顯示於雲端"鍵"的分類)
String value_3 = "random_2";                          //  資料類型 3 (顯示於雲端"鍵"的分類)
String topic = "\"nbiot/relay/1\"";         //  MQTT伺服器中的主題
String settingValue = "";
String settingRelay = "";

String data = "";                  //  儲存來自BC26回覆的字串參數
int sta = 1;                       //  迴圈的status參數判斷
int time_count = 0;
int count = 0;                     //  副程式中來自BC26的回覆次數計數器
int band = 8;                      //  設定頻段(B8)
int IP_count1 = 0;
void(* resetFunc) (void) = 0;      //  宣告系統重置參數
int reset_count = 0;               //  系統重新啟動計時器宣告
String data_tempt = "";            //  各種字串參數暫存器
String server_IP = "oliver0804.ddns.net";    // 上傳Web網址
int server_port = 1883;            //  不修改 (for MQTT)
String noreply = "no reply, reset all";
int runMode = 0;

int relayMqttTime = 30;
int relayOneTime = 30;
int relayTwoTime = 30;

String message(String data, String value_No) {
  data_tempt = "\"{\"";
  data_tempt.concat(value_No);
  data_tempt.concat("\":");
  data_tempt.concat(data);
  data_tempt.concat("}\"");

  //  Serial.println("message: \n" + data_tempt);
  return data_tempt;
}   // END of message

//String message(){
//  data_tempt = "\"{\"";
//  for(int i = 0; i < sizeof(sensor); i++){
//
//    data_tempt.concat(sensor[i]);
//    data_tempt.concat("\":");
//    data_tempt.concat(data[i]);
//    if(i < sizeof(sensor)){
//      data_tempt.concat("\", \"");
//    }else{
//      data_tempt.concat("\"");
//    }
//  }
//  data_tempt.concat("}\"");
//
//  Serial.println("message: " + data_tempt);
//  return data_tempt;
//}

void serial_read() {
  while (Serial1.available()) { //Check if there is an available byte to read,
    delay(10); //Delay added to make thing stable
    char c = Serial1.read(); //Conduct a serial read
    if (c == '\n') {
      break; //Exit the loop when the "" is detected after the word
    }
    data += c; //Shorthand for data = data + c
  }

  if (data.length() > 0) { // 判斷data內有值在更換
    data.trim();
    delay(100);
    Serial.print(sta);
    Serial.print(">>");
    Serial.println(data);
    if (sta == 1) {
      count++;
    }

    if (sta == 1) {
      if (count >= 1) {       //  Turn on BC26 status
        Serial.println("Turn on BC26 status");
        //Serial.println("sta = 1");
        sta++;
        count = 0;
      }
    } else if (sta == 2) {     //  wait for IP status
      for (int i = 0; i < 3; i++) {
        data_tempt += data[i];
      }
      Serial.println("Wait for IP status..");
      if (data_tempt == "+IP") {
        Serial.println("wait for IP status.."); //Serial.println("sta = 2");
        sta++;
      } else if (data_tempt == "+CG") {            // add from old version, 20200309
        sta++;  //Serial.println("sta = 2.5");
      }
    } else if (sta == 3) {

      if (data[0] == '+' && data[4] == 'R') {
        Serial.print(">>>>"); Serial.println(data[27]);
        settingRelay += data[27];//編號
        if (data[27] == '1' || data[27] == '2' || data[27] == '3') {
          settingValue += data[31];
          settingValue += data[32];
          settingValue += data[33];
          settingValue += data[34];
          Serial.print(">>>>"); Serial.println(settingValue.toInt());
          switch (settingRelay.toInt()) {
            case 1:
              Serial.print(">>>>"); Serial.println("Open Realy 1");
              digitalWrite(PB13, LOW);
              delay(settingValue.toInt());
              Serial.print(">>>>"); Serial.println("Close Realy 1");
              digitalWrite(PB13, HIGH);
              break;
            case 2:
              Serial.print(">>>>"); Serial.println("Open Realy 2");
              digitalWrite(PB14, LOW);
              delay(settingValue.toInt());
              Serial.print(">>>>"); Serial.println("Close Realy 2");
              digitalWrite(PB14, HIGH);
              break;
            case 3:
              Serial.print(">>>>"); Serial.println("Open Realy 3");
              digitalWrite(PB15, LOW);
              delay(settingValue.toInt());
              Serial.print(">>>>"); Serial.println("Close Realy 3");
              digitalWrite(PB15, HIGH);
              break;
          }
        } else if (data[27] == 'r' && data[28] == 'u' && data[29] == 'n') {
          

          runMode = 1;
          settingValue = "";
          settingValue += data[33];
          settingValue += data[34];
          settingValue += data[35];
          settingValue += data[36];
          relayMqttTime = settingValue.toInt();
          relayOneTime = relayMqttTime;
          settingValue = "";
          settingValue += data[33 + 4];
          settingValue += data[34 + 4];
          settingValue += data[35 + 4];
          settingValue += data[36 + 4];
          relayMqttTime = settingValue.toInt();
          relayTwoTime = relayMqttTime;

          Serial.print(">>>>");
          Serial.print("run:RELAYTIME");
          Serial.print("@");
          Serial.print(relayOneTime);
          Serial.print("@");

          Serial.print(relayTwoTime);
        }
      }

      //      if(data == "+QMTOPEN: 0,0"){         //  與MQTT伺服器連線
      //        sta++; Serial.println("sta = 3");
      //      }
    } else if (sta == 4) {
      if (data == "OK" || data == "+QMTCONN: 0,0,0") {     //  與裝置建立通道並連線
        delay(5 * 1000);
        //        sta++;
      }
    } else if (sta > 4) {      //  傳輸資料與伺服器端訊息回傳
      //      if(data == "OK"){
      //        sta++;
      //      }else if(data == "+QMTCLOSE: 0,0"){
      //        sta++;
      //      }
    }
  }
  settingRelay = "";
  settingValue = "";
  data = "";
  data_tempt = "";
}     // END of Serial read

void BC26_reset() {                               //  reset BC26
  Serial1.println("AT+QRST=1");
  while (sta == 1) {          //  等待模組訊息回覆
    serial_read();
    delay(1 * 1000);
    count++;
    if (count > 10) {        //  超過10秒未有回覆，重新啟動系統
      count = 0;
      Serial.println(noreply);
      resetFunc();
    }
  }
  count = 0;
}             // END of BC26 reset

void ask_for_IP() {                               //  查詢IP狀況
  Serial1.println("AT+CGPADDR=1");
  while (sta == 2) {          //  等待模組訊息回覆
    serial_read();
    delay(1 * 1000);
    count++;
    if (count > 10) {       //  每10秒問一次IP狀況
      IP_count1++;
      Serial1.println("AT+CGPADDR=1");
      count = 0;
      if (IP_count1 > 6) {    //  一分鐘後仍沒找到IP，重新開機
        Serial.println(noreply);
        resetFunc();
      }
    }
  }
  count = 0;
  IP_count1 = 0;
}                 // END of ask for IP


void build_MQTT_connect() {                        //  建立TCP連線通道
  data_tempt = "AT+QMTOPEN=0,\"";
  data_tempt.concat(server_IP);
  data_tempt.concat("\",");
  data_tempt.concat(server_port);
  Serial1.println(data_tempt);
  //  while (sta == 3) {          //  等待模組訊息回覆
  //    serial_read();
  //    delay(1*1000);
  //    count++;
  //    if (count > 10) {        //  超過10秒未有回覆，重新啟動系統
  //      count = 0;
  //      Serial.println(noreply);
  //      resetFunc();
  //    }
  //  }
  count = 0;
  data_tempt = "";
}                   // END of build MQTT connect

void connect_MQTT(String device_No) {                  //  與伺服器連接
  data_tempt = "AT+QMTCONN=0,\"";
  data_tempt.concat(user);
  data_tempt.concat("\",\"");
  data_tempt.concat(device_No);
  data_tempt.concat("\"");
  Serial1.println(data_tempt);
  //  while (sta == 4) {          //  等待模組訊息回覆
  //    serial_read();
  //    delay(1*1000);
  //    count++;
  //    if (count > 10) {         //  超過10秒未有回覆，重新啟動系統
  //      count = 0;
  //      Serial.println(noreply);
  //      resetFunc();
  //    }
  //  }
  count = 0;
  data_tempt = "";
}                   // END of connect MQTT

void reading(int sta_pre, int sta) {
  while (sta_pre == sta) {        //  等待模組訊息回覆
    serial_read();
    delay(1 * 1000);
    break;
    count++;
  }
  if (count > 10) { //  超過10秒未有回覆，
    count = 0;
    Serial.println("no replay, send next data after 2 seconds");
    delay(1 * 1000);
    sta_pre++;
  }
  count = 0;
  sta_pre = sta;
  data_tempt = "";
}

void data_publish(String topic, String message, String device_No) {    //  資料上傳function
  Serial.println("Sending data......");
  int sta_pre = sta;

  build_MQTT_connect();                       // need IP & port
  delay(3 * 1000);
  //connect_MQTT();                           // was
  connect_MQTT(device_No);
  delay(3 * 1000);

  data_tempt = "AT+QMTPUB=0,0,0,0,";
  data_tempt.concat(topic);
  data_tempt.concat(",");
  data_tempt.concat(message);
  Serial.println(data_tempt);
  //  "AT+QMTPUB=0,0,0,0,\"v1/devices/me/telemetry\",\"{\"key1\":data, \"key2\":data}\""
  Serial1.println(data_tempt);
  delay(3 * 1000);

  Serial1.println("AT+QMTCLOSE=0");
  //  delay(1*1000);

}       // END of data publish

void BC26_initail() {
  BC26_reset();
  delay(20 * 1000);     //  等待20秒連線
  ask_for_IP();         //  查詢IP狀況
  delay(2 * 1000);
  ask_for_IP();         //  查詢IP狀況
  delay(2 * 1000);
}

void long_delay(int hr, int minut, int sec) {       //  設定delay時間長度
  for (int i = 0; i < (hr * 60 * 60 + minut * 60 + sec); i++) {
    delay(1000);
  }
}
