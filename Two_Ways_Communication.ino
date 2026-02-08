#include <esp_now.h>
#include <WiFi.h>

#include <LiquidCrystal.h>

/*
  LCD pin mapping:
  RS  -> GPIO 13
  EN  -> GPIO 14
  RW  -> GND
  D4  -> GPIO 33
  D5  -> GPIO 32
  D6  -> GPIO 4
  D7  -> GPIO 5
*/

// Create An LCD Object. Signals: [ RS, EN, D4, D5, D6, D7 ]
LiquidCrystal lcd(13, 14, 33, 32, 4, 5);

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xA0, 0xB7, 0x65, 0x05, 0x5D, 0xC8};

// Define variables readings to be sent
char  req_message = 1;
int   ack_status = 0;

// Define variables to store incoming readings
float incomingTemp  = 0;
float incomingHum   = 0;
float incomingpH    = 0;
float incomingBat   = 0;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int   request;
    int   acknowledge;
    float Temp;
    float Hum;
    float pH;
    float Battery;
} struct_message;

// Create a struct_message called Sending_Data to hold sensor readings
struct_message Sending_Data;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
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

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTemp  = incomingReadings.Temp;
  incomingHum   = incomingReadings.Hum;
  incomingpH    = incomingReadings.pH;
  incomingBat   = incomingReadings.Battery;
  ack_status    = incomingReadings.acknowledge;
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Initialize LCD
  lcd.begin(16, 4);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.printf("Memulai... ");
  lcd.setCursor(0, 1);
  lcd.printf("Silahkan ");
  lcd.setCursor(0, 2);
  lcd.printf("Hidupkan Sensor!");
  lcd.setCursor(0, 3);
  lcd.printf("Menyambung... ");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  Sending_Data.request = 1;
  ack_status = 0;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Sending_Data, sizeof(Sending_Data));
}
 
int Timeout_Counter = 1;

void loop() {
   // Send message via ESP-NOW
  Sending_Data.request = 1;
  ack_status = 0;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Sending_Data, sizeof(Sending_Data));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(3000);

  if(ack_status == 0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Gagal Terhubung!");
    lcd.setCursor(0, 1);
    lcd.printf("Mencoba Berusaha");
    lcd.setCursor(0, 2);
    lcd.printf("Menyambung Lagi!");
    lcd.setCursor(0, 3);
    lcd.printf("+ Percobaan: %d", Timeout_Counter);
    Timeout_Counter++;
  }else{
    LCD_Display(incomingTemp, incomingpH, incomingHum, incomingBat);
    Timeout_Counter = 1;
  }

  delay(1000);
}

void LCD_Display(float Temp, float pH, float Humid, float Power){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Temp  : %.2f C", Temp);
  lcd.setCursor(0, 1);
  lcd.printf("pH    : %.2f", pH);
  lcd.setCursor(0, 2);
  lcd.printf("Humid : %.2f %%", Humid);
  lcd.setCursor(0, 3);
  lcd.printf("Power : %.1f  %%", Power);
}