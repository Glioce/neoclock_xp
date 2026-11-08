/*
  This example uses NeoDigitos, 7 segments displays based on Adafruit NeoPixel
  Use a ESP32 or ESP8266 and connect to internet to get Time and Weather

  Las siguientes son bibliotecas que ya se encuentran en el core de ESP32:

    Preferences - para guardar datos en la memoria no volátil
    https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/preferences.html
    https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/

    Ticker - para llamar funciones de forma periódica, se usa en los beeps de la alarma
    https://github.com/espressif/arduino-esp32/tree/master/libraries/Ticker

    time - obtener fecha y hora en diferentes formatos
    https://forum.arduino.cc/t/finding-out-which-time-h-is-used-for-compiling/664185

    Bibliotecas para conexión WiFi

    Bibliotecas para conectar a server remoto y manejar un server local

  Para obtener información del clima se usa el servicio wttr.in
  https://github.com/chubin/wttr.in
  Las consultas se realizan con el formato para obtener salida de una líniea
*/


// Para ESP8266
//#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>

// Para ESP32
#include <Arduino.h>
#include <Ticker.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <NeoDigito.h>
#include "time.h"
#include <Preferences.h>
#include <WebServer.h>
#include <ESPmDNS.h>  // Librería para mDNS en ESP32


#define PIN         2   // Pin where the display will be attached, ESP-01 Only GPIO 3(RX)
#define DIGITS      4   // Number of NeoDigitos connected
#define PIXPERSEG   2   // NeoPixels per segment,3 for BIG version, 2 for MEDIO
NeoDigito neodisplay = NeoDigito(DIGITS, PIXPERSEG, PIN);

Preferences prefs; //object to save preferences
//const char* ssid = "ssid";    // Your WiFi Network
//const char* pass = "pass";    // Password

String city = "Cholula+de+Rivadavia";    // Place name of your city for wheater
//https://wttr.in/Puebla?format=%C+%f+%h+%T+%l&lang=es
//https://wttr.in/Cholula+de+Rivadavia?format=%C+%f+%h+%T+%l&lang=es
String url = "https://wttr.in/" + city + "?format=%C+%f+%h+%T+%l&lang=es";  // wheater service, change language
const char* ntpServer = "pool.ntp.org";           // NTC server for time
const long  gmtOffset_sec = -6 * 60 * 60;         // Adjust for time zone for (UTC -6): -6 * 60 * 60
const int   daylightOffset_sec = 0;               // Set it to 3600 if your country observes Daylight saving time; otherwise, set it to 0.

#define LED_PIN 8  // LED para mostrar estado de la alarma (opcional)
#define BUZZER_PIN 3 //pin conectado al buzzer o bocina
WebServer server(80);  // Servidor en puerto 80
//bool ledState = false; // Estado inicial del LED
const char* hostname = "neoclock";  // Nombre para acceder: http://neoclock.local
#include "htmlPage.h" //página web raw literal

// Variables para almacenar la alarma
int alarmHours = 0;
int alarmMinutes = 0;
bool alarmActive = false;
bool alarmDays[7] = {false};  // 0=Dom, 1=Lun, ..., 6=Sáb
Ticker alarmBeeper; //enciende el buzzer en periodos cortos (sonido beep)
const int beepPeriod = 500; //periodo de beep en milisegundos
Ticker alarmStopper; //temporizador para detener la alarma
const int alarmDuration = 20; //duracion de la alarma en segundos
bool alarmStarted = false; //indica si la alarma ha iniciado

// Función para hacer beep
void beep() {
  digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
  Serial.println("beep");
}

// Función para apagar desactivar la alarma
void stopAlarm() {
  alarmBeeper.detach(); //desactiva el ticker beeper
  digitalWrite(BUZZER_PIN, LOW); //apaga buzzer
  Serial.println("Alarm stopped");
}

