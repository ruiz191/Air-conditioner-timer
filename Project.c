#include "Project.h"

#use fast_io(A)
#use fast_io(B)
#use fast_io(C)

#define RecargaT1Div 10


static int8 T1Div = RecargaT1Div;
static int8 Speed = 1;
static int16 Temp = 250;
static int16 RefTemp = 250;
static int16 CompTemp = 250;

static int16 RefreshDiv = 0;
static int16 WaitCounter = WAIT_TIME;
static int16 WaitCompCounter = WAIT_TIME;

static int8 Display = 0;
static int8 DivDisp = 0;
static int1 KeyPress = 1;

static int1 Cool = 0;
static int1 TimerEnable = 0;
static int1 Power = 0;
static int1 Comp = 0;
static int1 Wait = 0;
static int1 WaitComp = 0;

static int8 TimerSec = 0;
static int8 TimerMin = 0;
static int8 TimerHou = 1;

static int8 TimerSecSvd = 0;
static int8 TimerMinSvd = 0;
static int8 TimerHouSvd = 6;

static int8 ClockSec = 0;
static int8 ClockMin = 0;
static int8 ClockHou = 12;


//remoto
#define SIZE 9                  //equivale a 9*8=72 transiciones como maximo
#define N_CMD 8                //define el numeros de comandos(teclas) que entiende

const int8 TABLE[N_CMD][SIZE+1] ={  0xBE,0xFF,0xEB,0xAA,0xAA,0xFE,0xFF,0xAB,0x02,'P',  //play(P)
                                    0xBE,0xFF,0xEB,0xAA,0xFE,0xFA,0xAB,0xAF,0x02,'+',  //up(+)
                                    0xBE,0xFF,0xEB,0xAA,0xBA,0xBA,0xEF,0xEF,0x02,'-',  //dn(-)
                                    0xBE,0xFF,0xEB,0xAA,0xBE,0xFF,0xEB,0xAA,0x02,'C',  //zoom(C)
                                    0xBE,0xFF,0xEB,0xAA,0xAA,0xFA,0xFF,0xAF,0x02,'S',  //slow(S)
                                    0xBE,0xFF,0xEB,0xAA,0xAA,0xFF,0xFF,0xAA,0x02,'T',  //goto(T)
                                    0xBE,0xFF,0xEB,0xAA,0xEE,0xFE,0xBB,0xAB,0x02,'t',  //Repeat(t)
                                    0xBE,0xFF,0xEB,0xAA,0xAE,0xFF,0xFB,0xAA,0x02,'F'}; //mark(F)


/*const int8 TABLE[N_CMD][SIZE+1] ={  0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,'P',  //play(P)
                                    0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,'+',  //up(+)
                                    0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,'-',  //dn(-)
                                    0xBE,0xFF,0xEB,0xAA,0xBE,0xFF,0xEB,0xAA,0x02,'C',  //zoom(C)
                                    0xBE,0xFF,0xEB,0xAA,0xAA,0xFA,0xFF,0xAF,0x02,'S',  //slow(S)
                                    0xBE,0xFF,0xEB,0xAA,0xAA,0xFF,0xFF,0xAA,0x02,'T',  //goto(T)
                                    0xBE,0xFF,0xEB,0xAA,0xEE,0xFE,0xBB,0xAB,0x02,'t',  //Repeat(t)
                                    0xBE,0xFF,0xEB,0xAA,0xAE,0xFF,0xFB,0xAA,0x02,'F'}; //mark(F)*/



static int16 i = 0;
static int8 j = 0;

static int1 Stream[SIZE*8];
static int8 StreamByte[SIZE];
static int8 CmdRem = 0;

static int8 PinState = 1;
static int8 PinStateOld = 1;
static int8 RcvEn = false;
static int8 EndLoop;
static int8 Count = 0;
static int8 TMR = 0;
static int8 s1, s2;




