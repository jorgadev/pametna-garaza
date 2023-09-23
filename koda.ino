#include <SPI.h>
#include <MFRC522.h>
#include <dht.h>
dht DHT;

#define echoPin 3       // Motion sensor
#define trigPin 4

#define buzzerPin 5     // Buzzer

#define DHT11_PIN 6     // DHT

#define LED_RED 7       // Card Reader
#define LED_GREEN 8
#define RST_PIN 9
#define SS_PIN 10


byte readCard[4];
String tag_UID = "1B5E4FD3";        // Unique ID of our chip
String tagID = "";
MFRC522 mfrc522(SS_PIN, RST_PIN); 
void setup()
{
  pinMode(LED_GREEN, OUTPUT);       // initialize pin for the Green LED as an output.
  pinMode(LED_RED, OUTPUT);         // initialize pin for the Red LED as an output.
  digitalWrite(LED_GREEN, LOW);     // turn the Green LED off 
  digitalWrite(LED_RED, HIGH);      // turn the Red LED on (Making the initial state locked)
  
  Serial.begin(115200);             // Initialize serial communications with the PC
  SPI.begin();    
  mfrc522.PCD_Init();

  pinMode(buzzerPin, OUTPUT);       // initialize pin for the Buzzer as an output.
  pinMode(trigPin, OUTPUT);         // initialize output of the motion sensor
  pinMode(echoPin, INPUT);          // initialize input of the motion sensor
}

void buzz(int d){                   // Function sounds the Buzzer for (d) amount of time
  digitalWrite(buzzerPin, HIGH);
  delay(d);
  digitalWrite(buzzerPin, LOW);
}

int oldDistance = 0;                // Variable required for motion-detection logic

void loop()
{
  // Card reader:
  while (readID())
  {
    if (tagID == tag_UID)               // Switching the lights and playing sound
    {                                   // if the correct key is detected
      buzz(100);
      digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));  
      digitalWrite(LED_RED, !digitalRead(LED_RED));  
            
      oldDistance = 0;
    }
    else                                // If the key is wrong, play a diferent sound
    {
      buzz(50);
      delay(50);
      buzz(50);
      delay(50);
      buzz(50);
    }
  }

  //DHT:
  int chk = DHT.read11(DHT11_PIN);        // Initialize the DHT sensor
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);        // Print out the temperature
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);           // Print out the humidity
  Serial.println("-------------");
  
  // Fire Alarm if the temp. is above 25 deg. (arbitrary value)
  if(DHT.temperature >= 25) buzz(1500); 

  delay(500);


  //distance
  if (digitalRead(LED_RED))               // Only if the door is locked
  { 
    float duration, distance;
    
    digitalWrite(trigPin, LOW); 
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    duration = pulseIn(echoPin, HIGH);    // Pulsing a ray for a shot duration
    distance = (duration / 2) * 0.0344;   // Mesuring the distance based on speed of light
    distance = int(distance);           

    if (distance < oldDistance - 2){      // Logic for detecting movement
      digitalWrite(buzzerPin, HIGH);      // Intruder alarm: 
    }                                     // if the distance in front of the sensor changed
    else{
      oldDistance = distance;
    }
    delay(200);
  }
}
  // Function for reading the Unique ID of the key-card
  boolean readID()
  {
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
      return false;                       //Check if a new tag is detected
    }
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
      return false;                       //Check if a new tag is readable
    }
    tagID = "";
    for ( uint8_t i = 0; i < 4; i++)      // Read the 4 byte UID
    {
      tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); 
      // Convert the UID to a single String
    }
    tagID.toUpperCase();
    mfrc522.PICC_HaltA();                   // Stop reading
    return true;                            // Return if succsessful.
  }