// Función para cargar la alarma desde Preferences
void loadAlarmSettings() {
  prefs.begin("alarm", false);
  Serial.print("\nLoading saved alarm settings\n");
  alarmHours = prefs.getInt("hours", 0);
  Serial.println(alarmHours);
  alarmMinutes = prefs.getInt("minutes", 0);
  Serial.println(alarmMinutes);
  alarmActive = prefs.getBool("active", false);
  Serial.println(alarmActive);
  digitalWrite(LED_PIN, !alarmActive); //LED enciende en bajo
  for (int i = 0; i < 7; i++) {
    String d = String("day") + i;
    alarmDays[i] = prefs.getBool(d.c_str(), false);
    Serial.println(alarmDays[i]);
  }
  prefs.end();
  Serial.println();
}

// Función para guardar la alarma en Preferences
void saveAlarmSettings() {
  prefs.begin("alarm", false);
  Serial.print("\nSaving alarm settings\n");
  prefs.putInt("hours", alarmHours);
  Serial.println(alarmHours);
  prefs.putInt("minutes", alarmMinutes);
  Serial.println(alarmMinutes);
  prefs.putBool("active", alarmActive);
  Serial.println(alarmActive);
  digitalWrite(LED_PIN, !alarmActive); //LED enciende en bajo
  for (int i = 0; i < 7; i++) {
    String d = String("day") + i;
    prefs.putBool(d.c_str(), alarmDays[i]);
    Serial.println(alarmDays[i]);
  }
  prefs.end();
  Serial.println();
}

// Genera la página web con los valores actuales
String generateHTML() {
  String html = htmlPage;
  html.replace("%HOURS%", String(alarmHours));
  html.replace("%MINUTES%", String(alarmMinutes));
  html.replace("%ACTIVE%", alarmActive ? "checked" : "");
  html.replace("%MONDAY%", alarmDays[1] ? "checked" : "");
  html.replace("%TUESDAY%", alarmDays[2] ? "checked" : "");
  html.replace("%WEDNESDAY%", alarmDays[3] ? "checked" : "");
  html.replace("%THURSDAY%", alarmDays[4] ? "checked" : "");
  html.replace("%FRIDAY%", alarmDays[5] ? "checked" : "");
  html.replace("%SATURDAY%", alarmDays[6] ? "checked" : "");
  html.replace("%SUNDAY%", alarmDays[0] ? "checked" : "");
  html.replace("%IP%", WiFi.localIP().toString());
  return html;
}

// Maneja la página principal
void handleRoot() {
  server.send(200, "text/html", generateHTML());
}

// Procesa el formulario de la alarma
void handleSetAlarm() {
  if (server.method() == HTTP_POST) {
    alarmHours = server.arg("hours").toInt();
    alarmMinutes = server.arg("minutes").toInt();
    alarmActive = server.hasArg("alarm-active");

    // Reinicia los días
    for (int i = 0; i < 7; i++) alarmDays[i] = false;

    // Marca los días seleccionados
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "days") {
        int day = server.arg(i).toInt();
        if (day >= 0 && day <= 6) alarmDays[day] = true;
      }
    }

    saveAlarmSettings();
    server.sendHeader("Location", "/");
    server.send(303);
  }
}

// Variables para controlar lo que se muestra en el reloj
unsigned long previousMillisTime = 0;
unsigned long previousMillisWeather = 0;
unsigned long startDisplayTime = 0;
const long intervalTime = 500;
const long intervalWeather = 90000;
const long displayDuration = 2000;
const long displayDurationConditions = 500;
int weatherStage = 0;
bool displayingWeather = false;
String condition, temperature, humidity;

// Variables globales para almacenar la última hora y minutos conocidos
int lastKnownHour = -1;
int lastKnownMinute = -1;
bool lastKnownDelimiter = false; // Para almacenar el estado del delimitador
String dayAndDate;

// Función para convertir el número del día de la semana al nombre en español
String dayInSpanish(int dayOfWeek) {
  const char* days[] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
  return days[dayOfWeek];
}

// Función para obtener el nombre del mes
String monthInSpanish(int month) {
  const char* months[] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};
  return months[month];
}

