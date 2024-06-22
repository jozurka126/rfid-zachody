//setup
/*pinout
ESP - vec
rfid:
3V3 - 3V3
22-RST (meniteľné)
GND - GND
19 - MISO
23 - MOSI
18 - SCK
5 - SDA (meniteľné)
dispaly:
VIN - 5V
GND - GND
TX-RX
RX-TX
buzzer:
GND- - 
4 - + (meniteľné)
*/
//time setup
#define buzz 4
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <vector>
#define dw digitalWrite

const char* ssid     = "Wifi_Smolar_EXT";
const char* password = "neotravuj";
//server setup
const char* serverUrl = "https://jsonplaceholder.typicode.com/posts";
 //time setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);
//koniec time setupu

//display setup
void napis(String nazov, String co){
  Serial.print(nazov+".txt=\""+co+"\"");
  Serial.write(0xff);Serial.write(0xff);Serial.write(0xff);
}
#define toto_napis_ked_karta "ok"
//rfid setup
#define RST_PIN     22          // Reset pin (RST) for RC522
#define SS_PIN      5          // Slave Select pin (SS/SDA) for RC522
MFRC522 mfrc522(SS_PIN, RST_PIN);
String read();
String read(){
  MFRC522::StatusCode status;
  byte block = 13;  // Choose the block where you want to store the integer
  byte key[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // 6-byte key (all 0xFF)
  MFRC522::MIFARE_Key mifareKey;
  for (byte i = 0; i < 6; i++) {
      mifareKey.keyByte[i] = key[i];
  }
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &mifareKey, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    mfrc522.PICC_HaltA();        // Halt PICC
    mfrc522.PCD_StopCrypto1();
    return("0");
  }
  // Read data from the block
  byte buffer[18];
  byte bufferSize = sizeof(buffer);
  status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
  if (status != MFRC522::STATUS_OK) {
    mfrc522.PICC_HaltA();        // Halt PICC
    mfrc522.PCD_StopCrypto1();
    return("0");
  }
  
  // Convert bytes to integer
  int value = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
  mfrc522.PICC_HaltA();        // Halt PICC
    mfrc522.PCD_StopCrypto1();
  return(String(value));
}
//api setup
String send_id();
String send_id(String id){
  //api.send(id);
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["userId"] = id;
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl); // Specify the URL
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonString);
    if (httpResponseCode == 201) {
      String response = http.getString();
      return(response);
    }
  }
  return("macicka");

}
void get_data(String id){ //netreba?
  //String data = api.get(id);
  napis(String("t1"), String("tu budu infosky, id je")+id);
}
//buzz easy
void bzuc(int time){
  dw(buzz, HIGH);
  delay(time);
  dw(buzz, LOW);
}
//koniec?



