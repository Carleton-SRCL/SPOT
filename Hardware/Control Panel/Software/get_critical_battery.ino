#define bat_status A2

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
}

void loop() {
  // put your main code here, to run repeatedly:

  int sensorValue = analogRead(bat_status);
  Serial.println(sensorValue); // this will write the decimal value to the terminal
  delay(1000); // delay 1 s
}