// ------------------------------- printLocalTime
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    neodisplay.print("FAIL", Red);
    return;
  }

  //verificar si se ha alcanzado la hora para activar la alarma
  bool isAlarmTime = false;

  // Verificar si los segundos son pares
  bool isSecondEven = (timeinfo.tm_sec % 2 == 0);

  // Actualizar delimitador solo si ha cambiado
  if (isSecondEven != lastKnownDelimiter) {
    lastKnownDelimiter = isSecondEven;
    if (isSecondEven) {
      neodisplay.updateDelimiter(2, White);
    }
    else
      neodisplay.updateDelimiter(2, 0);
  }

  // Actualizar hora solo si ha cambiado
  if (timeinfo.tm_hour != lastKnownHour) {
    lastKnownHour = timeinfo.tm_hour;
    neodisplay.setCursor(0);
    String timeString = "";
    if (lastKnownHour < 10) {
      timeString += " ";
    }
    timeString += String(lastKnownHour);
    neodisplay.print(timeString, White); //Cian

    // comparar la hora de la alarma
    //isAlarmTime = (timeinfo.tm_hour == alarmHours);
    //isAlarmTime = (lastKnownHour == alarmHours);
    //Serial.print("hora "); Serial.println(lastKnownHour);
  }

  // Actualizar minutos solo si han cambiado
  if (timeinfo.tm_min != lastKnownMinute) {
    lastKnownMinute = timeinfo.tm_min;
    String timeString = "";
    if (lastKnownMinute < 10) {
      timeString += "0";
    }
    timeString += String(lastKnownMinute);
    neodisplay.setCursor(2);
    neodisplay.print(timeString, White); //Pink

    // comparar la hora y los minutos de la alarma
    //isAlarmTime = isAlarmTime and (timeinfo.tm_min == alarmMinutes);
    isAlarmTime = (lastKnownHour == alarmHours) and (lastKnownMinute == alarmMinutes);
    Serial.println(String(lastKnownHour) + ":" + lastKnownMinute);

    // Iniciar alarma si se ha alcanzado la hora
    // falta verficar dias de la semana y que la alarma está activada
    if (isAlarmTime) {
      if (alarmStarted == false) {
        alarmBeeper.attach_ms(beepPeriod, beep); //milisegundos
        alarmStopper.once(alarmDuration, stopAlarm); //segundos
        alarmStarted = true;
        Serial.println("Alarm started");
      }
    }
    else {
      alarmStarted = false;
    }
  } // fin actualizar minutos

  // Crear una cadena para el día y la fecha
  String day = dayInSpanish(timeinfo.tm_wday);
  String month = monthInSpanish(timeinfo.tm_mon);
  dayAndDate = "   " + day + " " + String(timeinfo.tm_mday) + " " + month + " "  + String(timeinfo.tm_year + 1900);

}