void setup() {
  //wifi, buzzer, serial komunikácia, rfid
  
  Serial.begin(9600);
  //rfid
  SPI.begin();                    // Initialize SPI bus
  mfrc522.PCD_Init(); 
  //wifi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  //wifi koniec
  pinMode(buzz, OUTPUT);
  //time
  timeClient.begin();
  Serial.write(0xff);Serial.write(0xff);Serial.write(0xff);
  //json
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["userId"] = 0;
}
int stav = 0;
String last_time = "";
void loop() {
    /*
  predstava:
  aktualizuj čas
  ak je karta a stav = 0, pošli jej id na x sekúnd, pošli id na server
  ak stav = 1, pošli na server request, získaj údaje, pošli ich do displaya
  prijímaj správy z displaya vo forme čísel - stavy
  ak je stav 2, čakaj na správu z displaya s id/zmenou stavu, keď dostaneš skontroluj či je karta, ak hej daj na ňu nove id čo 
  si dostal
  */
  if (stav == 0){//klasicka obrazovka, ukaze id, posle do api(soon)
    timeClient.update();
    String cas = String(timeClient.getFormattedTime());
    if (cas!=last_time){
    napis(String("t3"), cas);
    napis(String("t2"), String("Time:"));
    last_time = cas;}
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      digitalWrite(buzz, HIGH);
      napis(String("t2"), String("ID:"));
      String id = read();
      if (id == "0"){napis(String("t3"), String("error"));}
      else {napis(String("t3"), toto_napis_ked_karta);}
      delay(10); digitalWrite(buzz, LOW);
      //stav = 6;
      Serial.println(send_id(id));
      delay(500);}
    if (Serial.available()>0){
      String got = Serial.readString();
      if (int(got[0])==1){stav=1;}
      if (int(got[0])==2){stav=2;}
      //Serial.println(got.length());
      //Serial.println(got[0]);Serial.println(got[1]);Serial.println(got[2]);Serial.println(got[3]);
    }
  }
  if (stav==1){
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
      String idc = read();
      //Serial.println(send_id(idc));
      bzuc(750); delay(250);
      /* potom tu napisať tie veci, na displayi sú 4 sloty
      t1 : t5
      t2:t6
      t3:t7
      t4:t8
      napise sa tam funkciou napis("kde", "co") napr napis(String("t1), String(macicka"))
      */
      //* alebo použiť toto
      String response = send_id(idc);
      StaticJsonDocument<1024> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, response);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      } else {
        // Iterate through all key-value pairs
        std::vector<String> nahram;
        for (JsonPair kv : jsonDoc.as<JsonObject>()) {
          nahram.push_back(kv.key().c_str());
          nahram.push_back(kv.value().as<String>());
        }
        nahram.push_back("");nahram.push_back("");nahram.push_back("");nahram.push_back("");nahram.push_back("");nahram.push_back("");nahram.push_back("");nahram.push_back("");
        napis(String("t1"), nahram[0]);napis(String("t2"), nahram[2]);napis(String("t3"), nahram[4]);napis(String("t4"), nahram[6]);
        napis(String("t5"), nahram[1]);napis(String("t6"), nahram[3]);napis(String("t7"), nahram[5]);napis(String("t8"), nahram[7]);
      }
      //*/
    }
    if (Serial.available()>0){
      String got = Serial.readString();
      if (int(got[0])==0){stav=0;}
    }
  }
  if (stav==2){
    if (Serial.available()>0){
      int got = int(Serial.readString().toInt());
      if (got == 0){stav=0;}
      else if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
        String c = String(got);
        MFRC522::StatusCode status;
        byte block = 13;  // Choose the block where you want to store the integer
        byte key[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // 6-byte key (all 0xFF)
        MFRC522::MIFARE_Key mifareKey;
        for (byte i = 0; i < 6; i++) {
            mifareKey.keyByte[i] = key[i];
        }
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &mifareKey, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
          Serial.println("Authentication failed");
          return;
        }
        byte buffer[18];
        byte bufferSize = sizeof(buffer);
        napis(String("t1"), String("menim na "+c));
        dw(buzz, HIGH);delay(100);dw(buzz, LOW);
        delay(500);
        int newValue = got; // Change this to your desired 4-digit integer
        buffer[0] = (newValue >> 24) & 0xFF;
        buffer[1] = (newValue >> 16) & 0xFF;
        buffer[2] = (newValue >> 8) & 0xFF;
        buffer[3] = newValue & 0xFF;
        status = mfrc522.MIFARE_Write(block, buffer, 16);
        if (status != MFRC522::STATUS_OK) {
          napis(String("t1"), String(status));
          return;
        }
        napis(String("t1"), String("nahrate"));
        mfrc522.PICC_HaltA();        // Halt PICC
        mfrc522.PCD_StopCrypto1();
      }
      else{
        napis(String("t1"), String("Ziadna karta"));
      }
    }
  }/*
  if (Serial.available()>0){
    String got = Serial.readString();
    //Serial.println(got);
    if (int(got[0])==1){
      timeClient.update();
      napis(String("t0"), String(timeClient.getFormattedTime()));
    }
    if (int(got[0])==2){
      napis(String("t0"), String("som22"));delay(1000);
    }
    if (int(got[0])==3){
      napis(String("t0"), String("somthird"));delay(1000);
    }
  }*/
  //napis("t0", printLocalTime());

}
