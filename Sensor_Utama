#include <esp_now.h>
#include <WiFi.h>

// ===========================================================
#include <Wire.h>               //I2C
#include <Adafruit_GFX.h>       //OLED
#include <Adafruit_SSD1306.h>   //OLED

// Display Dimension
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// ===========================================================

// ===========================================================
#include <OneWire.h>
#include <DallasTemperature.h>  //DS18B20
const int oneWireBus = 18;    
#define DS_OFFSET 3.3
OneWire oneWire(oneWireBus);
DallasTemperature Tempsensors(&oneWire);
// ===========================================================

// ===========================================================
#include "DHT.h"                //DHT11/22
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
// ===========================================================

// ===========================================================
#define LDR 35
#define BATTERY 34
#define PH_METER 32 // OR 33
#define MOSFET_PIN 4

#define PH_OFFSET 33.628
#define PH_SLOPE -0.014476
#define MAX_TIMER 5
// ===========================================================


// Mac Address
uint8_t broadcastAddress[] = {0x78, 0x1C, 0x3C, 0x2C, 0x17, 0xEC};

// Tipe Status Pengiriman Data
char  req_message = 0;
int   ack_status = 0;

float incomingTemp  = 0;
float incomingHum   = 0;
float incomingpH    = 0;
float incomingBat   = 0;

String success;

// Struct Data
typedef struct struct_message {
    int   request;
    int   acknowledge;
    float Temp;
    float Hum;
    float pH;
    float Battery;
} struct_message;

// Struct Pengiriman
struct_message Sending_Data;

// Struct Penerimaan
struct_message incomingReadings;

// Variabel Pengukuran Sebelumnya (data temporary dulu)
float prev_LightRead        = 1000;
float prev_Battery_voltage  = 8;
float prev_Temp_Read        = 21;
float prev_ph_level         = 0.0;
float prev_humid            = 80.0;
float prev_airtemp          = 25.0;
int   Count                 = 0;

esp_now_peer_info_t peerInfo;

// Status Pengiriman
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Success!";
  }
  else{
    success = "Fail..";
  }
}

// Status Penerimaan
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  req_message  = incomingReadings.request;
}
 
void setup() {
  // Serial Monitor
  Serial.begin(115200);

  // MOSFET Sensor pH
  pinMode(MOSFET_PIN, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();

  display.setTextSize(1.75);
  display.setTextColor(WHITE);

  // Memulai sensor
  Tempsensors.begin();
  dht.begin();
  digitalWrite(MOSFET_PIN, HIGH);

  // Memulai WIFI
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Setup untuk ESP-Now
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;   
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  display.clearDisplay();

  // ============ Sensor ============ //
  Tempsensors.requestTemperatures(); 
  if (Count >= 80){
    digitalWrite(MOSFET_PIN, LOW);
    delay(100);
    Count = 0;
    digitalWrite(MOSFET_PIN, HIGH);
  }
  float LightRead   = analogRead(LDR);
  int   BatteryRead = analogRead(BATTERY);
  int   pH_Read     = analogRead(PH_METER);
  float Temp_Read   = Tempsensors.getTempCByIndex(0) - DS_OFFSET;

  float dht_hum     = dht.readHumidity();
  float dht_temp    = dht.readTemperature();

  float Battery_voltage     = 0.78 + BatteryRead*5.82/1071;
  // float Battery_Percentage  = 100*(Battery_voltage - 5.5)/(9.5 - 5.5);
  float pH_level            = PH_SLOPE*pH_Read + PH_OFFSET;

  float LightDisplay    = (LightRead + prev_LightRead)/2;
  float TempDisplay     = (Temp_Read + prev_Temp_Read)/2;
  float pHDisplay       = (pH_level + prev_ph_level)/2;
  float humid_display   = (dht_hum + prev_humid)/2;
  float AirTemp_display = (dht_temp + prev_airtemp)/2;
  float BatteryDisplay  = (prev_Battery_voltage + Battery_voltage)/2;

  float Battery_Percentage  = 100*(BatteryDisplay - 3.3)/(9 - 3.3);
  // ============ Sensor ============ //

  // ============== Payload ============== //
  Sending_Data.Temp     = TempDisplay;
  Sending_Data.pH       = pHDisplay;
  Sending_Data.Hum      = humid_display;
  Sending_Data.Battery  = Battery_Percentage;
  // ============== Payload ============== //

  esp_err_t result = 0;
  if(req_message == 1){
    Sending_Data.acknowledge = 1;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Sending_Data, sizeof(Sending_Data));
    req_message = 0;
  } else{
  }

  DisplayText(TempDisplay, AirTemp_display, LightDisplay, pHDisplay, humid_display, Battery_Percentage, result);
  delay(2000);
  // ============== Sesudah Sensor ============== //
  Count++;

  prev_LightRead        = LightRead;
  prev_Battery_voltage  = Battery_voltage;
  prev_Temp_Read        = Temp_Read;
  prev_ph_level         = pH_level;
  prev_humid            = dht_hum;
  prev_airtemp          = dht_temp;


  // ============== Sesudah Sensor ============== //
}

void DisplayText(float Temp, float AirTemp, int Light, float pH, float Humid, float Volt, int Result){
  display.setCursor(0, 5);
  display.print("Suhu Air : ");
  display.print(Temp);
  display.println(" C");
  // display.print("Suhu DHT : ");
  // display.print(AirTemp);
  // display.println(" C");
  // display.print("Cahaya   : ");
  // display.println(Light);
  display.print("pH level : ");
  display.println(pH);
  display.print("Lembab   : ");
  display.print(Humid);
  display.println(" %");
  display.print("Daya     : ");
  display.print(Volt);
  display.println(" %");
  display.print("Status   : ");

  if (Result == ESP_OK) {
    display.println("Sucess");
  }
  else {
    display.println("Error");
  }
  display.display(); 
}

