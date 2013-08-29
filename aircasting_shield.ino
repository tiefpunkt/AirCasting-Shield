#include "DHT.h"
#include "SimpleTimer.h"
#include "SoftwareSerial.h"

#define DHTPIN 2    
#define DHTTYPE DHT22 //DHT11, DHT21, DHT22

#define RELAIS 3
#define MQ_7 0
#define MQ_135 1

#include <CS_MQ7.h>

CS_MQ7 MQ7(3, 13);

DHT dht(DHTPIN, DHTTYPE);

SimpleTimer timer1;

SoftwareSerial bluetooth(6, 7); 

void setup()
{
  // Setup relais for MQ7
  pinMode(RELAIS, OUTPUT);
  digitalWrite(RELAIS, LOW);

  // Setup DHT22
  dht.begin();

  // Setup Serial connection
  Serial.begin(9600);

  // Setup Bluetooth Adapter
  bluetooth.begin(38400);
  bluetooth.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  bluetooth.print("\r\n+STNA=AirCastingSensor\r\n"); //set the bluetooth name as "SeeedBTSlave"
  bluetooth.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  bluetooth.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  bluetooth.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  delay(2000); // This delay is required.
  bluetooth.flush();


  timer1.setInterval(5000, processSensors);
}
 
void loop() {
  MQ7.CoPwrCycler();
  timer1.run();
}

void processSensors() {
  processDHT22();
  processMQ7();
  processMQ135();
  Serial.println();
}

void processDHT22() {
  float h = dht.readHumidity();     //Luftfeuchte auslesen
  float t = dht.readTemperature();  //Temperatur auslesen
 
  // Prüfen ob eine gültige Zahl zurückgegeben wird. Wenn NaN (not a number) zurückgegeben wird, dann Fehler ausgeben.
  if (isnan(t) || isnan(h)) {
    Serial.println("DHT22 konnte nicht ausgelesen werden");
  } else {
    Serial.print("Luftfeuchte: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperatur: ");
    Serial.print(t);
    Serial.print(" C\t");
    
    bluetooth.print(h);
    bluetooth.print(";AirCastingShieldRev1;DHT22H;Humidity;RH;percent;%;0;25;50;75;100");
    bluetooth.print("\n");
    bluetooth.print(t);
    bluetooth.print(";AirCastingShieldRev1;DHT22T;Temperature;C;degrees Celsius;C;0;10;15;20;25");
    bluetooth.print("\n");
  }
}

void processMQ7() {
  if(MQ7.CurrentState() == LOW){   //we are at 1.4v, read sensor data!
    int sensor1 = analogRead(MQ_7);
    float voltage = sensor1 * 5.0 / 1024;
    Serial.print("MQ7: ");
    Serial.print(voltage);
    Serial.print("V\t");
    
    float CO = map(sensor1, 0 , 1023, 0.0, 100.0);
    bluetooth.print(CO);
    bluetooth.print(";AirCastingShieldRev1;MQ7;CO Gas;CO;response indicator;RI;0;25;50;75;100");
    bluetooth.print("\n");
    
  } else {
    Serial.print("MQ7: HEATING\t");
  }
}

void processMQ135() {
    int sensor2 = analogRead(MQ_135);
    float voltage = sensor2 * 5.0 / 1024;
    Serial.print("MQ135: ");
    Serial.print(voltage);
    Serial.print("V");
    
    float VOC = map2(sensor2, 0 , 1023, 0.0, 100.0);
    bluetooth.print(VOC);
    bluetooth.print(";AirCastingShieldRev1;MQ135;VOC;VOC;response indicator;RI;0;25;50;75;100");
    bluetooth.print("\n");
}

float map2(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
