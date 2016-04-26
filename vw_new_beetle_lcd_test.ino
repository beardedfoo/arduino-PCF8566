#include <Wire.h>

#define PCF_CONTINUE (1 << 7)

#define PCF_CMD_SET_MODE ((1 << 6))
#define PCF_MODE_LOW_POWER (1 << 4)
#define PCF_MODE_ENABLE (1 << 3)
#define PCF_MODE_HALF_BIAS (1 << 2)
#define PCF_MODE_THIRD_BIAS 0
#define PCF_MODE_MULTIPLEX_4 0 // Multiplex 1:4
#define PCF_MODE_STATIC 1

#define PCF_CMD_SELECT_DEVICE(devNum) (((1 << 6) | (1 << 5)) & (devNum & 0x1f))

#define PCF_CMD_SELECT_BANK ((1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 1))

#define PCF_CMD_LOAD_DATA_POINTER(dp) (0x60 | (dp & 0x1f))

class PCF8566 {
  public:
    int i2cSlave;
  
    PCF8566(int address) {
      i2cSlave = address;
    }
  
    void init() {
      _startMsg();
      _write(PCF_CMD_SET_MODE | PCF_MODE_ENABLE | PCF_MODE_MULTIPLEX_4);
      _endMsg();
    }
    
    void testPattern(unsigned char buf[12]) {
      _startMsg();
      _write(PCF_CMD_SELECT_DEVICE(0) | PCF_CONTINUE);
      _write(PCF_CMD_LOAD_DATA_POINTER(0));
      for (int i = 0; i < 12; i++) {
        _write(buf[i]);
      }
      _endMsg();
    }
    
    void _write(unsigned char msg) {
      //Serial.print("i2c write: ");
      //Serial.println(msg, BIN);
      int wrote = Wire.write(msg);
      //Serial.print("Wrote ");
      //Serial.print(wrote);
      //Serial.println(" bytes");
    }
    
    void _startMsg() {
      //Serial.print("Starting i2c msg to ");
      //Serial.println(i2cSlave, HEX);
      Wire.beginTransmission(i2cSlave);
    }
    
    void _endMsg() {
      int error = Wire.endTransmission();
      if (error != 0) {
        Serial.print("Error endTransmission() == ");
        Serial.println(error);
      }
    }
};

PCF8566 lcd(0x3e);

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned char buf[12];
  for (int enableByte=1; enableByte < 11; enableByte++) {
    Serial.print("Testing byte ");
    Serial.println(enableByte);
    for (int enableBit=0; enableBit < 8; enableBit++) {
      for (int fill=0; fill<12; fill++) {
        buf[fill] = 0x00;
      }
      buf[enableByte] = 1 << enableBit;
  
      lcd.testPattern(buf);
      
      Serial.print("Enabled bit ");
      Serial.println(enableBit);
      while (Serial.read() != '\r') {}
    }
  }
}
