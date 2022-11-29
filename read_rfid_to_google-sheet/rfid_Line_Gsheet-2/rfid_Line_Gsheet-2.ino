#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>

// SETUP PIN RFID
#define RST_PIN D1
#define SS_PIN D2
MFRC522 mfrc522(SS_PIN, RST_PIN);
String t;
#define ON_Board_LED 2

const char *ssid = "xxxxxxxxxxxxxxxxxx";
const char *password = "xxxxxxxxxxxxxxxxxxxx";

const char *host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

long now = millis();
long lastMeasure = 0;

String GAS_ID = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; //--> spreadsheet script ID

String rfid_read()
{
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? ",0" : ","));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  return content.substring(1);
}
void setup()
{
  Serial.begin(9600);
  delay(500);
  SPI.begin();                                               // Init SPI bus
  mfrc522.PCD_Init();                                        // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:")); // shows in serial that it is ready to read
  WiFi.begin(ssid, password);                                //--> Connect to your WiFi router
  Serial.println("");

  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
  }
  digitalWrite(ON_Board_LED, HIGH);
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  client.setInsecure();
}
void loop()
{

  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  // some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  Serial.println(F("**Card Detected:**"));
  String send_key = "A";
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); // DUMP DETAIL
  send_key = rfid_read();
  // INTERFACE
  Serial.print(F("Data: "));
  byte buffer1[18];
  block = 4;
  len = 18;

  // GET DATA
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); // line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  String ct4 = "";
  String send_id = "";
  // PRINT STUDENT ID
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      ct4.concat(char(buffer1[i]));
      send_id = ct4;
      Serial.write(buffer1[i]);
    }
  }
  Serial.print(" ");

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); // line 834
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  String ct = "";
  String ct1 = "";
  String ct2 = "";
  String send_name = "";
  String send_lname = "";
  String send_blood = "";
  int cnt = 0;
  Serial.print("\n");
  // PRINT FRISTNAME,LASTNAME,BLOOD
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer2[i] != 32)
    {
      if (buffer2[i] == 124)
      {
        cnt++;
        Serial.print("\n");
        continue;
      }
      if (cnt == 0)
      {
        ct.concat(char(buffer2[i]));
        send_name = ct;
        Serial.write(buffer2[i]);
      }
      if (cnt == 1)
      {
        ct1.concat(char(buffer2[i]));
        send_lname = ct1;
        Serial.write(buffer2[i]);
      }
      if (cnt == 2)
      {
        ct2.concat(char(buffer2[i]));
        send_blood = ct2;
        Serial.write(buffer2[i]);
      }
    }
  }

  Serial.println(F("\n**End Reading**\n"));

  delay(1000); // change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  sendData(send_key, send_id, send_name, send_lname, send_blood);
}

void sendData(String value1, String value2, String value3, String value4, String value5)
{
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);

  // Connect to Google host
  if (!client.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    return;
  }
  String rfid_key = value1;
  String std_id = value2;
  String std_name = value3;
  String std_lastname = value4;
  String std_blood = value5;
  String url = "/macros/s/" + GAS_ID + "/exec?rfid_key=" + rfid_key + "&std_id=" + std_id + "&std_name=" + std_name + "&std_lastname=" + std_lastname + "&std_blood=" + std_blood; //  3 variables
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\""))
  {
    Serial.println("esp8266/Arduino CI successfull!");
  }
  else
  {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
}