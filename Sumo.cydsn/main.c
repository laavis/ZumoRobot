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
#include <time.h>
#include <stdlib.h>

int rread(void);

/**
 * @file    main.c
 * @brief   
 * @details  ** You should enable global interrupt for operating properly. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/

# define DELAY 1
# define SPEED 255
int main()
{
    
    struct sensors_ ref;
    struct sensors_ dig;
    struct sensors_ white;
    struct sensors_ black;
    CyGlobalIntEnable; 
    ADC_Battery_Start();    
    int16 adcresult =0;
    float volts = 0.0;
    UART_1_Start();
    sensor_isr_StartEx(sensor_isr_handler);
    Ultra_Start();
    reflectance_start();
    unsigned int IR_val;
    int pressed = 0;
    int mode = 0; 
    srand(time(NULL)); //initializing the random seed
    int r =1; //initializing r as a random number
    IR_led_Write(1);
    ADC_Battery_StartConvert();
        
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for get ADC converted value
            adcresult = ADC_Battery_GetResult16();
            volts = (float32)adcresult*7.522/4095;
        }
        if(volts<4){
           BatteryLed_Write(1);
        }
        else{
           BatteryLed_Write(0);
        }
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
        motor_turn(100, 100, 5);    //drive to start line
        reflectance_read(&ref);
        reflectance_digital(&dig);
    } while(dig.l3 == 1 && dig.r3 == 1);    //outer sensors are white
    
    motor_stop();
    
    //Get ir signal to begin battle
    
     while(pressed == 0){
        IR_val = get_IR();
        if(IR_val){
            pressed = 1;
        }
    }
   
    float kP = 1.15;
    float leftDiv, rightDiv;
    motor_start();
    motor_forward(SPEED,400);
  
    for(;;)
    {
        if(volts<4){  
            BatteryLed_Write(1);
        }
        if(volts>=4){
            BatteryLed_Write(0);
        }
        reflectance_read(&ref);  //read out each period of reflectance sensors
        reflectance_digital(&dig);  //read 0 or 1 according to results of reflectance period    
        if(dig.l1 == 0 && dig.r1 == 0){ // if both of the middle sensors are on black go backwards for 500 ms
            motor_backward(SPEED,500);
            CyDelay(500);
            
        }
        else if(dig.r3 == 0)//if only the far right sensor is on black turn left sharp for 100  ms or 90 degrees
        {
            motor_sharpleft(SPEED,SPEED,100);
            CyDelay(100);
            motor_forward(SPEED,200);
            CyDelay(200);
        }
        else if(dig.l3 == 0) //if only the far left sensor is on black turn right sharp for 100  ms or 90 degrees
        {
            motor_sharpright(SPEED,SPEED,100);
            CyDelay(100);
            motor_forward(SPEED,200);
            CyDelay(200);
        }
        else if(dig.r1 == 0){ // if the right middle sensor is on black turn sharp left for 180 ms or 180 degrees
            motor_sharpleft(SPEED,SPEED,180);
            CyDelay(180);
            motor_forward(SPEED,200);
            CyDelay(200);
        }
        else if(dig.l1 == 0){ // if the left middle sensor is on black turn sharp right for 180 ms or 180 degrees
            motor_sharpright(SPEED,SPEED,180);
            CyDelay(180);
            motor_forward(SPEED,200);
            CyDelay(200);
        }
        else if(Ultra_GetDistance() <15){ // if the ultrasound gets  a distance of under 15 centimeters to an object(hopefully another zumo) then it goes straight towards it and tries to push it out
            motor_forward(SPEED,DELAY);
             CyDelay(DELAY);
        }
        else{ // set a random mode that do different things
            r = rand() % 700; // r is a random number from 0 to 500 
            if(mode == 3 ){ // spin in place to the left
                motor_sharpleft(SPEED,SPEED,DELAY);
                CyDelay(DELAY);
            }
            else if(mode == 1|| mode == 2){// go straight forward
                motor_turn(SPEED,SPEED,DELAY);
                CyDelay(DELAY);
            }
            else if(mode == 4){// spin in place towards the right
                motor_sharpright(SPEED,SPEED,DELAY);
                CyDelay(DELAY);
            
            }
            else if(mode == 5){ // turn in a small circle to the left
                motor_sharpleft(50,SPEED,DELAY);
                CyDelay(DELAY);
            }
             else if(mode == 6){ // turn in a small circle towards the right
                motor_sharpright(SPEED,50,DELAY);
                CyDelay(DELAY);
            }
            if(r>0 && r<7){// if the mode exists go set mode to the value of r
                mode = r; 
            
                ADC_Battery_StartConvert();
            
                if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for get ADC converted value
                    adcresult = ADC_Battery_GetResult16();
                    volts = (float32)adcresult*7.522/4095;
        }
                
            }
        }
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
