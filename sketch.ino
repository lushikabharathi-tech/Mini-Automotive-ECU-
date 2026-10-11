const byte rpmPin = A0;
const byte tempPin = A1;
const byte batteryPin = A2;

const byte buzzerPin = 8;
const byte fanLedPin = 9;
const byte warningLedPin = 10;
const byte statusLedPin = 11;

const byte ignitionButtonPin = 2;
const byte faultButtonPin = 3;

bool ignitionOn = false;
bool manualFault = false;

bool lastIgnitionReading = HIGH;
bool lastFaultReading = HIGH;
bool ignitionHandled = false;
bool faultHandled = false;

unsigned long lastIgnitionChange = 0;
unsigned long lastFaultChange = 0;
const unsigned long debounceMs = 40;

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(fanLedPin, OUTPUT);
  pinMode(warningLedPin, OUTPUT);
  pinMode(statusLedPin, OUTPUT);

  pinMode(ignitionButtonPin, INPUT_PULLUP);
  pinMode(faultButtonPin, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Mini Automotive ECU Started");
  Serial.println("D2: Ignition toggle | D3: Fault toggle");
}

void loop() {
  readButtons();

  int rpmRaw = analogRead(rpmPin);
  int tempRaw = analogRead(tempPin);
  int batteryRaw = analogRead(batteryPin);

  int rpm = map(rpmRaw, 0, 1023, 0, 6000);
  int temperatureC = map(tempRaw, 0, 1023, 0, 150);
  float batteryVoltage =
      batteryRaw * (5.0 / 1023.0) * 3.0;

  bool overTemperature = temperatureC >= 100;
  bool overSpeed = rpm >= 5000;
  bool lowBattery = batteryVoltage < 10.5;

  bool faultDetected =
      manualFault || overTemperature ||
      overSpeed || lowBattery;

  bool fanRequested =
      ignitionOn && temperatureC >= 80;

  digitalWrite(fanLedPin, fanRequested);
  digitalWrite(warningLedPin,
               ignitionOn && faultDetected);
  digitalWrite(statusLedPin,
               ignitionOn && !faultDetected);

  if (ignitionOn && faultDetected) {
    tone(buzzerPin, 2000);
  } else {
    noTone(buzzerPin);
  }

  Serial.print("Ignition: ");
  Serial.print(ignitionOn ? "ON" : "OFF");
  Serial.print(" | RPM: ");
  Serial.print(rpm);
  Serial.print(" | Temp: ");
  Serial.print(temperatureC);
  Serial.print(" C | Battery: ");
  Serial.print(batteryVoltage, 1);
  Serial.print(" V | Status: ");

  if (!ignitionOn) {
    Serial.println("IGNITION OFF");
  } else if (faultDetected) {
    Serial.println("FAULT DETECTED");
  } else if (temperatureC >= 80) {
    Serial.println("HIGH TEMP - FAN REQUESTED");
  } else {
    Serial.println("NORMAL");
  }

  delay(300);
}

void readButtons() {
  unsigned long now = millis();

  bool ignitionReading =
      digitalRead(ignitionButtonPin);

  if (ignitionReading != lastIgnitionReading) {
    lastIgnitionChange = now;
    lastIgnitionReading = ignitionReading;
  }

  if (now - lastIgnitionChange > debounceMs &&
      ignitionReading == LOW && !ignitionHandled) {
    ignitionOn = !ignitionOn;
    ignitionHandled = true;
  }

  if (ignitionReading == HIGH) {
    ignitionHandled = false;
  }

  bool faultReading = digitalRead(faultButtonPin);

  if (faultReading != lastFaultReading) {
    lastFaultChange = now;
    lastFaultReading = faultReading;
  }

  if (now - lastFaultChange > debounceMs &&
      faultReading == LOW && !faultHandled) {
    manualFault = !manualFault;
    faultHandled = true;
  }

  if (faultReading == HIGH) {
    faultHandled = false;
  }
}