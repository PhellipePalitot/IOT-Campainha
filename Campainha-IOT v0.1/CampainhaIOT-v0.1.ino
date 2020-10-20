
/*
Projeto em desenvolvimento pelo estudante Luis Phellipe Palitot Moreno
atualmente cursando Engenharia da computação na ufpb.
Base do código desenvolvido a partir da biblioteca "UniversalTelegramBot.h".
*/




#include <DNSServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#define CAMERA_MODEL_AI_THINKER

// Funções Declaradas 
void tiraFoto(String chat_id);
void flashControl(bool i);

// Initialize Wifi connection to the router
char ssid[] = "**********";     // your network SSID (name)
char password[] = "******"; // your network key

// Initialize Telegram BOT
#define BOTtoken "**************************"  // your Bot Token (Get from Botfather)

//------- ------------------------ ------

#include "camera_pins.h"
#include "camera_code.h"

#define FLASH_LED_PIN 4

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

bool flashState = LOW;

camera_fb_t * fb = NULL;

bool isMoreDataAvailable();
byte* getNextBuffer();
int getNextBufferLen();

bool dataAvailable = false;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    //ID 1 e ID 2 são os ID's das pessoas autorizadas a utilizar o programa.

    if(chat_id == "ID-1" || chat_id == "ID-2"){
    }else{
      text = "/intruso";
    }
    
    if (text == "/intruso"){
       String welcome = "Você não tem permição.";
       bot.sendMessage(chat_id, welcome, "Markdown");
    }

    if (text == "/photo") {

      tiraFoto(chat_id);
    }
    if (text == "/photoflash") {
      flashControl(true);
      tiraFoto(chat_id);
      flashControl(false);
    }

    if (text == "/start") {
      String welcome = "Bem vindo a IOT-Campainham, Desenvolvido Por Phellipe Palitot.\n\n";
      welcome += "/photo : Tirar uma foto\n";
      welcome += "/photoflash : Tirando uma foto com o flash\n\n";
      welcome += "My GitHub: https://github.com/PhellipePalitot";
      welcome += "https://www.instagram.com/phellipepalitot/";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}
 
bool isMoreDataAvailable() {
  if (dataAvailable)
  {
    dataAvailable = false;
    return true;
  } else {
    return false;
  }
}

byte* getNextBuffer(){
  if (fb){
    return fb->buf;
  } else {
    return nullptr;
  }
}

int getNextBufferLen(){
  if (fb){
    return fb->len;
  } else {
    return 0;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState); //defaults to low

  if (!setupCamera()) {
    Serial.println("Camera Setup Failed!");
    while (true) {
      delay(100);
    }
  }

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Make the bot wait for a new message for up to 60seconds
  bot.longPoll = 60;
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
}

//Inicio das Funções.

//Função TiraFoto.
void tiraFoto(String chat_id){
  fb = NULL;

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(chat_id, "Camera capture failed", "");
        return;
      }
      dataAvailable = true;
      Serial.println("Sending");
      bot.sendPhotoByBinary(chat_id, "image/jpeg", fb->len,
                            isMoreDataAvailable, nullptr,
                            getNextBuffer, getNextBufferLen);

      Serial.println("done!");

      esp_camera_fb_return(fb); 
}
//Função liga/desliga flash.
void flashControl(bool i){
  digitalWrite(FLASH_LED_PIN, i);
}
