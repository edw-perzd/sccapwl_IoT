#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <addons/TokenHelper.h>
// #include <LiquidCrystal.h> // Libreria para la pantalla LCD
#include <OneWire.h>            // Libreria para la comunicación ONEWIRE
#include <DallasTemperature.h>  // Libreria para el sensor de temperatura

#define PIN_TEMPERATURA 2  // Pin para el sensor de temperatura
#define PIN_TDS 27         // Pin para el sensor de TDS
#define PIN_PH 35          // Pin para el sensor de pH

#define WIFI_SSID "WINDOWS-L4A2JTV 2447"
#define WIFI_PASSWORD "desktop_EDU123"

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

float tdsValue = 0;

const char* tz = "CET-1CEST,M3.5.0,M10.5.0/3";
int DISTANCIA = 0;
int pinEco = 19;
int pinGatillo = 21;
long readUltrasonicDistance(int triggerPin, int echoPin) {
  //Iniciamos el pin del emisor de reuido en salida
  pinMode(triggerPin, OUTPUT);
  //Apagamos el emisor de sonido
  digitalWrite(triggerPin, LOW);
  //Retrasamos la emision de sonido por 2 milesismas de segundo
  delayMicroseconds(2);
  // Comenzamos a emitir sonido
  digitalWrite(triggerPin, HIGH);
  //Retrasamos la emision de sonido por 2 milesismas de segundo
  delayMicroseconds(10);
  //Apagamos el emisor de sonido
  digitalWrite(triggerPin, LOW);
  //Comenzamos a escuchar el sonido
  pinMode(echoPin, INPUT);
  // Calculamos el tiempo que tardo en regresar el sonido
  return pulseIn(echoPin, HIGH);
}

void setup() {
  pinMode(PIN_TDS, INPUT);

  Serial.begin(9600);
  Serial.println("Sensor de PH");
  Serial.println("Sensor de Temperatura");

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

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", tz, 1);
  tzset();
}

// Método que devuelve el pH mediante el voltaje recibido
double ph(float voltage) {
  return 7 + ((1.65 - voltage) / 0.18);
}

void loop() {
  int measuringstds = 0;  // Variable donde se guardaran las muestras de TDS
  for (int i = 0; i < samples; i++) {
    measuringstds += analogRead(PIN_TDS);
    delay(10);
  }
  float averageVoltage = (3.3 / adc_resolution * measuringstds / samples);

  //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationCoefficient = 1.0 + 0.02 * (25.0 - 25.0);
  //temperature compensation
  float compensationVoltage = averageVoltage / compensationCoefficient;

  //convert voltage value to tds value
  tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;


  Serial.print("TDS Value:");
  Serial.print(tdsValue);
  Serial.println("ppm");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

    String documentPath = "depositos/WL041216";
    FirebaseJson json;
    json.set("fields/agua_contenida/mapValue/fields/nivelTDS_agua/doubleValue", tdsValue);

    int measurings = 0;  // Variable donde se guardaran las muestras de pH
    for (int i = 0; i < samples; i++) {
      measurings += analogRead(PIN_PH);  // Lectura del sensor de pH
      delay(10);
    }

    // Se promedian las muestras para obtener el voltaje
    float voltage = (3.3 / adc_resolution * measurings / samples);


    Serial.print("pH= ");
    Serial.println(ph(voltage));  // Se manda el voltaje para obtener el pH

    /* Se obtiene la temperatura */
    temp1.requestTemperatures();
    float temper = temp1.getTempCByIndex(0);
    /****************************/

    if (temper != DEVICE_DISCONNECTED_C)  // Verificar si se hizo la medición
    {
      Serial.print("Temperatura: ");
      Serial.print(temper);
      Serial.println(" °C");
      json.set("fields/agua_contenida/mapValue/fields/nivelTemp_agua/doubleValue", temper);

      double pH = ph(voltage);  // Se manda el voltaje para obtener el pH
      json.set("fields/agua_contenida/mapValue/fields/nivelpH_agua/doubleValue", pH);

      DISTANCIA = 0.01723 * readUltrasonicDistance(pinGatillo, pinEco);
      //Mostramos la disstancia
      Serial.println(DISTANCIA);
      //Si la distancia es menor a 20 encendemos el led
      delay(10);
      double cantAgua = DISTANCIA;
      json.set("fields/agua_contenida/mapValue/fields/cantidad_agua/doubleValue", cantAgua);

      Serial.print("TDS Value:");
      Serial.print(tdsValue);
      Serial.println("ppm");
      if (pH < 5.5 || tdsValue > 500) {
        json.set("fields/agua_contenida/mapValue/fields/estado_agua/booleanValue", false);
      } else {
        json.set("fields/agua_contenida/mapValue/fields/estado_agua/booleanValue", true);
      }
      json.set("fields/agua_contenida/mapValue/fields/ultimaActualizacion_agua/stringValue", timeString);
    } else {
      Serial.println("Error: No se pudo leer la temperatura");
    }
    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), json.raw(), "agua_contenida")) {
      Serial.println("Datos enviados correctamente");
    } else {
      Serial.println("Error al enviar los datos");
      Serial.println(fbdo.errorReason());
    }
  }

  delay(5000);
}