#int_CCP1
void COMP_isr()      //ocur every  (CCP_1 * 0.25*8)us = 100ms
{
T1Div--;
if(T1Div == 0){
    T1Div = RecargaT1Div;
    ClockSec++;
    if(ClockSec == 60){
        ClockSec = 0;
        ClockMin++;
        if(ClockMin == 60){
            ClockMin = 0;
            ClockHou++;
            if(ClockHou == 13){
                ClockHou = 1;
                }
            }
        }
    if(TimerEnable == true){
        TimerSec--;
        if(TimerSec == -1){
            TimerSec = 59;
            TimerMin--;
            if(TimerMin == -1){
                TimerMin = 59;
                if(TimerHou != 0){
                    TimerHou--;
                    }
                }
            }
        }
    }
}



void main()
{
    Config();

    While(1){

//---------------------------------------------------------------------------
//    Checheo del Control Remote
//---------------------------------------------------------------------------
        //output_bit(PIN_A1,!input(PIN_A1));

        PinState = input(PIN_REMT);//leer en pin del remoto
           //si el pin del remoto bajo a cero y no hay recepcion andando
        if(PinState == FALSE && RcvEn ==  false){

            EndLoop = false;
            Count = 0;                  //lleva la cuenta de las transiciones
            i = 0;                      //lleva el numero de encuestas al pin
            PinStateOld = FALSE;
            do{
                PinState = input(PIN_REMT);
                if(PinState != PinStateOld){   //esperar por transicion
                    TMR = get_timer0();
                    set_timer0(0);

                    if( RcvEn == true ){       //si no es la primera transicion
                        if(TMR < 0x20)
                            Stream[Count] = 1;
                        else
                            Stream[Count] = 0;
                        Count++;               //lleva la cuenta de las transiciones
                        }
                    PinStateOld = PinState;
                    i = 0;                    //en cada transicion se resetea el numero de encuestas al pin
                    RcvEn = true;             //en cada transicion se setea
                    }
                i++;                          //incrementa el numero de encuestas al pin

                   // si pasaron maximo numero de transiciones o se encuesto mas de 2600 veces al pin
                if(i > 2600 || Count >= SIZE*8 ){ //650
                    RcvEn = false;
                    EndLoop = true;           //se termina el ciclo
                    }

                }while( EndLoop == false );      //ciclo de encuesta al pin

               //se completa el resto del stream si hace falta con ceros
            while( Count <= SIZE*8 ){
                Stream[Count] = 0;
                Count++;
                }

               //se crea a partir del stream binario otro de bytes
            for(i = 0; i< SIZE; i++){
                for (j = 0; j < N_CMD; j++){
                    if(Stream[i*8 + j] == 1)
                        bit_set(StreamByte[i], j);
                    else
                        bit_clear(StreamByte[i], j);
                    }
                }
            //FOR DEBUGING PURPOSE ONLY!!
            /*WriteCmd(0x01);
            WriteC('<');
            for(i = 0; i< SIZE; i++){
                WriteC((StreamByte[i]%16)+0x41);
                WriteC((StreamByte[i]/16)+0x41);
                }
            WriteC('>');
            delay_ms(4000);*/

               //se compara con el numero de botones conocidos
            for (j = 0; j < N_CMD; j++){
                i = 0;
                do  {
                    s1 = TABLE[j][i];
                    s2 = StreamByte[i];
                    i++;
                    }while((i < SIZE) && (s1 == s2));  //se continua comparando si son iguales
                                                   //y mientras queden bytes por comparar
                if(i == SIZE)        //si compare con todos los byte es que hay coincidencia
                    CmdRem = TABLE[j][9];   //copio el cogigo guardado en la misma tabla al final
                }
            //delay_ms(100);
            }


        if(RefreshDiv == 0){ //una vez cada 100ms aprox

    //------------------------------------------------------------------
    //    Adquisicion de la Temperatura
    //------------------------------------------------------------------

            SampleADC();    //demora aprox 16.6ms


    //------------------------------------------------------------------
    //    Escaneo de teclas
    //------------------------------------------------------------------

            if( (!input(PIN_PWER)) || (CmdRem == 'P') ){  // if PIN_xx go to high
                while( !input(PIN_PWER) ) { //wait until released
                    }
                KeyPress = 1;
                Power = !Power;
                if(Power == false)
                    WaitComp = true;

                TimerEnable = false;
                TimerSec = TimerSecSvd;
                TimerMin = TimerMinSvd;
                TimerHou = TimerHouSvd;

                Display = 3; //Power  menu
                DivDisp = LOOP_TIME-10;
                }

            if(CmdRem == 'S'){    //if remote Display button
                KeyPress = 1;
                Display++;
                if(Display >= 9)
                    Display = 0;
                DivDisp = 0;
                }

            if(CmdRem == 'T'){    //if remote Timer (click Command) button
                KeyPress = 1;
                if(Display < 6)
                    Display = 6;
                Display++;
                if(Display == 9){
                    Display = 6;
                    }

                DivDisp = 0;
                }

            if( (!input(PIN_TMER))){  // if PIN_xx go to high
                i=0;
                while( !input(PIN_TMER) && i<HOLD_DELAY ){ //wait until released
                    i++;
                    delay_ms(1);
                    }
                if(i == HOLD_DELAY){ //if Hold command
                    if (Power == true){
                        KeyPress = 1;
                        TimerSec = TimerSecSvd;
                        TimerMin = TimerMinSvd;
                        TimerHou = TimerHouSvd;
                        TimerEnable = !TimerEnable;

                        Display = 2; //timer  menu
                        DivDisp = LOOP_TIME-10;
                        }
                    while( !input(PIN_TMER)){ }
                    }
                else{        //if click Command
                    KeyPress = 1;
                    if(Display < 6)
                        Display = 6;
                    Display++;
                    if(Display == 9){
                        Display = 6;
                        }
                    DivDisp = 0;
                    }
                }

            if( (!input(PIN_TPUP)) || (CmdRem == '+') ){  // if PIN_xx go to high
                //while( !input(PIN_TPUP) ) { //wait until released
                    //}
                KeyPress = 1;
                if(Display < 6)
                    Display = 6;
                if(Display == 8){            //change Timer Mins
                    TimerEnable = false;
                    TimerMinSvd+=15;
                    if(TimerMinSvd == 60){
                        TimerMinSvd = 0;
                        }
                    }
                else if(Display == 7){       //change Clock Mins
                    ClockMin++;
                    ClockSec = 0;
                    if(ClockMin == 60){
                        ClockMin = 0;
                        }
                    }
                else if(Display == 6){      //change Temp
                    if(RefTemp < 400) RefTemp += 2;
                    }
                DivDisp = 0;
                }


            if( (!input(PIN_TPDN)) || (CmdRem == '-') ){  // if PIN_xx go to high
                //while( !input(PIN_TPDN) ) { //wait until released
                    //}
                KeyPress = 1;
                if(Display < 6)
                    Display = 6;
                if(Display == 8){             //change Timer Hours
                    TimerEnable = false;
                    TimerHouSvd++;
                    if(TimerHouSvd == 13){
                        TimerHouSvd = 1;
                        }
                    }
                else if(Display == 7){      //change Clock Hours
                    ClockHou++;
                    if(ClockHou == 13){
                        ClockHou = 1;
                        }
                    }
                else if(Display == 6){        //change Temp
                    if(RefTemp > 150) RefTemp -= 2;
                    }
                DivDisp = 0;
                }

            if( !input(PIN_LOWV) ){  // if PIN_xx go to high
                if(Wait == false){
                    Wait = true;
                    SaveConfig();
                    }
                WaitCounter = WAIT_TIME;
                Display = 9; //Low Voltage detect
                DivDisp = 0;
                }


            if(Power == true){
                if( (!input(PIN_COOL)) || (CmdRem == 'C') ){  // if PIN_xx go to high
                    while( !input(PIN_COOL) ) { //wait until released
                        }
                    KeyPress = 1;
                    Cool = !Cool;

                    Display = 5; //cool  menu
                    DivDisp = LOOP_TIME-10;
                    }
    
                if( (!input(PIN_FSPD)) || (CmdRem == 'F') ){  // if PIN_xx go to high
                    while( !input(PIN_FSPD) ) { //wait until released
                        }
                    KeyPress = 1;
                    Speed++;
                    if(Speed == 5)
                        Speed = 1;

                    Display = 4; //fan  menu
                    DivDisp = 0;
                    }
    
                if(CmdRem == 't'){    //if remote Timer Enable(hold) button
                    KeyPress = 1;
                    TimerSec = TimerSecSvd;
                    TimerMin = TimerMinSvd;
                    TimerHou = TimerHouSvd;
                    TimerEnable = !TimerEnable;
    
                    Display = 2; //timer  menu
                    DivDisp = LOOP_TIME-10;
                    }

                }

            CmdRem = 0;  //clear remote command to avoid multiple triggering


    //-----------------------------------------------------------------
    //    Logica de estados
    //-----------------------------------------------------------------
    
            if(Power == true){
                if(Wait == false){
                    if(TimerEnable == true){
                        if(TimerMin == 0 && TimerHou == 0 && TimerSec == 0){
                            TimerEnable = false;
                            TimerSec = TimerSecSvd;
                            TimerMin = TimerMinSvd;
                            TimerHou = TimerHouSvd;
                            Power = false;
                            WaitComp = true;
                            }
                        }

                    if(Cool == true){
                        if(Temp > CompTemp){
                            Comp = ON;
                            CompTemp = (RefTemp-HYST);
                            }
                        else{
                            if(Comp == ON)
                                WaitComp = true;
                            Comp = OFF;
                            CompTemp = (RefTemp+HYST-1);
                            }
                        }
                    else{//(Cool == false)
                        if(Comp == ON)
                            WaitComp = true;
                        Comp = OFF;
                        }
    
                    switch(Speed){
                        case 1: {
                            Output_Low(PIN_FAN2);
                            Output_Low(PIN_FAN3);
                            Output_High(PIN_FAN1);
                            break;
                            }
                        case 2: {
                            Output_Low(PIN_FAN1);
                            Output_Low(PIN_FAN3);
                            Output_High(PIN_FAN2);
                            break;
                            }
                        case 3: {
                            Output_Low(PIN_FAN1);
                            Output_Low(PIN_FAN2);
                            Output_High(PIN_FAN3);
                            break;
                            }
                        case 4: {
                            if (Comp == ON){
                                Output_Low(PIN_FAN1);
                                Output_Low(PIN_FAN2);
                                Output_High(PIN_FAN3);
                                }
                            else{//(Comp == OFF)
                                Output_Low(PIN_FAN2);
                                Output_Low(PIN_FAN3);
                                Output_High(PIN_FAN1);
                                }
                            break;
                            }
                        }


                    if( WaitComp == false && Comp == ON )
                        Output_High(PIN_COMP);
                    else
                        Output_Low(PIN_COMP);


                    }
                else{//(Wait == true)
                    Output_Low(PIN_COMP);
                    Output_Low(PIN_FAN1);
                    Output_Low(PIN_FAN2);
                    Output_Low(PIN_FAN3);
                    }
                }
            else{//(Power == false)
                Output_Low(PIN_COMP);
                Output_Low(PIN_FAN1);
                Output_Low(PIN_FAN2);
                Output_Low(PIN_FAN3);

                }

    //------------------------------------------------------------------
    //    Visualizacion en LCD y Backlight
    //------------------------------------------------------------------
            RefreshLCD();

            if(KeyPress == 1){

                i=0;
                while(i<600){
                    int1 pin;
                    pin = input(PIN_BEEP);
                    if(pin == 0)
                        delay_us(200);
                    else
                        delay_us(200);            //beep at 5KHz for 150ms

                    output_bit(PIN_BEEP,!pin);
                    i++;
                    }
                delay_ms(100);
                }



            if(WaitComp == true){
                WaitCompCounter--;
                if(WaitCompCounter == 0){
                    WaitComp = false;
                    WaitCompCounter = WAIT_TIME;
                    }
                }

            if(Wait == true){
                WaitCounter--;
                if(WaitCounter == 0){
                    Wait = false;
                    //WaitComp = true;
                    WaitCounter = WAIT_TIME;
                    }
                }

            if(KeyPress == 1){
                KeyPress = 0;
                }


            }//if (RefreshDiv = 0)

        RefreshDiv++;
        if(RefreshDiv == 400)        //refresca cada 100ms aprox
            RefreshDiv = 0;
        delay_us(250);
        }  //while
}// main


