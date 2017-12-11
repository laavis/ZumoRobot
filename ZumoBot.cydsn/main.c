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

void checkState(int *state, uint16 *l1,uint16 *r1); //decleration

# define DELAY 1
# define SPEED 255
int main()
{
    struct sensors_ ref;
    struct sensors_ dig;
    struct sensors_ white;
    struct sensors_ black;
    CyGlobalIntEnable; 
    UART_1_Start();
    ADC_Battery_Start(); 
    sensor_isr_StartEx(sensor_isr_handler);
    unsigned int IR_val;
    reflectance_start();
    int pressed = 0;
    IR_led_Write(1);
    int16 adcresult =0;
    float volts = 0.0;
    int boolean = 0;
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
    reflectance_set_threshold(white.l3 + (black.l3 - white.l3)/2 ,white.l1 + (black.l1 - white.l1)/2 ,white.r1 + (black.r1 - white.r1)/2 ,white.r3 + (black.r3 - white.r3)/2);
    
    CyDelay(1000);
    
    IR_led_Write(1);
    printf("Press button to drive to startline.\n");
    while(SW1_Read() == 1) {    //wait till button is pressed
        CyDelay(10);
    }
    
    CyDelay(500);
    
    IR_led_Write(1);
    motor_start();
    
    do {
        motor_turn(100, 100, DELAY);    //drive to start line
        reflectance_digital(&dig);
    } while(dig.l3 == 1 && dig.r3 == 1);    //outer sensors are white
    
    motor_stop();
    
    printf("Get ir signal to start race.\n");
    while(pressed == 0){
    IR_val = get_IR();
        if(IR_val){
            pressed = 1;
        }
    }
    int stop = 0;
    int currentState;
    int onLine =0;
    float leftDiv, rightDiv;
    motor_start();
    int leftSpeed = 0, rightSpeed = 0;
    
    motor_turn(SPEED, SPEED, 200);
    
    for(;;)
    {
        if(volts<4){
            BatteryLed_Write(1);
        }
        else{
             BatteryLed_Write(0);
        }
        reflectance_read(&ref);//print out each period of reflectance sensors  
        reflectance_digital(&dig);      //print out 0 or 1 according to results of reflectance period
        checkState(&currentState, &dig.l1, &dig.r1); //0 == turn right, 1 == turn left.
        if (ref.l1 > ref.r1) { // adjust the values of the turn speeds depending on the sharpness of the turn by creating leftDiv and rightDiv
            leftDiv = ((float)ref.r1 / ref.l1);
            rightDiv = 1;
        }
        else {
            rightDiv = ((float)ref.l1 / ref.r1);
            leftDiv = 1;
        }
        
        if(dig.r3 == 0 && dig.l3 == 0 && onLine == 0 ) // when the leftmost and the rightmost and the zumo enter the black line, if it is the last line the zumo stops
        {
            stop++;
            onLine =1;
            if(stop >= 2){
                motor_stop();
            }
        }
        if(dig.r3 == 1 && dig.l3 == 1 && onLine == 1) // when the outer sensers of the zumo go on white after the black line set onLine to 0
        {
            onLine = 0;   
        }
        if (currentState == 0) { //turn right sharp
            leftSpeed = SPEED;
            rightSpeed = SPEED/4;
            motor_sharpright(leftSpeed,rightSpeed,DELAY);
        }
        else if (currentState == 1) {   //turn left sharp
            leftSpeed = SPEED/4;
            rightSpeed = SPEED;
            motor_sharpleft(leftSpeed,rightSpeed,DELAY);
        }
        else if(currentState == -1){    //adjust turn
            leftSpeed = SPEED * leftDiv;
            rightSpeed = SPEED * rightDiv;
            motor_turn(leftSpeed, rightSpeed, DELAY);
        }
        else if(currentState == -2){    //forward
            motor_turn(SPEED * leftDiv, SPEED * rightDiv, DELAY);
        }
        CyDelay(DELAY);
    }
}

int previous;
void checkState(int *state, uint16 *l1,uint16 *r1){ //definition
    if (*l1 == 1 && *r1 == 1) {
        if (previous == 0) {
            *state = 0; //turn right
        }
        else {
            *state = 1;  //turn left
        }
    }
    else if (*l1 == 1) {
        previous = 0; //turn right if line lost
        *state = -1;
    }
    else if (*r1 == 1) {
        previous = 1;   //turn left if line lost
        *state = -1;
    }
    else *state = -2; //all black, dont turn
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
