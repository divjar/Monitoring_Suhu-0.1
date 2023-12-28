// Tagar notifikasi sensor suhu dengan web dan telegram
// token untuk ID Blynk
#define BLYNK_TEMPLATE_ID "TMPL6cmLpYKJr"
#define BLYNK_TEMPLATE_NAME "Monitoring Suhu Server NOC"
#define BLYNK_AUTH_TOKEN "EMOXfvUMlicJCvVLd0frx0YZEmcn94jh"
#define BLYNK_PRINT Serial
//---------------------------------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
//---------------------------------------------------
#define DHTPIN D4
#define DHTTYPE DHT22  //DHT11
//---------------------------------------------------
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
//---------------------------------------------------
int buzzer = D8; //kita menggunakan pin 5 untuk buzzer indikator suara alarm
int led_awas = D0; //kita menggunakan pin 5 untuk led merah indikator awas
int led_aman = D1; //kita menggunakan pin 4 untuk led hijau indikator aman
// Koneksi WIFI
char ssid[] = "UMM-ICT";     // diisi nama wifi
char password[] = "UMM.1964"; // diisi password wifi
char auth[] = BLYNK_AUTH_TOKEN;
//masukan domain http://www.example.com/sensordata.php
const char* SERVER_NAME = "http://suhu.umm.ac.id/sensordata.php";
//API harus disamakan dengan yg ada di config.php
String PROJECT_API_KEY = "hello world";

// ID Telegram untuk BOT
#define BOTtoken "6278281775:AAF-OrIWLbRZn96JZ5fklZsHJ9EogCwirrk" // diisi Token Bot (Dapat dari Telegram Botfather) //DivjarSuhu_bot
//-----------------------------------------------------
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//memeriksa pesan baru setiap 1 detik.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
//-------------------------------------------------------------------
//Send an HTTP POST request every 30 seconds
unsigned long lastMillis = 0;
long interval = 5000;
//-------------------------------------------------------------------


//-------------------------------------------------------------------
void setup(){
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  // bisa konfigurasi spesifik server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  
  dht.begin();
  pinMode(buzzer, OUTPUT); //mengatur buzzer sebagai OUTPUT
  pinMode(led_awas, OUTPUT); //mengatur led sebagai OUTPUT
  pinMode(led_aman, OUTPUT); //mengatur led sebagai OUTPUT
  timer.setInterval(2500L, sendSensor);

Serial.begin(115200);
  dht.begin();
  // Ini adalah cara paling sederhana agar ini berfungsi
  // jika Anda menyampaikan informasi sensitif, atau mengendalikan
  // sesuatu yang penting, silakan gunakan certStore atau di
  // paling sedikit klien.setFingerPrint
  client.setInsecure();

  // Atur WiFi ke mode stasiun dan putuskan sambungan dari AP sebelumnya
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Mencoba terhubung ke jaringan:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}
//-------------------------------------------------------------------
void loop() {
  Blynk.run();
  timer.run();
  int t = dht.readTemperature();
  int h = dht.readHumidity();
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    if(WiFi.status()== WL_CONNECTED){
    if(millis() - lastMillis > interval) {
       //Send an HTTP POST request every interval seconds
       upload_temperature();

    lastTimeBotRan = millis();
    }
  }
}
}
//--------------------------------------
void upload_temperature()
{
  //--------------------------------------------------------------------------------
  //Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  //Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  //Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  //--------------------------------------------------------------------------------
  //°C
  String humidity = String(h, 2);
  String temperature = String(t, 2);
  String heat_index = String(hic, 2);

  Serial.println("Temperature: "+temperature);
  Serial.println("Humidity: "+humidity);
  //Serial.println(heat_index);
  Serial.println("--------------------------");
  //--------------------------------------------------------------------------------
  //HTTP POST request data
  String temperature_data;
  temperature_data = "api_key="+PROJECT_API_KEY;
  temperature_data += "&temperature="+temperature;
  temperature_data += "&humidity="+humidity;

  Serial.print("temperature_data: ");
  Serial.println(temperature_data);
  //--------------------------------------------------------------------------------
  
  WiFiClient client;
  HTTPClient http;

  http.begin(client, SERVER_NAME);
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Send HTTP POST request
  int httpResponseCode = http.POST(temperature_data);
  //--------------------------------------------------------------------------------
  // If you need an HTTP request with a content type: 
  //application/json, use the following:
  //http.addHeader("Content-Type", "application/json");
  //temperature_data = "{\"api_key\":\""+PROJECT_API_KEY+"\",";
  //temperature_data += "\"temperature\":\""+temperature+"\",";
  //temperature_data += "\"humidity\":\""+humidity+"\"";
  //temperature_data += "}";
  //int httpResponseCode = http.POST(temperature_data);
  //--------------------------------------------------------------------------------
  // If you need an HTTP request with a content type: text/plain
  //http.addHeader("Content-Type", "text/plain");
  //int httpResponseCode = http.POST("Hello, World!");
  //--------------------------------------------------------------------------------
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
    
  // Free resources
  http.end();
  }
//-------------------------------------------------------------------
void sendSensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // atau dht.readTemperature(true) untuk Fahrenheit //(-2 untuk kalibrasi sensor)
  if (isnan(h) || isnan(t)) {
    Serial.println("SENSOR TIDAK TERBACA!!!");
    return;
  }

  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V0, t);
  Serial.print("Temperature : ");
  Serial.print(t);
  Serial.print("    Humidity : ");
  Serial.println(h);


//if(t > 27){
    //Blynk.email("tagaralaziis@gmail.com", "Alert", "Temperature over 28C!"); // untuk notifikasi email
// Blynk.logEvent("temp_alert","Temp above 27 degrees");
// }


if ( h > 95.00){      //jika kelembapan lebih besar dari 95.00
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (300);

      digitalWrite(led_awas, HIGH);   //led merah menyala
      digitalWrite(led_aman, LOW);    //led hijau mati
       }

 else if( t > 27.00){         //jika suhu lebih besar dari 27.00 
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (100);
     digitalWrite(buzzer, HIGH);   //buzzer menyala 
     delay (100);
     digitalWrite(buzzer, LOW);
     delay (300);
     
     digitalWrite(led_awas, HIGH);   //led merah menyala
     digitalWrite(led_aman, LOW);    //led hijau mati
     }
          
 else{                              //jika tidak   
     digitalWrite(buzzer, LOW);    //buzzer mati
     digitalWrite(led_aman, HIGH); //led hijau menyala
     digitalWrite(led_awas, LOW);  //led merah mati
      }
}


//-------------------------------------------------------------------
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";
 
    //Cek Pembacaan Sensor DHT11
    if (text == "/suhu") {
      int t = dht.readTemperature(); //(-2 untuk kalibrasi sensor)
       String temp = "● Suhu NOC : ";
       temp += int(t);
       temp +=" °C\n";
      
      bot.sendMessage(chat_id,temp, "");
    }
    if (text == "/kelembapan") {
      int h = dht.readHumidity();
       String temp = "● Kelembaban NOC : ";
       temp += int(h);
       temp += " %";
     
      bot.sendMessage(chat_id,temp, "");
    }
    
    //Cek Command untuk setiap aksi
    if (text == "/start") {
      String welcome = "Hi " + from_name + ", pilih pengecekan:\n";
      welcome += "🌡️ /suhu  (Status Suhu)\n";
      welcome += "💧 /kelembapan (Status Kelembapan)\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}
