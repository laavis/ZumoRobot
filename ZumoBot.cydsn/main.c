/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

#include <project.h>
#include <stdio.h>
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "I2C_made.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "IR.h"
#include "Ambient.h"
#include "Beep.h"

int rread(void);

/**
 * @file    main.c
 * @brief   
 * @details  ** You should enable global interrupt for operating properly. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/


//battery level//
/*int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    ADC_Battery_Start();        
    int16 adcresult =0;
    float volts = 0.0;
    int boolean = 0;

    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board

    for(;;)
    {
        
        ADC_Battery_StartConvert();
        
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for get ADC converted value
            adcresult = ADC_Battery_GetResult16();
            volts = (float32)adcresult*7.522/4095;
            //volts = ADC_Battery_CountsTo_Volts(adcresult);                  // convert value to Volts
        
            // If you want to print value
            printf("%d %f\r\n",adcresult, volts);
        }
       
        if(volts<4){
            if(boolean == 0){
            BatteryLed_Write(1);
            boolean =1;
            }
            else{
             BatteryLed_Write(0);
            boolean =0;}
            

        }
        if(volts>=4){
        BatteryLed_Write(0);}
         CyDelay(500);
        
    }
 }  */ 
//*/

//ultra sonic sensor//
/*
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    Ultra_Start();                          // Ultra Sonic Start function
    while(1) {
        //If you want to print out the value  
        printf("distance = %5.0f\r\n", Ultra_GetDistance());
        CyDelay(1000);
    }
}   
//*/

//IR receiver//
/*
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    
    unsigned int IR_val; 
    
    for(;;)
    {
       IR_val = get_IR();
       printf("%x\r\n\n",IR_val);
    }    
 }   
//*/

# define DELAY 10
# define SPEED 50
int main()
{
    struct sensors_ ref;
    struct sensors_ dig;
    struct sensors_ white;
    struct sensors_ black;
    CyGlobalIntEnable; 
    UART_1_Start();
    sensor_isr_StartEx(sensor_isr_handler);
    
    reflectance_start();

    IR_led_Write(1);
    //CyDelay(5);
    //valk 5800, 3630,4000,8820
    // must 23999,
    //reflectance_set_threshold(14899 ,13814 ,13999 ,16409);
    //raja arvo = valk + (musta-valk)/2
    
    printf("\nPress button while on white.\n");
    while (SW1_Read() == 1) { //read the center button: 0 is pressed and 1 is not
        CyDelay(10);
        reflectance_read(&white);
        printf("%5d %5d %5d %5d\r", white.l3, white.l1, white.r1, white.r3);
    }
    IR_led_Write(0);

    CyDelay(1000);  //wait 5 sec so that user does not read black as well by accident
    
    IR_led_Write(1);
    printf("\nPress button while on black.\n");
    while (SW1_Read() == 1) { //read the center button: 0 is pressed and 1 is not
        CyDelay(10);
        reflectance_read(&black);
        printf("%5d %5d %5d %5d\r", black.l3, black.l1, black.r1, black.r3);
    }
    IR_led_Write(0);
    
    //calculate and set sensor thresholds
    reflectance_set_threshold(white.l3 + black.l3/2 ,white.l1 + black.l1/2 ,white.r1 + black.r1/2 ,white.r3 + black.r3/2);
    
    CyDelay(1000);
    
    IR_led_Write(1);
    printf("Press button to drive to startline.\n");
    while(SW1_Read() == 1) {    //wait till button is pressed
        CyDelay(10);
    }
    IR_led_Write(0);
    
    CyDelay(500);
    
    IR_led_Write(1);
    motor_start();
    
    do {
        motor_turn(SPEED, SPEED, DELAY);    //drive to start line
        reflectance_digital(&dig);
    } while(dig.l3 == 1 && dig.r3 == 1);    //outer sensors are white
    
    motor_stop();
    
    printf("Press button to start race.\n");
    while(SW1_Read() == 1) {    //wait till button is pressed
        CyDelay(10);
    }
    
    //int current_state = 0;
    float division;
    motor_start();
    for(;;)
    {
        reflectance_read(&ref);
        printf("%d %d %d %d \n", ref.l3, ref.l1, ref.r1, ref.r3);       //print out each period of reflectance sensors
        reflectance_digital(&dig);      //print out 0 or 1 according to results of reflectance period
        printf("%d %d %d %d \n", dig.l3, dig.l1, dig.r1, dig.r3);        //print out 0 or 1 according to results of reflectance period
        
        division = (float)ref.l1/ref.r1;
        
        int leftSpeed = SPEED* (1/division);
        int rightSpeed = SPEED* division;
        printf("Division: %f - Right: %d - Left: %d\n", division, rightSpeed, leftSpeed);
        CyDelay(DELAY);
        
        motor_turn(leftSpeed, rightSpeed, DELAY);
        
    }
}

#if 0
int rread(void)
{
    SC0_SetDriveMode(PIN_DM_STRONG);
    SC0_Write(1);
    CyDelayUs(10);
    SC0_SetDriveMode(PIN_DM_DIG_HIZ);
    Timer_1_Start();
    uint16_t start = Timer_1_ReadCounter();
    uint16_t end = 0;
    while(!(Timer_1_ReadStatusRegister() & Timer_1_STATUS_TC)) {
        if(SC0_Read() == 0 && end == 0) {
            end = Timer_1_ReadCounter();
        }
    }
    Timer_1_Stop();
    
    return (start - end);
}
#endif

/* Don't remove the functions below */
int _write(int file, char *ptr, int len)
{
    (void)file; /* Parameter is not used, suppress unused argument warning */
	int n;
	for(n = 0; n < len; n++) {
        if(*ptr == '\n') UART_1_PutChar('\r');
		UART_1_PutChar(*ptr++);
	}
	return len;
}

int _read (int file, char *ptr, int count)
{
    int chs = 0;
    char ch;
 
    (void)file; /* Parameter is not used, suppress unused argument warning */
    while(count > 0) {
        ch = UART_1_GetChar();
        if(ch != 0) {
            UART_1_PutChar(ch);
            chs++;
            if(ch == '\r') {
                ch = '\n';
                UART_1_PutChar(ch);
            }
            *ptr++ = ch;
            count--;
            if(ch == '\n') break;
        }
    }
    return chs;
}
/* [] END OF FILE */
