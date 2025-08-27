#define MQ2pin 34   // Analog output of MQ2 connected to GPIO34

int sensorValue = 0;  // variable to store sensor value

void setup() {
  Serial.begin(115200); // start serial monitor
  Serial.println("MQ2 warming up!");
  delay(4000); // allow MQ2 to warm up
}

void loop() {
  sensorValue = analogRead(MQ2pin); // read analog value (0 - 4095)
  
  Serial.print("MQ2 Analog Value: ");
  Serial.println(sensorValue);

  delay(1000); // wait 1s before next reading
}
