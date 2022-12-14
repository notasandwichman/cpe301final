#define RDA 0x80
#define TBE 0x20
#define MAX_STEPS 64
#include <dht.h>
#include <LiquidCrystal.h>

const int rs = 3, en = 4, d4 = 14, d5 = 15, d6 = 16, d7 = 17;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

volatile unsigned char* portB = (unsigned char *)0x25;
volatile unsigned char* ddrB = (unsigned char *)0x24;
volatile unsigned char* portE = (unsigned char *)0x2E;
volatile unsigned char* ddrE = (unsigned char *)0x2D;
volatile unsigned char* pinE = (unsigned char *)0x2C;

volatile unsigned char* ddrH = (unsigned char *)0x101;
volatile unsigned char* portH = (unsigned char *)0x102;
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

dht DHT;

void setup(){
  U0init(9600);
  adcinit();
  *ddrB |= 0xF0;
  *ddrH |= 0x10;
  lcd.begin(16, 2);
}
void loop(){
  unsigned int waterLevel = adcread(1);
  unsigned int potentiometer = adcread(3);
  if (potentiometer < 600 && potentiometer > 50){ //potentiometer control of fan direction
    turnClockwise();
  } else if (potentiometer > 600){
    turnCounterClockwise();
  }
  if (waterLevel < 150){//waterLevel indicator
      lcd.setCursor(0,0);
      lcd.print("!!! Water Level !!!");
      lcd.setCursor(0, 1);
      lcd.print("!!!  Low  !!!");
  } else {
      lcd.setCursor(0,0);
      lcd.print("!!! Water Level !!!");
      lcd.setCursor(0, 1);
      lcd.print("!!!  Good  !!!");
  }
}

void U0init(int U0baud){
  unsigned long fclock = 16000000;
  unsigned int tbaud = (fclock/16/U0baud-1);
  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0  = tbaud;  
}
void print_int(unsigned int out_num){
  // clear a flag (for printing 0's in the middle of numbers)
  unsigned char print_flag = 0;
  // if its greater than 1000
  if(out_num >= 1000)
  {
    // get the 1000's digit, add to '0' 
    putChar(out_num / 1000 + '0');
    // set the print flag
    print_flag = 1;
    // mod the out num by 1000
    out_num = out_num % 1000;
  }
  // if its greater than 100 or we've already printed the 1000's
  if(out_num >= 100 || print_flag)
  {
    // get the 100's digit, add to '0'
    putChar(out_num / 100 + '0');
    // set the print flag
    print_flag = 1;
    // mod the output num by 100
    out_num = out_num % 100;
  } 
  // if its greater than 10, or we've already printed the 10's
  if(out_num >= 10 || print_flag)
  {
    putChar(out_num / 10 + '0');
    print_flag = 1;
    out_num = out_num % 10;
  } 
  // always print the last digit (in case it's 0)
  putChar(out_num + '0');
  // print a newline
  putChar('\n');
}

void adcinit(){
  // setup the A register
  *my_ADCSRA |=  0b10000000;// set bit   7 to 1 to enable the ADC
  *my_ADCSRA &=  0b11011111;// clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &=  0b11110111;// clear bit 3 to 0 to disable the ADC interrupt
  *my_ADCSRA &=  0b11111000;// clear bit 2:0 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &=  0b11110111;// clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &=  0b11111000;// clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &=  0b01111111;// clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |=  0b01000000;// set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &=  0b11011111;// clear bit 5 to 0 for right adjust result
  *my_ADMUX  &=  0b11100000;// clear bit 4-0 to 0 to reset the channel and gain bits
}
unsigned int adcread(unsigned char adc_channel_num){
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}
unsigned char U0kbhit(){
  return *myUCSR0A & RDA;
}
unsigned char getChar(){
  return *myUDR0;
}
void putChar(unsigned char U0pdata){
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}

void turnCounterClockwise(){
  *portB |= 0x10;
  *portB &= 0x10;
  delay(2);
  *portB |= 0x20;
  *portB &= 0x20;
  delay(2);
  *portB |= 0x40;
  *portB &= 0x40;
  delay(2);
  *portB |= 0x80;
  *portB &= 0x80;
  delay(2);
}

void turnClockwise(){
  *portB |= 0x80;
  *portB &= 0x80;
  delay(2);
  *portB |= 0x40;
  *portB &= 0x40;
  delay(2);
  *portB |= 0x20;
  *portB &= 0x20;
  delay(2);
  *portB |= 0x10;
  *portB &= 0x10;
  delay(2);
}
