#include "mbed.h"

I2C i2c(P0_10,P0_11);           //sda, scl
SPI spi(P0_14, P0_12, P0_17);   //mosi, miso, sclk
DigitalOut cs(P0_13);

void lcd_init(int adr);     //lcd init func
const int lcd_adr = 0x7C;   //lcd i2c adr 0x7C
void char_disp(int adr, int8_t position, char data);
void val_disp(int adr, int8_t position, int8_t digit,int val);

#define intv 0.25   //interval time (s)
uint8_t buf[4];
uint8_t state;
uint8_t fault;
int16_t it;    //internal temp (12-bit, 0.0625C resolution)
int16_t tc;    //thermocoupler temp (14-bit, 0.25C resolution)
float tc_f;
uint16_t val[2];//for dispaly values

int main(){
    cs=1;
    wait(0.2);
    lcd_init(lcd_adr);
    while(1){
        //SPI read
        cs=0;
        buf[3]=spi.write(0x00);
        buf[2]=spi.write(0x00);
        buf[1]=spi.write(0x00);
        buf[0]=spi.write(0x00);
        cs=1;

        //calc values
        state=buf[0]&0b00001111;
        it=(buf[1]<<4)+(buf[0]>>4);
        fault=buf[2]&0b00000001;
        tc=(buf[3]<<6)+(buf[2]>>2);

        if(tc>=8192){
            tc=((~tc)&0b0011111111111111)+1;
            tc=tc*-1;
        }

        tc_f=tc*0.25;

        if(tc_f>=0){
            char_disp(lcd_adr,0,'+');
        }else if(tc_f<0){
            char_disp(lcd_adr,0,'-');
            tc_f=tc_f*-1;
        }

        if(fault){
            for(int i=0;i<8;++i){
                char_disp(lcd_adr,i,'-');
            }
        }else{
            val[0]=(uint16_t)tc_f;
            val[1]=(uint16_t)((tc_f*100)-val[0]*100);
            val_disp(lcd_adr,1,3,val[0]);
            char_disp(lcd_adr,4,'.');
            val_disp(lcd_adr,5,2,val[1]);
            char_disp(lcd_adr,7,'C');
        }
        wait(intv);
    } 
}

//disp char func
void char_disp(int adr, int8_t position, char data){
    char buf[2];
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    buf[0]='@';
    buf[1]=data;
    i2c.write(adr,buf, 2);
}

//disp val func
void val_disp(int adr, int8_t position, int8_t digit, int val){
    char buf[2];
    char data[4];
    int8_t i;
    buf[0]=0x0;
    buf[1]=0x80+position;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(adr,buf, 2);
    data[3]=0x30+val%10;        //1
    data[2]=0x30+(val/10)%10;   //10
    data[1]=0x30+(val/100)%10;  //100
    data[0]=0x30+(val/1000)%10; //1000
    buf[0]='@';
    for(i=0;i<digit;++i){
        buf[1]=data[i+4-digit];
        i2c.write(adr,buf, 2);
    }
}

//LCD init func
void lcd_init(int adr){
    char lcd_data[2];
    lcd_data[0] = 0x0;
    lcd_data[1]=0x38;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x39;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x14;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x70;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x56;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x6C;
    i2c.write(adr, lcd_data, 2);
    wait(0.2);
    lcd_data[1]=0x38;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x0C;
    i2c.write(adr, lcd_data, 2);
    lcd_data[1]=0x01;
    i2c.write(adr, lcd_data, 2);
}
