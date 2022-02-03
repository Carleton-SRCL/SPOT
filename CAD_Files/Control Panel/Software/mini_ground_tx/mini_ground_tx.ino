// Arduino digital pins
#define BUTTON_PIN  9
#define kill_signal 13
#define restore_signal 20

void setup()
{
  pinMode(BUTTON_PIN, INPUT); // pin that e-stop button is connected to
  Serial.begin(1200);  // Hardware supports up to 2400, but 1200 gives longer range
}

void loop() // looping forever
{
  if (digitalRead(BUTTON_PIN)==LOW) // if e-stop signal received
  {
    if (Serial.availableForWrite() > 60) // only write if serial pipeline is almost clear (to avoid backlog)
    {
      writeUInt(kill_signal); // SEND E-STOP SIGNAL!!!
    }
  }
  else // if e-stop is not pressed
  {
    if (Serial.availableForWrite() > 60) // if serial pipeline is almost clear
    {
      writeUInt(restore_signal); // send resume signal
    }
  }
}


// sends the signal when requested (not written by Kirk)
void writeUInt(unsigned int val)
{
#define NETWORK_SIG_SIZE 3

#define VAL_SIZE         2
#define CHECKSUM_SIZE    1
#define PACKET_SIZE      (NETWORK_SIG_SIZE + VAL_SIZE + CHECKSUM_SIZE)

// The network address byte and can be change if you want to run different devices in proximity to each other without interfearance
#define NET_ADDR 5

const byte g_network_sig[NETWORK_SIG_SIZE] = {0x8F, 0xAA, NET_ADDR};  // Few bytes used to initiate a transfer
  byte checksum = (val/256) ^ (val&0xFF);
  Serial.write(0xF0);  // This gets reciever in sync with transmitter
  Serial.write(g_network_sig, NETWORK_SIG_SIZE);
  Serial.write((byte*)&val, VAL_SIZE);
  Serial.write(checksum); //CHECKSUM_SIZE
}
