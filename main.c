#include <msp430.h>

#define   RX   0x04U
#define   TX   0x02U
#define   BAUD_RATE 9600U

#define RED_LED   0x01
#define GREEN_LED 0x40

static volatile unsigned int sleepCounter;
__attribute__((interrupt(WDT_VECTOR))) void WDT_ISR(void) {
   if(sleepCounter != 0U) {
      sleepCounter--;
      if(sleepCounter == 0U) {
         __bic_SR_register_on_exit((unsigned int) LPM1_bits);
      }
   }
}

/*@unused@*/
static void sleep(unsigned int t) {
   sleepCounter = t;
   __bis_SR_register((unsigned int)(LPM1_bits + GIE));
}

#ifdef __MSP430_HAS_SD16_A1__
static unsigned int readTemperature(void) {
   return SD16MEM0;
}
static void initADC(void) {
   SD16CTL = (unsigned int) SD16SSEL_1 | SD16REFON | SD16XDIV_3 | SD16DIV_3;
   SD16INCTL0 = (unsigned int) SD16INCH_6;
   SD16CCTL0 = (unsigned int) SD16SC;
}
#elif defined __MSP430_HAS_ADC10__

__attribute__((interrupt(ADC10_VECTOR)))
void ADC10_ISR(void) {

   /* ADC conversion complete. Clean flags and wake up. */
   ADC10CTL0 &= (unsigned int) ~(ENC + ADC10IFG);
   __bic_SR_register_on_exit((unsigned int) LPM1_bits);
}

static unsigned int readTemperature(void) {

   /* Clear interrupt flag, enable and start conversion, enable interrupt */
   ADC10CTL0 &= (unsigned int) ~ADC10IFG;
   ADC10CTL0 |= (unsigned int) (ENC + ADC10SC + ADC10IE);

   /* Sleep and wait for ADC interrupt */
   __bis_SR_register((unsigned int)(LPM1_bits + GIE));

   return ADC10MEM;
}

static void initADC(void) {
   ADC10CTL0 = (unsigned int) (SREF_1 + ADC10SHT_3 + REFON + ADC10ON);
   ADC10CTL1 = (unsigned int) (INCH_10 + ADC10DIV_3);
}

#else
#error Could not recognize ADC hardware!
#endif

static volatile unsigned char rxBitCounter; /* How many more bits to receive, minus one.
                                 0 means one more bit to receive. */
static volatile unsigned char recvBits; /* Current receiving byte buffer */
static volatile unsigned char txBitCounter;
static volatile unsigned int txBits;
static volatile unsigned char receivedByte; /* Last received byte */
static volatile char isReceiving, isTransmitting;
static volatile char wakeAfterRx, wakeAfterTx;
#ifdef __MSP430_HAS_TA2__
__attribute__((interrupt(TIMERA0_VECTOR)))
#elif defined __MSP430_HAS_TA3__
__attribute__((interrupt(TIMER0_A0_VECTOR)))
#else
#error Could not recognize Timer hardware!
#endif
void TIMERA0_ISR(void) {
   if(isReceiving) {

      recvBits >>= 1U;

      if((P1IN & RX) != 0U) {
         recvBits |= 0x80;
      }

      if(rxBitCounter == 0U) {
         /* Done receiving, handle received byte */

         if(isTransmitting == 0) {
            CCTL0 &= (unsigned int) ~CCIE;  /* CCR0 interrupt disabled */
         }
         P1IE  |= RX;  /* Enable RX start bit interrupt */
         P1IFG &= ~RX; /* Clear interrupt */
         isReceiving = 0U;

         receivedByte = recvBits;
         if(wakeAfterRx != 0) {
            __bic_SR_register_on_exit((unsigned int) LPM1_bits);
         }
      }
      else {
         rxBitCounter--;
      }
   }

   if(isTransmitting) {
      if(txBits & 1U) {
         P1OUT |= TX;
      }
      else {
         P1OUT &= ~TX;
      }
      if(txBitCounter == 0U) {
         if(isReceiving == 0) {
            CCTL0 &= (unsigned int) ~CCIE;  /* CCR0 interrupt disabled */
         }
         isTransmitting = 0U;
         if(wakeAfterTx != 0) {
            __bic_SR_register_on_exit((unsigned int) LPM1_bits);
         }

      }
      else {
         txBits >>= 1;
         txBitCounter--;
      }
   }
}

