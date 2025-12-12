#include <Arduino.h>
#include <Adafruit_ADS7830.h>
#include <DmxOutput.h>
#include <DmxOutput.pio.h>

Adafruit_ADS7830 adc0, adc1, adc2;
DmxOutput dmx_out;

uint8_t universe[24 + 1]; 
uint8_t fade[] = {0,0,0};

typedef struct Color {
  uint8_t rgbw[4];
} Color;
Color a,b,c,d;
static Color x = {0,0,0,0};
Color *crayon_box[5] = {&x, &a, &b, &c, &d};

typedef struct Fixture {
  Color *letter;
  uint8_t *fade;
} Fixture;
Fixture lights[6];

uint8_t scale8(uint8_t i, uint8_t scale){ //thank you fastLED :)
  return (((uint16_t)i) * (1 + (uint16_t)(scale))) >> 8;
}

void output(){
  for(uint8_t i=0; i<6; i++){
    for(uint8_t j=0; j<4; j++){
      universe[1 + 4*i + j] = *lights[i].fade==2 ? lights[i].letter->rgbw[j] : scale8(lights[i].letter->rgbw[j], *lights[i].fade);
    }
  }
  dmx_out.write(universe, 24+1);
}

void update_fixture(){
  fade[2] = adc2.readADCsingle(6); //master fade update
  fade[1] = adc2.readADCsingle(7);
  for(uint8_t i=0; i<6; i++){
    lights[5-i].letter = crayon_box[map(adc2.readADCsingle(i), 0, 255, 4, 0)]; //rotary switches
    pinMode(i+4, INPUT_PULLUP); //mod R to check floating
    bool pup = digitalRead(i+4);
    pinMode(i+4, INPUT_PULLDOWN);
    lights[5-i].fade = pup==digitalRead(i+4) ? &fade[pup+1] : &fade[0]; //set fades per fixture
  }
}

void update_colors(){
  for(uint8_t i=0; i<4; i++){
    a.rgbw[i] = 255 - adc0.readADCsingle(i);
    b.rgbw[i] = 255 - adc0.readADCsingle(i+4);
    c.rgbw[i] = 255 - adc1.readADCsingle(7-i);
    d.rgbw[i] = 255 - adc1.readADCsingle(3-i);
  }
}

void setup() {
  Serial.begin(115200);
  adc0.begin(0x48);
  adc1.begin(0x49);
  adc2.begin(0x4A);
  dmx_out.begin(3);
}

void loop() {
  if(!dmx_out.busy()){
    update_colors();
    update_fixture();
    output();
  }
}