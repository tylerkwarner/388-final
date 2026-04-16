#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "eecs388_lib.h"


#define BUF_SIZE  (80)
char string_array[BUF_SIZE];
int bytes_read;


void auto_brake(int devid)
{
    uint8_t frame[9];
    int i;

    static uint32_t last_clock = 0; 
    static int led_state = 0;
    // Task1 & 2: 
    // Your code here (Use Lab 02 - Lab 04 for reference)
    // Use the directions given in the project document
    int count = 0;
    while (count <= 100){
        if (ser_isready(devid)){
            if(ser_read(devid) == 0x59){
                if (ser_isready(devid) && ser_read(devid) == 0x59) {
                    frame[0] = 0x59;
                    frame[1] = 0x59;

                    for (i = 2; i <9; i++){
                        while (!ser_isready(devid));
                        frame[i] = ser_read(devid);

                    }

                    int distance = frame[2] + (frame[3] << 8);

                    printf("\nDistance = %d cm", distance);

                    //Turn OFF all LEDs first
                    gpio_write(GREEN_LED, 0);
                    gpio_write(RED_LED, 0);

                    //distance logic
                    if (distance > 200) {
                        //Green
                        gpio_write(GREEN_LED, 1);
                    }
                    else if (distance > 100) {
                        //yellow (red + green)
                        gpio_write(GREEN_LED, 1);
                        gpio_write(RED_LED, 1);
                    }
                    else if (distance > 60) {
                        //red
                        gpio_write(RED_LED, 1);
                    }
                    else {
                        uint32_t current_clock = clock();

                        if ((current_clock - last_clock) >= (CLOCKS_PER_SEC / 10)){
                                last_clock = current_clock;
                                led_state = !led_state;
                            } 

                        gpio_write(RED_LED, led_state);
                        //flashing red
                        // gpio_write(RED_LED, 1);
                        // delay(100);
                        // gpio_write(RED_LED,0);
                        // delay(100);
                    }
                }
            }

        }
          count++;
    }
}

int read_from_pi(int devid)
{
    // Task-3: 
    // You code goes here (Use Lab 09-option1 for reference)
    // After performing Task-2 at dnn.py code, modify this part to read angle values from Raspberry Pi.

    

    int angle = 0; 
        if (ser_isready(devid)) {
            printf("working");
            bytes_read = ser_readline(devid, BUF_SIZE, string_array);
            if (bytes_read > 0) {
                printf("\nUART Received: %s", string_array);
                // Convert string (e.g., "45") to integer
                angle = atoi(string_array);
        }

    }
    return angle;
}

void steering(int gpio, int pos)
{
    
    if (pos < 0) pos = 0; 
    if (pos > 180) pos = 180; 
    

    uint16_t var1 = (544 + ((2400 - 544) * (uint32_t)pos) / 180);
    gpio_write(PIN_19, ON); 
    delay_usec(var1); 
    gpio_write(PIN_19, OFF); 
    delay(16);
    delay_usec(4000 - var1); 
    



    // Task-4: 
    // Your code goes here (Use Lab 05 for reference)
    // Check the project document to understand the task
    

}
int main()
{
    
    static uint32_t last_clock = 0;
    uint32_t lastLidarTime = 0; 
    uint32_t LIDAR_INTERVAL = 5;
    uint32_t lastLedTime = 0; 
    uint32_t LED_INTERVAL = 10; 
    uint32_t lastServoTime = 0; 
    uint32_t SERVO_INTERVAL = 15; 
    // initialize UART channels
    ser_setup(0); // uart0
    ser_setup(1); // uart1

    int pi_to_hifive = 1; //The connection with Pi uses uart 0
    int lidar_to_hifive = 0; //the lidar uses uart 0
    
    printf("\nUsing UART %d for Pi -> HiFive", pi_to_hifive);
    printf("\nUsing UART %d for Lidar -> HiFive", lidar_to_hifive);
    
    //Initializing PINs
    gpio_mode(PIN_19, OUTPUT);
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(BLUE_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);

    printf("Setup completed.\n");
    printf("Begin the main loop.\n");

    while (1) { 
        
        
        uint32_t cur_time = clock(); 
        uint32_t milli_cur_time = (cur_time * 1000) / CLOCKS_PER_SEC;

        if (milli_cur_time - lastLidarTime >= LIDAR_INTERVAL)
        {
            lastLidarTime = milli_cur_time; 
            auto_brake(lidar_to_hifive);
        } 


        

        auto_brake(lidar_to_hifive); // measuring distance using lidar and braking
        int angle = read_from_pi(pi_to_hifive); //getting turn direction from pi
        printf("\nangle=%d", angle);
        int gpio = PIN_19; 
        for (int i = 0; i < 10; i++){
            // Here, we set the angle to 180 if the prediction from the DNN is a positive angle
            // and 0 if the prediction is a negative angle.
            // This is so that it is easier to see the movement of the servo.
            // You are welcome to pass the angle values directly to the steering function.
            // If the servo function is written correctly, it should still work,
            // only the movements of the servo will be more subtle
            // if(angle>0){
            //     steering(gpio, 180);
            // }
            // else {
            //     steering(gpio,0);
            // }
            
            // Uncomment the line below to see the actual angles on the servo.
            // Remember to comment out the if-else statement above!
            steering(gpio, angle);
        }

    }
    return 0;
}
