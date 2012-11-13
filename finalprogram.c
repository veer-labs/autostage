//          MalariaAUTOstage.c      RGD/SMS 11/26/08
//          Sheel Shah


#include <16F84a.H>
#fuses XT, NOWDT, NOPROTECT     //  This sets up the internal device fuses: HS crystal, WDT ON, Code Protect OFF
#use delay (clock=4000000)    //  4 MHz crystal, 1 microsecond instruction cycle time.
#byte PORTA = 0x05
#byte PORTB = 0x06

/////////  Step motor and power screw characteristics:
/////////  20 steps in rt or lt is one field/ half step forward/backward is one field
/////////  NOTE: max step rate is: delay is ~ 2000us for MicroMo gearhead step motor without ramp up


////////  PROTOCOL:
//  Hit switch to power microcontroller (on)
//  Insert sample into sample tray
//  Position sample with position controls
//  Focus with bottom polarizer in phase
//  Once focused, rotate polarizer out of phase
//  Push diagnose button
//  Wait for Diagnosis
//  Wait for sample to return home



int i;
int j;

unsigned int step;          //  this is the counter for keeping track of which step is current
unsigned int fieldx;        //  counter
unsigned int fieldy;

unsigned int d;             //  dwell counter



unsigned long int num_stepsx;//  for taking a requested number of half steps
unsigned long int num_stepsy;//  for taking a requested number of half steps
unsigned long int bx;        //  num_step counter
unsigned long int by;        //  num_step counter

int steps[9] = {0b00000000,0b00001000,0b00001100,0b00000100,0b00000110,0b00000010,0b00000011,0b00000001,0b00001001};

///////////////////////////////////////

// SUBROUTINES



void init_ports(void) {

   SET_TRIS_A(0b00000000);  // PORTA = all outputs except RA4, which is an INPUT
   SET_TRIS_B(0b11100001);  // PORTB = all inputs, except B0, which is an OUTPUT to the LED

   SETUP_COUNTERS(RTCC_INTERNAL, WDT_1152MS);   //  Set Watch Dog Timer prescaler to 1152 ms
   // The Watch Dog Timer (WDT) is just an internal timer for the microprocessor that runs off
   // on the side independently.  If the microprocessor hangs up or gets confused, the WDT
   // will reset the microprocessor to the first line of the executable code "main()".
   // To stop this from happening accidentally, we need to reset the WDT very frequently.
   // in this case, every second or less.  Otherwise, the controller will reset automatically.


   // Default output values:
      PORTA = 0b00000000;
      Output_Bit(Pin_B4, 0);
      Output_Bit(Pin_B1, 0);
      Output_Bit(Pin_B2, 0);
      Output_Bit(Pin_B3, 0);

   // Default variable values

   step = 0;
   bx = 0;
   by = 0;

}

/////////////////////////////////////////////////////////////////////////

void output_on_port_b(int x_step){
   OUTPUT_BIT(PIN_B1, bit_test(x_step,0));
   OUTPUT_BIT(PIN_B4, bit_test(x_step,1));
   OUTPUT_BIT(PIN_B3, bit_test(x_step,2));
   OUTPUT_BIT(PIN_B2, bit_test(x_step,3));
}

///////////////////////////////////////////////////////////////////////

void output_on_port_a(int y_step){
   OUTPUT_BIT(PIN_A0, bit_test(y_step,0));
   OUTPUT_BIT(PIN_A2, bit_test(y_step,1));
   OUTPUT_BIT(PIN_A1, bit_test(y_step,2));
   OUTPUT_BIT(PIN_A3, bit_test(y_step,3));
}

///////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
void take_stepsxright(void)  {      //  take a specified number of steps: num_steps -- one field x right
  for(i=1;i<num_stepsx;i++){
   j = i%9;
   if(j!=0){
      output_on_port_b(steps[j]);
      delay_ms(14);                    //  delay between steps for lengthening strain
      output_on_port_b(steps[0]);   //  de-energize the motor coils
   }
  }
}

/////////////////////////////////////////////////////////////////////////
void take_stepsxleft(void)  {      //  take a specified number of steps: num_steps -- one field x right
  for(i=0;i<num_stepsx;i++){
   j = 8 - i%9;
   if(j!=0){
      output_on_port_b(steps[j]);
      delay_ms(14);                    //  delay between steps for lengthening strain
      output_on_port_b(steps[0]);   //  de-energize the motor coils
   }
  }
}



/////////////////////////////////////////////////////////////////////////
void take_stepsyfwd(void)  {      //  take a specified number of steps: num_steps -- one field x right
  for(i=1;i<num_stepsy;i++){
   j = i%9;
   if(j!=0){
      output_on_port_a(steps[j]);
      delay_ms(14);                    //  delay between steps for lengthening strain
      output_on_port_a(steps[0]);   //  de-energize the motor coils
   }
  }
}

