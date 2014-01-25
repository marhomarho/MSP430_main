#include <msp430.h>

/*
 * main.c
 */
 
void init_MSP430(void); // initialization uController
void init_SPI(void); // initialization SPI
void init_I2C(void); // initialization I2C
void init_UART(void); // initialization UART
void init_ADC(void);
void init_WDT_Timer(void); // initialization Wake-up timer

void WDT_Timer_Update(void); // Set the Wake-up timer dinamically depending Battery/Energy Level
void read_ADCvalue(char adcChannel); // Read the ADC value
void readEnergLevel(void); //Read the energy level, adc voltage and current of 4 channel: VBat,Vsc,Ibat,Isc
//algorithm to compute energy level based on (+ proportional, - invers_proportional): +Battery level,+(Vsc*Isc),-Battery Current. everything as %
void txDataSPI(char data, int nByte); //tx datas through SPI
void txRF(); //RF TX
void read_SPI_sensor_data();
void read_I2C_sensor_data();
void txUART(); //+ routine interupt receive

 
void measure_count_RC(void);
char measuring=0;
int timer_count;
int meas_cnt[16];
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer

     DCOCTL = 0;                               // Select lowest DCOx and MODx settings
     BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1, 8, 12 or 16MHz
     DCOCTL = CALDCO_1MHZ;
     BCSCTL2 |= DIVS_2;                        // SMCLK/(0:1,1:2,2:4,3:8)
     BCSCTL1 |= DIVA_1;                        // ACLK/(0:1,1:2,2:4,3:8)
     BCSCTL3 |= XCAP_1;
     // BCSCTL3 |= LFXT1S_2;                      // LFXT1 = VLO
     P1OUT &=~ BIT0+BIT6;
     P1DIR |= BIT0 + BIT6;
     P1OUT |= BIT0;  // TURN ON LED

     TA0CTL = TASSEL_1 + MC_1+ ID_3; //MC_1 Count up, TASSEL aclk

     P2DIR |= BIT4; //OUTPUT

while(1)
    {

    if (!measuring)
    measure_count_RC();

    __delay_cycles(100);


    }
}

     void measure_count_RC(void)
     {
         P1OUT |= BIT6;
         P2OUT |= BIT4;  // PIN2.4=0;
         __delay_cycles(4); // wait for hard charge of the capactiro

         P2IES |= BIT4; //INTERRUPT EDGE
         P2IE |= BIT4; //hi-lo edge
         //PxIES = 0 means a change from low to high (0 to 1) will create an interrupt flag
         //PxIES = 1 means a change from high to low (1 to 0) will create an interrupt flag

         P2IFG &=~ BIT4; // P2.4 IFG cleared
         P2DIR &=~ BIT4; //INPUT
         timer_count = TAR; // Take a snapshot of the timer
         measuring=1;
         _BIS_SR(LPM3_bits + GIE); //__enable_interrupt(); // enable all interrupts

         //meas_cnt[i]= timer_count;

         // Now repeat with charging cycle and average results
   }


     // Port 1 interrupt service routine
     #pragma vector=PORT2_VECTOR
     __interrupt void Port_2(void)
     {
         P2IE &= ~BIT4; // disable futher interuput
         timer_count=TAR-timer_count;
         // Get charge/discharge time
         P1OUT ^= (BIT0); // P1.0 = toggle
         P1OUT &= ~BIT6;
         P2IFG &= ~BIT4; // P1.3 IFG cleared
         //P1IES ^= BUTTON; // toggle the interrupt edge,
         measuring=0;

     // the interrupt vector will be called
     // when P1.3 goes from HitoLow as well as
     // LowtoHigh
          __bic_SR_register_on_exit(LPM3_bits);     // Exit LPM3 on reti

     }



