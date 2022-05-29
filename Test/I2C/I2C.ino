#include <ESP8266WiFi.h>

// ESP8266 I2C
#define GPIO_LED 0
#define GPIO_I2C_SCL 5
#define GPIO_I2C_SDA 4

#define I2C_CLOCK_PULSE 5
#define I2C_READ HIGH
#define I2C_WRITE LOW

// BME280 https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout
#define BME280_REGISTER_TEMPDATA 0xFA
#define BME280_REGISTER_DIG_T1 0x88
#define BME280_REGISTER_CHIPID 0xD0

void setup() {
  Serial.begin(115200);
  
  pinMode(GPIO_LED, OUTPUT);
  pinMode(GPIO_I2C_SCL, OUTPUT);
  pinMode(GPIO_I2C_SDA, INPUT);
  
  Serial.print("Setup\n");
}

void loop() {
  delay(1000);
  digitalWrite(GPIO_LED, HIGH);

  uint8_t MoistureAddress = 0x36;
  uint8_t BME280Address = 0x77;
  uint8_t OhterAddress = 0x12;
  
  int MoistureValue = ReadTemp(MoistureAddress);
  int OhterValue = ReadTemp(OhterAddress);
  int BME280Value = ReadTemp(BME280Address);

  digitalWrite(GPIO_LED, LOW);
}

void ClockPulse()
{
  digitalWrite(GPIO_I2C_SCL, LOW);
  delay(I2C_CLOCK_PULSE);
  digitalWrite(GPIO_I2C_SCL, HIGH);
  delay(I2C_CLOCK_PULSE);
  digitalWrite(GPIO_I2C_SCL, LOW);
  delay(I2C_CLOCK_PULSE);
}

uint16_t ReadTemp(uint8_t Address)
{
  I2CStart();

  // Address BME280
  // Should this be read or write if I need to specify the register?
  if (I2C_Address(Address, I2C_WRITE))
  {
    Serial.print("Failed to Acknowledge AddressDevice ");
    Serial.print(Address, HEX);
    Serial.print("\n");
    return 0;
  }
  Serial.print("AddressDevice Acknowledged ");     
  Serial.print(Address, HEX);
  Serial.print("\n");

  // Check The Chip ID
  if (!WriteValue(BME280_REGISTER_CHIPID))
  {
    Serial.print("Failed to Write Register ");
    Serial.print(BME280_REGISTER_CHIPID, HEX);
    Serial.print("\n");
    return 0;
  }
  Serial.print("Wrote Register BME280_REGISTER_TEMPDATA ");     
  Serial.print(BME280_REGISTER_CHIPID, HEX);
  Serial.print("\n");
  
  uint16_t ChipID = ReadValue();
  Serial.print("ChipID 0x");
  Serial.print(ChipID, HEX);
  Serial.print(", expected 0x60");
  if (ChipID != 0x60)
  {
    return 0;
  }
  
  uint16_t Temp = ReadValue();
  
  Serial.print("Read:  ");     
  Serial.print(Temp);
  Serial.print("\n");

  return Temp;
}

void I2CStart()
{
  pinMode(GPIO_I2C_SDA, OUTPUT);
  
  // Start Condition: Data Drops Low While Clock Still High
  digitalWrite(GPIO_I2C_SCL, HIGH);
  digitalWrite(GPIO_I2C_SDA, HIGH);
  delay(I2C_CLOCK_PULSE);
  digitalWrite(GPIO_I2C_SDA, LOW);
  delay(I2C_CLOCK_PULSE);
}

void I2CStop()
{
  pinMode(GPIO_I2C_SDA, OUTPUT);
  
  // Stop Condition: Data Drops Low While Clock Still High
  digitalWrite(GPIO_I2C_SCL, HIGH);
  digitalWrite(GPIO_I2C_SDA, HIGH);
  delay(I2C_CLOCK_PULSE);
}


bool I2C_Address(uint8_t Address, int ReadOrWrite)
{  
  pinMode(GPIO_I2C_SDA, OUTPUT);

  // 7 bit Address
  for (uint8_t i = 0; i < 7; i++)
  {
    if ((Address >> i) & 1 == 1)
    {
      digitalWrite(GPIO_I2C_SDA, HIGH);
    }
    else
    {
      digitalWrite(GPIO_I2C_SDA, LOW);
    }
    ClockPulse();
  }
  Serial.print("\n"); 

  // Read or Write
  digitalWrite(GPIO_I2C_SDA, ReadOrWrite);
  ClockPulse();

  // Recieve Acknowledge
  pinMode(GPIO_I2C_SDA, INPUT);
  int Acknowledge = digitalRead(GPIO_I2C_SDA);
  ClockPulse();

  return Acknowledge != 0;
}

bool WriteValue(uint8_t Value)
{
  pinMode(GPIO_I2C_SDA, OUTPUT);

  // 8 bit Value
  Serial.print("Write: "); 
  for (uint8_t i = 0; i < 8; i++)
  {
    if ((Value >> i) & 1 == 1)
    {
      Serial.print(1); 
      digitalWrite(GPIO_I2C_SDA, HIGH);
    }
    else
    {
      Serial.print(0); 
      digitalWrite(GPIO_I2C_SDA, LOW);
    }
    ClockPulse();
  }
  Serial.print("\n"); 

  // Read or Write
  digitalWrite(GPIO_I2C_SDA, HIGH);
  ClockPulse();

  // Recieve Acknowledge
  pinMode(GPIO_I2C_SDA, INPUT);
  ClockPulse();
  int Acknowledge = digitalRead(GPIO_I2C_SDA);

  return Acknowledge != 0;
}

uint8_t ReadValue()
{
  uint8_t Value = 0;
  
  pinMode(GPIO_I2C_SDA, INPUT);

  // 8 bit Value
  Serial.print("Read: "); 
  for (uint8_t i = 0; i < 8; i++)
  {
    ClockPulse();
    int PinValue = digitalRead(GPIO_I2C_SDA);
    Serial.print(PinValue);
    Value |= (PinValue == HIGH ? 1 : 0) << i;
  }
  Serial.print("\n"); 

  pinMode(GPIO_I2C_SDA, OUTPUT);
  digitalWrite(GPIO_I2C_SDA, HIGH);
  ClockPulse();
  
  return Value;
}