__attribute__((interrupt(PORT1_VECTOR))) void P1_ISR(void) {

   isReceiving = 1;

   /* If we are transmitting, the timer can not be adjusted.
      The recieved byte could be noisy, but will hopefully be ok.
      Don't send bytes from the other side when receiving!
    */
   if(isTransmitting == 0) {
      TAR = CCR0 + (CCR0 >> 1); /* Initialize to half the period, to sample in the middle*/
   }
   TACCTL0 = (unsigned int) CCIE;           /* CCR0 interrupt enabled */
   P1IE  &= !RX;  /* Disable RX start bit interrupt */
   P1IFG &= ~RX; /* Clear interrupt */

   rxBitCounter = 7U;
}

static void putbyte(char c) {
   wakeAfterTx = 1;
   txBits = 0x600U | ((unsigned int)c << 1U);
   txBitCounter = 9U;
   isTransmitting = 1;
   TACCTL0 = (unsigned int) CCIE;           /* CCR0 interrupt enabled */
   __bis_SR_register((unsigned int)(LPM1_bits + GIE));
}
static void print(char *s) {
   while(*s) {
      putbyte((*s));
      s++;
   }
}
static void printHex(unsigned int h) {
   unsigned int i;
   char * digits = "0123456789abcdef";
   for(i=0U; i<4U; i++) {
      putbyte(digits[(0xf000U & h)>>12U]);
      h <<= 4U;
   }
}

static char getbyte(void) {
   wakeAfterRx = 1;
   __bis_SR_register((unsigned int)(LPM1_bits + GIE));
   wakeAfterRx = 0;
   return (char) receivedByte;
}

static void commandLine(void) {
   char * help = "Uart test program\r\n"
         "Commands:\r\n"
         "r: blink red led\r\n"
         "g: blink green led\r\n"
         "t: read temperature sensor\r\n"
         "?: print this help\r\n";
   char c;
   print(help);
   for(;;) {
      print(">>>");
      c = getbyte();
      putbyte(c);
      print("\r\n");
      switch(c) {
         case 'r':
            print("Blinking red led...\r\n");
            P1OUT ^= RED_LED;
            break;
         case 'g':
            print("Blinking green led...\r\n");
            P1OUT ^= GREEN_LED;
            break;
         case 't':
            print("Temperature: ");
            printHex(readTemperature());
            print("\r\n");
            break;
         case '?':
            print(help);
            break;
         default:
            print("Unknown command.\r\n");
      }
   }

}

int main(void) {
   /* setup watchdog timer */
   WDTCTL = (unsigned int) WDT_MDLY_0_064; /* ~30mS intervals */
   IE1 |= WDTIE; /* enable watchdog interrupt */

   /* Go turbo mode! */
   BCSCTL1 =  CALBC1_12MHZ;
   DCOCTL = CALDCO_12MHZ;

   /* Setup initial pin directions */
   P1DIR &= ~RX;
   P1DIR |= (TX | RED_LED | GREEN_LED);

   P1IES |= RX;  /* RX normally high, interrupt on falling edge */
   P1IE  |= RX;  /* Enable RX start bit interrupt */
   P1IFG &= ~RX; /* Clear interrupt */

   /* Timer settings */
   TACTL = (unsigned int) (TASSEL_2 + MC_1 + ID_1);  /* SMCLK/2, upmode */
   CCR0 =  6000000UL/BAUD_RATE; /* 12000000.0 / 2 / BAUD_RATE*/

   initADC();
   commandLine();
   for(;;);
}
