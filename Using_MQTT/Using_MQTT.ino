#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP_Mail_Client.h>
#include "credentials.h"

#define MQ2pin 34
int sensorValue = 0;
#define TRIG_PIN_1 5
#define ECHO_PIN_1 18
#define TRIG_PIN_2 14
#define ECHO_PIN_2 27

SMTPSession smtp;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // // Switch on the LED if an 1 was received as first character
  // if ((char)payload[0] == '1') {
  //   digitalWrite(12, LOW);   // Turn the LED on (Note that LOW is the voltage level
  //   // but actually the LED is on; this is because
  //   // it is active low on the ESP-01)
  // } else {
  //   digitalWrite(12, HIGH);  // Turn the LED off by making the voltage HIGH
  // }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("device/dustbin1", "MQTT Server is connected");
      // ... and resubscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  
  Serial.println("MQ2 warming up!");
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  smtp.debug(1);

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

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    long distance1 = readUltrasonicDistance(TRIG_PIN_1, ECHO_PIN_1);
    long distance2 = readUltrasonicDistance(TRIG_PIN_2, ECHO_PIN_2);
    float meanDistance = (distance1 + distance2) / 2.0;
    Serial.print("Sensor 1: ");
    Serial.print(distance1);
    Serial.print(" cm\t");

    Serial.print("Sensor 2: ");
    Serial.print(distance2);
    Serial.print(" cm\t");
    sensorValue = analogRead(MQ2pin); // read analog value (0 - 4095)
    Serial.print("MQ2 Analog Value: ");
    Serial.println(sensorValue);

    snprintf (msg, MSG_BUFFER_SIZE, "Total Distance is: %4f and Gas Level: %d", meanDistance, sensorValue);
    Serial.print("Publish message: ");
    Serial.println(msg);
    delay(3000);
    client.publish("device/dustbin1", msg);
    if(meanDistance<4)
    {
        snprintf (msg, MSG_BUFFER_SIZE, "Total Distance is: %4f", meanDistance);
        Serial.print("Publish message: ");
        Serial.println(msg);
        delay(3000);
        client.publish("device/dustbin1", msg);

        smtp.debug(1);
        ESP_Mail_Session session;
        session.server.host_name = SMTP_SERVER ;
        session.server.port = SMTP_PORT;
        session.login.email = SENDER_EMAIL;
        session.login.password = SENDER_PASSWORD;
        session.login.user_domain = "";
        /* Declare the message class */

        SMTP_Message message;
        message.sender.name = "ESP 32";
        message.sender.email = SENDER_EMAIL;
        message.subject = "Dustbin Level";
        message.addRecipient(RECIPIENT_NAME,RECIPIENT_EMAIL);
        //Send HTML message
        String htmlMsg = "<div style=\"color:#000000;\"><h1> Dustbin-1 Filled!</h1><p>Dustbin-1 Is Full.</p></div>";
        message.html.content = htmlMsg.c_str();
        message.html.content = htmlMsg.c_str();
        message.text.charSet = "us-ascii";
        message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
        if (!smtp.connect(&session))
        {
          return;
        }

        if (!MailClient.sendMail(&smtp, &message))
        {
          Serial.println("Error sending Email, " + smtp.errorReason());
        }
        delay(15000);
    }
 
    
  }
}