void Config(void){
    SET_TRIS_A( 0x3D );               //A0, A2, A3, A4, A5 input; A1  output
    SET_TRIS_B( 0x0F );               //A0, A1, A2, A3 input; A4, A5, A6, A7, output
    SET_TRIS_C( 0x03 );               //C0, C1 input; C2, C3, C4, C5, C6, C7, output
    port_b_pullups(TRUE);
    setup_adc_ports(AN0_VREF_VREF);   //Input: PIN_A0   VRefh=A3 VRefl=A2
    setup_adc(ADC_CLOCK_INTERNAL);
    setup_spi(FALSE);
    setup_ccp1(CCP_COMPARE_RESET_TIMER);
    setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128);
    setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);


/*con XT de 16MHz  time base de 0.25us
timer 0 res 0.25us  div 128 --count every 32 us
timer 1 res 0.25us  div 8  --count every 2 us
*/
    setup_comparator(NC_NC_NC_NC);
    setup_vref(FALSE);
    enable_interrupts(INT_CCP1);

   // TODO: USER CODE!!
    OUTPUT_A( 0x00 );
    OUTPUT_B( 0x00 );
    OUTPUT_C( 0x00 );
    output_bit( PIN_EN, 1);
    delay_ms(200);  //demora necesaria para el LCD start-up time

    output_bit( PIN_RS, 0);
    OUTPUT_B( 0x20 );
    output_bit( PIN_EN, 0);
    delay_us(1);
    output_bit( PIN_EN, 1);
    delay_ms( 4 );

    WriteCmd(0x28);               //initialize LCD
    WriteCmd(0x0C);
    WriteCmd(0x01);

    CCP_1 = 50000;

    set_adc_channel(0);

    LoadConfig();
    if(TimerEnable == true){
        TimerEnable = false;
        TimerSec = TimerSecSvd;
        TimerMin = TimerMinSvd;
        TimerHou = TimerHouSvd;
        }

    enable_interrupts(GLOBAL);
   }


