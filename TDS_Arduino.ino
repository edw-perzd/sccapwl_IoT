#define PIN_TDS A1         // Pin para el sensor de TDS

float tdsValue = 0;

int samples = 10;               // Número de pruebas que se tomarán de pH
float adc_resolution = 1024.0;  //ARDUINO UNO ADC Resolution

void setup() {
  Serial.begin(9600);

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

  Serial.print(tdsValue);

  delay(5000);

}
