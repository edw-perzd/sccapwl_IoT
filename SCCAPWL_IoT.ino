#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <addons/TokenHelper.h>
// #include <LiquidCrystal.h> // Libreria para la pantalla LCD
#include <OneWire.h>            // Libreria para la comunicación ONEWIRE
#include <DallasTemperature.h>  // Libreria para el sensor de temperatura

#define PIN_TEMPERATURA 2  // Pin para el sensor de temperatura
#define PIN_TDS 27         // Pin para el sensor de TDS
#define PIN_PH 35          // Pin para el sensor de pH

#define WIFI_SSID "AndroidA"
#define WIFI_PASSWORD "epnx0141"

#define API_KEY "AIzaSyDBVSgdW3Sd5eXOgU4Wmym348_ZesCadsc"
#define FIREBASE_PROJECT_ID "sccapwl-9fc3d"
#define USER_EMAIL "legna642.dav@gmail.com"
#define USER_PASSWORD "lamisma1"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int samples = 10;               // Número de pruebas que se tomarán de pH
float adc_resolution = 4095.0;  //ESP 32 ADC Resolution

OneWire oneWire(PIN_TEMPERATURA);   // Se inicializa comunicación ONEWIRE
DallasTemperature temp1(&oneWire);  // Se inicializa en sensor de temperatura

float averageVoltage = 0;
float tdsValue = 0;

// LiquidCrystal lcd(22, 23, 5, 18, 19, 21); // Se inicializa la pantalla LCD


void setup() {
  Serial.begin(9600);
  Serial.println("Sensor de PH");
  Serial.println("Sensor de Temperatura");

  /*LCD*/
  // lcd.begin(16, 2);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Conexión exitosa");
  Serial.print("Conectado con la IP: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);
  
}

// Método que devuelve el pH mediante el voltaje recibido
float ph(float voltage) {
  return 7 + ((1.65 - voltage) / 0.18);
}

void loop() {
  String documentPath = "depositos/Rm7rB9BUFS4wrynACoqN";
  FirebaseJson json;
  json.set("fields/agua_contenida/mapValue/fields/cantidad_agua/stringValue", "300");
  json.set("fields/agua_contenida/mapValue/fields/estado_agua/booleanValue", false);
  json.set("fields/agua_contenida/mapValue/fields/nivelTDS_agua/stringValue", "121");
  json.set("fields/agua_contenida/mapValue/fields/nivelTemp_agua/stringValue", "25.10");
  json.set("fields/agua_contenida/mapValue/fields/nivelpH_agua/stringValue", "6.59");

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), json.raw(), "agua_contenida")) {
    Serial.println("Datos enviados correctamente");
  } else {
    Serial.println("Error al enviar los datos");
    Serial.println(fbdo.errorReason());
  }
  delay(5000);


  int measurings = 0;     // Variable donde se guardaran las muestras de pH
  int measuringstds = 0;  // Variable donde se guardaran las muestras de TDS
  for (int i = 0; i < samples; i++) {
    measurings += analogRead(PIN_PH);  // Lectura del sensor de pH
    measuringstds += analogRead(PIN_TDS);
    delay(10);
  }

  // Se promedian las muestras para obtener el voltaje
  float voltage = (3.3 / adc_resolution * measurings / samples);
  float averageVoltage = (3.3 / adc_resolution * measuringstds / samples);

  Serial.print("pH= ");
  Serial.println(ph(voltage));  // Se manda el voltaje para obtener el pH

  /* Se obtiene la temperatura */
  temp1.requestTemperatures();
  float temper = temp1.getTempCByIndex(0);
  /****************************/

  if (temper != DEVICE_DISCONNECTED_C)  // Verificar si se hizo la medición
  {
    Serial.print("Temperatura: ");
    Serial.println(temper);
    Serial.print(" °C");

    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationCoefficient = 1.0 + 0.02 * (temper - 25.0);
    //temperature compensation
    float compensationVoltage = averageVoltage / compensationCoefficient;

    //convert voltage value to tds value
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;

    Serial.print("TDS Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");

    /*LCD*/
    // lcd.setCursor(0, 0);
    // lcd.print("Temp(C): ");
    // lcd.print(temper);

    // lcd.setCursor(0, 2);
    // lcd.print("pH: ");
    // lcd.print(ph(voltage));
  } else {
    Serial.println("Error: No se pudo leer la temperatura");
  }
  delay(1000);
  // Turn off the display:
  // lcd.noDisplay();
  // delay(500);
  // Turn on the display:
  // lcd.display();
  // delay(500);
}
