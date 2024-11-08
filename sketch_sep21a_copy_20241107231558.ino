#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_SSD1306.h>   
#define ANCHO 128     // reemplaza ocurrencia de ANCHO por 128
#define ALTO 64       // reemplaza ocurrencia de ALTO por 64
#define OLED_RESET -1      //-- configuracion reset
Adafruit_SSD1306 oled(ANCHO, ALTO, &Wire, OLED_RESET);  // crea objeto

const char *ssid = "Fer";
const char *password = "juan6933";
WebServer server(80);
const int led = 13;

/////////////////////////////////////////////////////////////////////
const int sensorPin = 34; // Pin al que está conectado el sensor
volatile int sensorValue = 0;
volatile long timeold = 0; 

const int frenoPin = 35; // Pin al que está conectado el sensor2
volatile int frenoValue = 0;
volatile long frenoTime = 0; 

int rpm = 0;
int velocidad = 0;
volatile long frenado = 0; 
//////////////////////////////////////////////////////

void handleRoot() {
  digitalWrite(led, 1);
  char temp[400];

  snprintf(
    temp, 400,

    "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1> Hola bienvenido ESP32 </h1> <p> </p> \
    <div><h1> Vel: %02d K/h</h1> <h1> Rpm: %02d rpm</h1> <h1> Frenado: %02d ms </h1></div>\
  </body>\
 </html>",

    velocidad, rpm, frenoTime
  );
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);

}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  //server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
////////////////////////////////////////////////////////////////////////////////////////
  pinMode(sensorPin, INPUT);
  pinMode(frenoPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), rpmInterrupt, RISING);
  Serial.begin(9600);
//////////////////////////////////////////////////////////////////////////////////////    
  Wire.begin();         // inicializa bus I2C
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C
  oled.setRotation(0);  // se escoje la orientacion del display puede ser 0, 1, 2 o 3
  oled.setTextColor(WHITE);  // se le dice a la pantalla que el texto que se va a desplegar es en color blanco
  oled.dim(false); //dejamos el brillo en maximo

}

void loop(void) {
  server.handleClient();
  delay(2);  //allow the cpu to switch to other tasks
  long time = millis();
  if (time - timeold >= 1000) {   
    timeold = time;
    Rpm();
    mostrar(); // mostrar en lcd
  }
    if (digitalRead(frenoPin)){ freno();
    delay(5000); }
}
void freno(){
   long time1 = millis();
   long time2 = millis();
   delay(10);
   while(digitalRead(frenoPin)){
   long time = millis();
  if (time - time2 >= 100) { 
     if (sensorValue == 0) {   
        frenoTime = time2 - time1;
      }else{
    time2 = millis();
    sensorValue=0;
      }
   }  
  }
}
//////////////////////////////////////////////////
void rpmInterrupt() {
  sensorValue++;
}
void Rpm(){
    rpm = sensorValue*60;
    rpm = rpm/48;
    velocidad = rpm*1.25*60;
    velocidad = velocidad/1000;
  }
void mostrar(){
  oled.clearDisplay();   
  oled.drawRect(14, 0, 100, 13, WHITE); 
  oled.setCursor(18, 3);   
  oled.setTextSize(1);   
  oled.setTextColor(WHITE); 
  oled.print("DISPLAY SSD1306"); 
    
  oled.setCursor(32, 17); 
  oled.setTextSize(3);
  oled.print(velocidad); 
  oled.setTextSize(1);
  oled.print(" K/h"); 
    
  oled.setCursor(0, 45); 
  oled.setTextSize(1);
  oled.print(rpm); 
  oled.print(" Rpm"); 
  
  oled.setCursor(0, 55); 
  oled.print(sensorValue); 
  oled.print(" Hz"); 
    
  oled.setCursor(55, 55);
  oled.print(frenoTime); 
  oled.print(" /s");
    
  oled.display();   
    
  sensorValue = 0;
}