void RefreshLCD(void){
    DivDisp++;
    if(DivDisp == LOOP_TIME){
        DivDisp = 0;

        Display++;
        if(Display >= 2)
            Display = 0;
        }

    WriteCmd(0x02);
    WriteCmd(0x80);

    WriteStr(Display);
    WriteCmd(0xC0);
    switch(Display){
        case 0:{    //Temperature
            DspInt16(Temp);
            break;
            }
        case 1:{    //Time
            DspTime(Clock);
            break;
            }
        case 2:{    //Timer
            if(TimerEnable) WriteStr(10);
            else WriteStr(11);
            WriteC(' ');
            break;
            }
        case 3:{    //Power
            if(Power) WriteStr(10);
            else WriteStr(11);
            WriteC(' ');
            break;
            }
       case 4:{     //Fan speed
            if(Speed != 4){
                WriteC(Speed+0x30);
                switch(Speed){
                    case 1: { WriteC('r'); break; }
                    case 2: { WriteC('d'); break; }
                    case 3: { WriteC('r'); break; }
                    }
                WriteC('a');
                WriteC('.');
                WriteC(' ');
                WriteC('V');
                WriteC('e');
                WriteC('l');

                }
            else{
                WriteC('E');
                WriteC('c');
                WriteC('o');
                WriteC(' ');
                WriteC('M');
                WriteC('o');
                WriteC('d');
                WriteC('e');
                }

            break;
            }

        case 5:{    //cool
            if(Cool) WriteStr(10);
            else WriteStr(11);
            WriteC(' ');
            WriteC(' ');
            WriteC(' ');
            break;
            }
        case 6:{    //set temperature
            DspInt16(RefTemp);
            break;
            }
        case 7:{    //set clock
            DspTime(Clock);
            break;
            }
        case 8:{    //set timer
            DspTime(Timer);
            break;
            }
        case 9:{  //Over Voltaje detected
            WriteC('D');
            WriteC('e');
            WriteC('t');
            WriteC('e');
            WriteC('c');
            WriteC('t');
            WriteC('e');
            WriteC('d');
            break;
            }
        }

    }


