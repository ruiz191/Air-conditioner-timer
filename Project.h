#include <16F873A.h>
#device adc=10

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES HS                       //Crystal osc > 4mhz
#FUSES PUT                      //Power Up Timer
#FUSES NOPROTECT                //Code can be read
#FUSES NODEBUG                  //No Debug mode for ICD
#FUSES BROWNOUT                 //Reset when brownout detected
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOCPD                    //No EE protection
#FUSES NOWRT                    //Program memory not write protected

#use delay(clock=16000000)


#define PIN_EN PIN_C7
#define PIN_RS PIN_C6


#define PIN_BEEP PIN_A1
#define PIN_FAN1 PIN_C2
#define PIN_FAN2 PIN_C3
#define PIN_FAN3 PIN_C4
#define PIN_COMP PIN_C5


#define PIN_COOL PIN_B0
#define PIN_TMER PIN_B1
#define PIN_FSPD PIN_B2
#define PIN_TPDN PIN_B3
#define PIN_TPUP PIN_C0
#define PIN_PWER PIN_C1
#define PIN_LOWV PIN_A4

#define PIN_REMT PIN_A5



#define HOLD_DELAY 0xFF        //tiempo que debe ser mantenido en HOLD los botones
#define WAIT_TIME 1800         //tiempo espera del compresor 1800 aprox 3min0seg  100 = 5s
#define LOOP_TIME 50           //tiempo que espera para rotar display (50 = aprox 5s)

#define HYST 5                 //banda muerta en decimas de gradosC para el control ON-OFF

//#define STEP_MODE 0x11         //single phase
//#define STEP_MODE 0x33         //dual phase


void SampleADC(void);
void Config(void);
void WriteCmd(int8 Cmd);
void WriteC(int8 Cmd);
void DspInt16(int16 Temp);
void RefreshLCD(void);
void WriteStr(int8 Display);
void DspTime(int1 time);
void SaveConfig(void);
void LoadConfig(void);

#define Clock 0
#define Timer 1

#define ON 1
#define OFF 0


const char STRINGS[12][8]={"Temp   ", "Time   ", "Timer  ",  "Power  ","Fan    ","Cool   ","Adj Tmp","Adj Clk","Adj Tmr","Over V.","     ON","    OFF"};

