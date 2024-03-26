#define BLYNK_PRINT Serial
#define TINY_GSM_MODEM_SIM800
#define BLYNK_HEARTBEAT 8

#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
#include <DHT.h>

char auth[] = "ke_-uSE5Xvw7YTAlHe2WZv6tKW8g7nMS";
char apn[]  = "orangeinternet";
char user[] = "";
char pass[] = "";

#define temperatureHumiditySensor 4
#define fireSensor 5
#define smokeSensor A1
#define gasSensor A0
#define gasValve 8
#define acDevice1 6
#define siren 7

#define DHTTYPE DHT11
SoftwareSerial SerialAT(3, 2); //RX, TX
TinyGsm modem(SerialAT);
DHT dht(temperatureHumiditySensor, DHTTYPE);

WidgetLCD stateLCD(V0);
WidgetLED fireLED(V3);
WidgetLED smokeLED(V4);
WidgetLED gasLED(V5);
BlynkTimer timer;

bool safetyStatus = false, resetStatus = false;
int fireValue, smokeValue, gasValue;

void setup()
{
  pinMode(fireSensor, INPUT);
  pinMode(gasValve, OUTPUT);
  pinMode(acDevice1, OUTPUT);
  pinMode(siren, OUTPUT);
  digitalWrite(gasValve, HIGH);
  digitalWrite(acDevice1, HIGH);
  digitalWrite(siren, LOW);
  Serial.begin(115200);
  delay(10);
  SerialAT.begin(9600);
  delay(3000);
  modem.restart();
  Blynk.begin(auth, modem, apn, user, pass);
  dht.begin();
  timer.setInterval(4000L, readTemperatureHumidity);
  stateLCD.clear();
  stateLCD.print(0, 0, "IoT Fire Control");
  stateLCD.print(0, 1, "State is SAFE.");
  fireLED.off();
  smokeLED.off();
  gasLED.off();
}

void loop()
{
  Blynk.run();
  timer.run();
  readSensors();
}

void readTemperatureHumidity()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) 
  {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
}

void readSensors()
{
  fireValue = digitalRead(fireSensor);
  if (fireValue == 1)
  {
    safetyStatus = true;
    resetStatus = true;
    fireLED.on();
    Blynk.email("IoT Fire Control", "WARNING: Fire Incident Detected!!");
    digitalWrite(gasValve, LOW);
    digitalWrite(acDevice1, LOW);
    digitalWrite(siren, HIGH);
    stateLCD.clear();
    stateLCD.print(0, 0, "IoT Fire Control");
    stateLCD.print(0, 1, "FIRE Detected!!");
  }

  smokeValue = analogRead(smokeSensor);
  Serial.println(smokeValue);
  if (smokeValue > 400)
  {
    safetyStatus = true;
    resetStatus = true;
    smokeLED.on();
    Blynk.email("IoT Fire Control", "WARNING: Smoke Leakage Detected!!");
    digitalWrite(gasValve, LOW);
    digitalWrite(acDevice1, LOW);
    digitalWrite(siren, HIGH);
    stateLCD.clear();
    stateLCD.print(0, 0, "IoT Fire Control");
    stateLCD.print(0, 1, "SMOKE Detected!!");
  }

  /*gasValue = analogRead(gasValue);
  Serial.println(gasValue);
  if (gasValue < 200)
  {
    safetyStatus = true;
    resetStatus = true;
    gasLED.on();
    Blynk.notify("WARNING: Gas Leakage Detected!!");
    Blynk.email("IoT Fire Control", "WARNING: Gas Leakage Detected!!");
    digitalWrite(gasValve, LOW);
    digitalWrite(acDevice1, LOW);
    //digitalWrite(acDevice2, LOW);
    digitalWrite(siren, HIGH);
    stateLCD.clear();
    stateLCD.print(0, 0, "IoT Fire Control");
    stateLCD.print(0, 1, "GAS Detected!!");
  }*/
}

BLYNK_WRITE(V6)
{
  int pinValue = param.asInt();
  if (pinValue == 1)
  {
    if (safetyStatus == true)
    {
      safetyStatus = false;
      resetStatus = false;
      fireLED.off();
      smokeLED.off();
      gasLED.off();
      digitalWrite(gasValve, HIGH);
      digitalWrite(acDevice1, HIGH);
      digitalWrite(siren, LOW);
      stateLCD.clear();
      stateLCD.print(0, 0, "IoT Fire Control");
      stateLCD.print(0, 1, "State is SAFE.");
    }
  }
}