void SampleADC(void){            //(1/60)/64smp  =  samp every 260us aprox
    Temp = 0;


    for(i =0; i<64; i++){
        Temp = Temp + read_adc(ADC_START_AND_READ);
        delay_us(260-53);     //53us toma dar una vuelta al ciclo aprox!!!
        }
    Temp = Temp>>7;        //6 es lo que va!!!, pero 7 ya divide por 2!!!!
    }

void DspInt16(int16 Var){
    int8 Stack[5];
    for(i = 0; i<5;i++){
        Stack[i] = (Var%10) + 0x30 ;
        Var = Var/10;
        }
    //WriteC(Stack[4]);
    //WriteC(Stack[3]);
    WriteC(Stack[2]);
    WriteC(Stack[1]);
    WriteC('.');
    WriteC(Stack[0]);
    WriteC(' ');
    WriteC(0xDF); //grados(°)
    WriteC('C');
    WriteC(' ');
    }

void DspTime(int1 time){
    int8 Hou, Min, Sec;
    if (Time == Clock){
        Hou = ClockHou;
        Min = ClockMin;
        Sec = ClockSec;
        }
    else{// (Time == Timer)
        if(TimerEnable == true){    //if Timer is already running display actual time
            Hou = TimerHou;
            Min = TimerMin;
            Sec = TimerSec;
            }
        else{                       //if is stoped display saved(target) time
            Hou = TimerHouSvd;
            Min = TimerMinSvd;
            Sec = TimerSecSvd;
            }
        }
    WriteC(Hou/10 + 0x30);
    WriteC(Hou%10 + 0x30);
    WriteC(':');
    WriteC(Min/10 + 0x30);
    WriteC(Min%10 + 0x30);
    WriteC(':');
    WriteC(Sec/10 + 0x30);
    WriteC(Sec%10 + 0x30);
    }

