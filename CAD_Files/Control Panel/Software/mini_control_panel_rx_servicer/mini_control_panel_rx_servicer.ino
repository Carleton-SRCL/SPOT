// Arduino digital pins
#define good 9
#define bad 8
#define SWITCH_PIN  6
#define kill_signal 13
#define restore_signal 20
#define bat_status A2
#define bat_light 7

int last;
int timer;
float time_out = 5; // time out [seconds] how long to wait if no signal is received before freaking out
int start = 0;
int bat;
const int pulse_time = 5000;// how many milliseconds to pulse red LED when no signal is received. should equal time_out
const int low_batt_value = 475;// 23.8 V on the battery
const int critical_battery = 466; // corresponds to 23.3 V on battery (MUST CHARGE BATTERY)

void setup()
{
  pinMode(bat_light, OUTPUT); // low battery LED
  pinMode(good, OUTPUT); // led for good e-stop status
  pinMode(bad, OUTPUT); //led for bad e-stop status
  pinMode(SWITCH_PIN, OUTPUT); // pin that MOSFET is connected to
  digitalWrite(SWITCH_PIN, LOW); // Initializing power to be off until "ready" signal is received from ground station
  digitalWrite(bad, HIGH); // initializing bad LED to be on
  digitalWrite(good, LOW); // turning off good e-stop LED
  digitalWrite(bat_light, LOW); // low battery light off
  Serial.begin(1200);  // Hardware supports up to 2400, but 1200 gives longer range
  delay(250);
}

void loop() // looping forever
{

  if (analogRead(bat_status) < low_batt_value) // if battery is low
  {
    digitalWrite(bat_light, HIGH); // illuminate low battery LED
  }
  if (analogRead(bat_status) < critical_battery) // if battery is critically low
  {
    digitalWrite(SWITCH_PIN, LOW); // kill power
    digitalWrite(good, LOW); // signal e-stop is triggered
    digitalWrite(bad, HIGH);

    while (true) // looping forever until battery gets charged
    {
      digitalWrite(bat_light, HIGH);
      delay(250);
      digitalWrite(bat_light, LOW);
      delay(250);
    }
  }

  int signal = readUInt(true); // read signal from receiver chip

  if (signal == kill_signal || signal == 0 ) // Check to see if we got the e-stop signal or no signal at all
  {
    digitalWrite(SWITCH_PIN, LOW); // if we got the E-STOP signal (13), kill power!!!
    digitalWrite(good, LOW);
    digitalWrite(bad, HIGH); // alert the operator

    while (signal != restore_signal) // while waiting for restore signal
    {
      if (analogRead(bat_status) < low_batt_value) // check if battery is low
      {
        digitalWrite(bat_light, HIGH); // write it as low if low
      }
      if (analogRead(bat_status) < critical_battery) // if battery is critically low
      {
        digitalWrite(SWITCH_PIN, LOW); // kill power
        digitalWrite(good, LOW); // signal e-stop is triggered
        digitalWrite(bad, HIGH);

        while (true) // looping forever until battery gets charged
        {
          digitalWrite(bat_light, HIGH);
          delay(250);
          digitalWrite(bat_light, LOW);
          delay(250);
        }

      }

      if (signal == 0) // if we got no signal, pulse the BAD LED
      {
        digitalWrite(bad, LOW);
        delay(pulse_time);
        digitalWrite(bad, HIGH);
      }
      signal = readUInt(true); // takes up to 5 seconds to execute if no signal comes in

    }// Keep power killed until "RESTORE" signal is received (20)

  }
  digitalWrite(SWITCH_PIN, HIGH); // Since RESTORE signal was received, allow power to flow again
  digitalWrite(good, HIGH);
  digitalWrite(bad, LOW);
}


// Receives an unsigned int over the RF network (not written by Kirk)
unsigned int readUInt(bool wait)
{

#define NETWORK_SIG_SIZE 3
#define VAL_SIZE         2
#define CHECKSUM_SIZE    1
#define PACKET_SIZE      (NETWORK_SIG_SIZE + VAL_SIZE + CHECKSUM_SIZE)

  // The network address byte and can be change if you want to run different devices in proximity to each other without interfearance
#define NET_ADDR 5

  const byte g_network_sig[NETWORK_SIG_SIZE] = {0x8F, 0xAA, NET_ADDR};  // Few bytes used to initiate a transfer

  int pos = 0;          // Position in the network signature
  unsigned int val;     // Value of the unsigned int
  byte c = 0;           // Current byte

  if ((Serial.available() < PACKET_SIZE) && (wait == false))
  {
    return 0;
  }
  unsigned long start = millis();
  while (pos < NETWORK_SIG_SIZE)
  {
    while (Serial.available() == 0) // Wait until something is avalible
    {
      if (millis() - start > time_out * 1000)
      {
        return 0;
      }
    }
    c = Serial.read();

    if (c == g_network_sig[pos])
    {
      if (pos == NETWORK_SIG_SIZE - 1)
      {
        byte checksum;
        unsigned long start = millis();
        while (Serial.available() < VAL_SIZE + CHECKSUM_SIZE) // Wait until something is avalible
        {
          if (millis() - start > time_out * 1000)
          {
            return 0;
          }
        }
        val      =  Serial.read();
        val      += ((unsigned int)Serial.read()) * 256;
        checksum =  Serial.read();

        if (checksum != ((val / 256) ^ (val & 0xFF)))
        {
          // Checksum failed
          pos = -1;
        }
      }
      ++pos;
    }
    else if (c == g_network_sig[0])
    {
      pos = 1;
    }
    else
    {
      pos = 0;
      if (!wait)
      {
        return 0;
      }
    }
    if (millis() - start > time_out * 1000) // if we keep getting errorous data for more than the timeout
    {
      return 0;
    }
  }
  return val;
}