// ----------------------------------- setup
void setup()
{
  Serial.begin(115200);
  Serial.print("\n\nNeoClock\n\n");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  // Inicialiar digitos
  neodisplay.begin();               // This fuction calls Adafruit_NeoPixel.begin() to configure.
  neodisplay.setBrightness(51);     // 25% of brightness, min 1 and max 255
  neodisplay.print("WiFi", Red);    // It prints the value, on Red color.

  // Iniciar conexión a Wifi (varios pasos)
  Serial.println("Reading saved WiFi config");
  prefs.begin("wifi-config", false); // Modo RW (no borrar al abrir)

  // Intenta conectarse con credenciales guardadas
  String savedSSID = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end(); //abc

  if (savedSSID != "") { //Si hay datos guardados
    Serial.println("Conecting with saved credentials");
    //Serial.println(savedSSID);
    //Serial.println(savedPass);
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
      delay(500);
      Serial.print(".");
      intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) { //Si ya está conectado
      Serial.println("\nConectado a WiFi guardado!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      //return; // Sale del setup si ya está conectado
    }
    else {
      Serial.println("\nCan't connect to saved WiFi");
    }
  }
  else {
    Serial.println("No saved credentials");
  }

  // Si no hay credenciales o falla la conexión, inicia SmartConfig
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Iniciando SmartConfig...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();

    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("\nSmartConfig recibido!");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // Guarda las credenciales en NVS
    prefs.begin("wifi-config", false); //abc
    prefs.putString("ssid", WiFi.SSID());
    prefs.putString("pass", WiFi.psk());
    prefs.end();

    // Espera conexión WiFi
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("\nConectado a WiFi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }

  // Iniciar mDNS
  if (!MDNS.begin(hostname)) {
    Serial.println("Error iniciando mDNS!");
  } else {
    Serial.printf("mDNS iniciado\nAccede a: http://%s.local\n", hostname);
  }

  // Cargar configuración guardada de la alarma
  loadAlarmSettings();

  // Configurar rutas del servidor
  server.on("/", handleRoot);
  server.on("/set-alarm", handleSetAlarm);
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // Mostrar palabra WiFi en color verde
  Serial.print("\nReady\n");
  neodisplay.updateColor(Green);    // Color specified by name Red, White, YELLOW, etc or 32bit, or 8bit numbers (R, G, B)
  delay(500);

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // Once config is not WiFi connection needed
  printLocalTime();

}


// ----------------------------------- loop
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisTime >= intervalTime && !displayingWeather) {
    previousMillisTime = currentMillis;
    printLocalTime();
  }

  if (currentMillis - previousMillisWeather >= intervalWeather && !displayingWeather) {
    previousMillisWeather = currentMillis;
    displayingWeather = true;
    weatherStage = 0;

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      HTTPClient http;

      // Ignore SSL certificate errors
      client.setInsecure();

      http.begin(client, url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println(payload);

        int plusSign = payload.indexOf('+');
        int percentSign = payload.indexOf('%');
        int colonSign = payload.indexOf(':');
        int minusSign = payload.lastIndexOf('-');
        int lastSpace = payload.lastIndexOf(' ');

        String time = payload.substring(percentSign + 2, minusSign - 1);  // Time is between the percentage sign and the minus sign
        String timezone = payload.substring(minusSign, lastSpace);  // Timezone is between the minus sign and the last space
        String location = payload.substring(lastSpace + 1);  // Location is everything after the last space
        //condition = "   " + location + " " + payload.substring(0, plusSign - 1);
        condition = "   " + payload.substring(0, plusSign - 1);
        temperature = payload.substring(plusSign + 1, percentSign - 3);
        humidity = payload.substring(percentSign - 2, percentSign + 1);

        startDisplayTime = currentMillis;
      }

      http.end();
    }
  }

  if (displayingWeather) {
    if (weatherStage <= 2 && currentMillis - startDisplayTime >= displayDuration) {
      startDisplayTime = currentMillis;
      neodisplay.clear();
      weatherStage++;
      if (weatherStage == 1) {
        neodisplay.print(temperature, Orange); //Purple
      } else if (weatherStage == 2) {
        neodisplay.print(humidity, Cian); //Cian
      }
    } else if (weatherStage > 2 && currentMillis - startDisplayTime >= displayDurationConditions) {
      startDisplayTime = currentMillis;
      neodisplay.clear();
      weatherStage++;
      int startChar = weatherStage - 3;
      if (startChar < condition.length()) {
        String conditionSnippet = condition.substring(startChar, startChar + 4);
        neodisplay.print(conditionSnippet, Yellow);
      }
      // Luego mostrar el día y la fecha
      else if (startChar - condition.length() < dayAndDate.length()) {
        int dateChar = startChar - condition.length();
        String dateSnippet = dayAndDate.substring(dateChar, dateChar + 4);
        neodisplay.print(dateSnippet);
        neodisplay.updateColor(Purple); //Rainbow
      }
      else {
        displayingWeather = false;
        lastKnownHour = -1;
        lastKnownMinute = -1;
        lastKnownDelimiter = false;
      }
    }
  }
  //Serial.println("loop");
  server.handleClient();  // Manejar peticiones web
}
