#define TRIG_PIN_1 5
#define ECHO_PIN_1 18
#define TRIG_PIN_2 14
#define ECHO_PIN_2 27

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
}

long readUltrasonicDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // Timeout 30ms to avoid blocking
  long distance = duration * 0.034 / 2; // Convert to cm
  return distance;
}

void loop() {
  long distance1 = readUltrasonicDistance(TRIG_PIN_1, ECHO_PIN_1);
  long distance2 = readUltrasonicDistance(TRIG_PIN_2, ECHO_PIN_2);
  
  float meanDistance = (distance1 + distance2) / 2.0;

  Serial.print("Sensor 1: ");
  Serial.print(distance1);
  Serial.print(" cm\t");

  Serial.print("Sensor 2: ");
  Serial.print(distance2);
  Serial.print(" cm\t");

  Serial.print("Mean Distance: ");
  Serial.print(meanDistance);
  Serial.println(" cm");

  delay(20000);
}
