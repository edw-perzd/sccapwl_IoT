int pHSense = 35; // Pin del ESP32 para el sensor de pH
int samples = 10; // Número de pruebas que se tomarán de pH
float adc_resolution = 4095.0; //ESP 32 ADC Resolution

void setup()
{
  Serial.begin(9600);
  delay(100);
  Serial.println("Sensor de PH");
}

// Método que devuelve el pH mediante el voltaje recibido
float ph (float voltage) {
  return 7 + ((1.65 - voltage) / 0.18);
}

void loop()
{
  int measurings=0; // Variable donde se guardaran las muestras

  for (int i = 0; i < samples; i++)
  {
    measurings += analogRead(pHSense);
    delay(10);
  }
  // Se promedian las muestras para obtener el voltaje
  float voltage = 3.3 / adc_resolution * measurings/samples;

  Serial.print("pH= ");
  Serial.println(ph(voltage)); // Se manda el voltaje para obtener el pH
  delay(1000);
}