void WriteCmd(int8 Cmd){

    output_bit( PIN_RS, 0);
    OUTPUT_B(Cmd & 0xF0);
    output_bit( PIN_EN, 0);
    delay_us(1);
    output_bit( PIN_EN, 1);

    swap(Cmd);

    OUTPUT_B(Cmd & 0xF0);
    output_bit( PIN_EN, 0);
    delay_us(1);
    output_bit( PIN_EN, 1);

    delay_ms( 5 );
}


void WriteStr(int8 Line){
    int8 i;
    for(i = 0; i < 7; i++){
        WriteC(STRINGS[Line][i]);
        }
    }



void WriteC(int8 Charact){

    output_bit( PIN_RS, 1);
    OUTPUT_B(Charact & 0xF0);
    output_bit( PIN_EN, 0);
    delay_us(1);
    output_bit( PIN_EN, 1);

    swap(Charact);

    OUTPUT_B(Charact & 0xF0);
    output_bit( PIN_EN, 0);
    delay_us(1);
    output_bit( PIN_EN, 1);
    delay_us( 100);
    }

void  SaveConfig(void){
    write_eeprom(0x20,*(0x3E));
    write_eeprom(0x21,*(&RefTemp));
    write_eeprom(0x22,*(&(RefTemp)+1));
    write_eeprom(0x23,TimerMinSvd);
    write_eeprom(0x24,TimerHouSvd);
    write_eeprom(0x25, Speed);
    }

void  LoadConfig(void){
   *(0x3E) = read_EEPROM(0x20);
   *(&RefTemp) = read_EEPROM(0x21);
   *(&(RefTemp)+1) = read_EEPROM(0x22);
   TimerMinSvd = read_EEPROM(0x23);
   TimerHouSvd = read_EEPROM(0x24);
   Speed = read_EEPROM(0x25);
   }

#ROM 0x2120 = {0,250,0,0,6,1}
