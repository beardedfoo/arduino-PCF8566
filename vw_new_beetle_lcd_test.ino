/*

This is a program to test connectivity with the LCD display module of the
VW "New Beetle" instrument cluster. It was tested and developed with an
instrument cluster from a 1999 model year, but should work with all '98-'11
intrument cluster LCD modules.

The instrument cluster of the New Beetle is separated into two boards which
have an I2C bus connecting between them. One board is the primary board with
the LED's and gauges on it, and the other controls only the odometer/tripometer
LCD at the base of the speedometer.

The LCD module of the instrument cluster is a well-packaged NXT PFC8566T IC.
There are 6-pins that connect the LCD module to the main board of the instrument
cluster. When viewed from the IC-side of the board, with the pins up the pins are
as follows:
                              
  (Uncomfirmed pinouts, models may vary. Verify before use!!)
      
                                  o   o   o
        |=====|_|====|            |   |   |
        |            |      Pins: A   B   C
        |     P      |
        |     F      |
        |     C      |
        |     8      |
        |     5      |
        |     6      |      Pins: D   E   F
        |     6      |            |   |   |
        |============|            o   o   o

Pin A = SCL
Pin B = SDA
Pin C = +5 Volts
Pin D = Ground
Pin E = +12 Volts (optional, for LCD backlights only)
Pin F = Ground 

The I2C communication between the boards follows the specifications
in the PFC8566 data sheet, with the notable exception that the I2C
slave address differs from the PFC8566 specifications.

To run this on an arduino connect the SCL, SDA, 5v, and GND lines from
the arduino to the LCD module. To run the unit while uninstalled this
can be achieved with some female to male jumper cables.

The LCD module can also be ran by an arduino while installed in the
instrument cluster if you bend SDA and SCL pins such that they no longer
insert into the main board of the cluster and solder some wires to them.
To pass the I2C wires to the back of the cluster drill some holes through the
main board of the instrument cluster and through the white backing plate
of the cluster. The ground and +12v must be shared between the arduino and
the cluster gauge.

+12v and ground can be obtained/supplied at the blue connector on the back of
the instrument cluster. Pin 23 on the blue connector is +12v and pin 24 is
ground. The instrument cluster has a builtin +5v voltage regulator.

*/

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