/////////////////////////////////////////////////////////////////////////
void take_stepsybk(void)  {      //  take a specified number of steps: num_steps -- one field x right
  for(i=0;i<num_stepsy;i++){
   j = 8 - i%9;
   if(j!=0){
      output_on_port_a(steps[j]);
      delay_ms(14);                    //  delay between steps for lengthening strain
      output_on_port_a(steps[0]);   //  de-energize the motor coils
   }
  }
}

///////////////////////////////////////////////////////////////////////
void SHORTENX(void) {  // jog the carriage to lengthen all specimens while <LENG> is pushed
      i=0;
    while(!INPUT(PIN_B5)) {
      j = 8-i%9;
      if(j!=0){
         output_on_port_b(steps[j]);
         delay_us(2000);                    //  delay between steps for manual jog
         output_on_port_b(steps[0]);   //  de-energize the motor coils
      }
      i++;
   }
}


///////////////////////////////////////////////////////////////////////
void LENGTHENX(void) {  // jog the carriage to shorten all specimens while <SHORT> is pushed
      i=0;
    while(!INPUT(PIN_B6)) {
      j = i%9;
      if(j!=0){
         output_on_port_b(steps[j]);
         delay_us(2000);                    //  delay between steps for manual jog
         output_on_port_b(steps[0]);   //  de-energize the motor coils
      }
      i++;                        //  de-energize the motor coils
   }
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void SHORTENY(void) {  // jog the carriage to lengthen all specimens while <LENG> is pushed
    i = 0;
    while(!INPUT(PIN_B5)) {
      j = 8 - i%9;
      if(j!=0){
         output_on_port_a(steps[j]);
         delay_us(2000);                    //  delay between steps for lengthening strain
         output_on_port_a(steps[0]);   //  de-energize the motor coils
      }
      i++;
     }                               //  end while
}


///////////////////////////////////////////////////////////////////////
void LENGTHENY(void) {  // jog the carriage to shorten all specimens while <SHORT> is pushed
       i=0;
    while(!INPUT(PIN_B6)) {
      j = i%9;
      if(j!=0){
         output_on_port_a(steps[j]);
         delay_us(2000);                    //  delay between steps for manual jog
         output_on_port_a(steps[0]);   //  de-energize the motor coils
      }
      i++;
   }
}
///////////////////////////////////////////////////////////////////////
/////Interrupt Subroutines
#INT_EXT
EXT_isr(void)
{
         fieldy = 0;

         num_stepsx = 180;

         num_stepsy = 54;

	 for (fieldy = 0; fieldy < 10; fieldy++) {
                delay_ms(500)
                Output_Bit(Pin_A4, 1);
           		delay_ms(200);
           		Output_Bit(Pin_A4, 0);
           		delay_ms(500)

		if (fieldy%2 == 0) {
			 for (fieldx = 0; fieldx < 10; fieldx++) {
            		take_stepsxright();
                    delay_ms(500)
                    Output_Bit(Pin_A4, 1);
           		    delay_ms(200);
           		    Output_Bit(Pin_A4, 0);
           		    delay_ms(500)
         		   	}
            		}

		else {
			 for (fieldx = 0; fieldx < 10; fieldx++) {
          			 take_stepsxleft();
                delay_ms(500)
                Output_Bit(Pin_A4, 1);
           		delay_ms(200);
           		Output_Bit(Pin_A4, 0);
           		delay_ms(500)
            			}
		}

               take_stepsyfwd();

	}

            for (fieldy = 0; fieldy < 10; fieldy++) {
            take_stepsybk();
            }

   output_on_port_a(steps[0]);
   output_on_port_b(steps[0]);
}




///////////////////////////////////////
//////////////////////////////////////
// PIC16F84A goes here at RESET


void main() {

      enable_interrupts(INT_EXT);
      enable_interrupts(GlOBAL);
      EXT_INT_EDGE(H_TO_L);


      init_ports();           // Initialize ports



// NOTE: All input switches are active LOW

while (TRUE) {

   if(!INPUT(PIN_B7) && !INPUT(PIN_B6)) {
         LENGTHENY();   //  Operator has choosen y axis (B7 low) and pushed lengthen button
      }

   else if(!INPUT(PIN_B7) && !INPUT(PIN_B5)) {
         SHORTENY();   //  Operator has choosen y axis (B7 low) and pushed shorten button
      }

   else if(INPUT(PIN_B7) && !INPUT(PIN_B5)) {
         SHORTENX();  //  Operator has choosen x axis (B7 high) and pushed shorten button
      }

   else if(INPUT(PIN_B7) && !INPUT(PIN_B6)) {
         LENGTHENX();  //  Operator has choosen x axis (B7 high) and pushed lengthen button
      }

}



}



