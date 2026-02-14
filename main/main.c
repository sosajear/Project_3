#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>
#include <hd44780.h>
#include <esp_idf_lib_helpers.h>
#include <inttypes.h>
#include <stdio.h>

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"

//Delay function declaration
void delay_ms(int t);

#define ADC_CHANNEL_1   ADC1_CHANNEL_0  // GPIO1
#define ADC_CHANNEL_2   ADC1_CHANNEL_1  // GPIO2
#define ADC_ATTEN       ADC_ATTEN_DB_11

#define D_WEIGHT_BUTTON GPIO_NUM_4 //Button for Driver Weight Sensor
#define D_SEATBELT_BUTTON GPIO_NUM_6 //Button for Driver Seatbelt Sensor
#define P_WEIGHT_BUTTON GPIO_NUM_5 //Button for Passenger Weight Sensor
#define P_SEATBELT_BUTTON GPIO_NUM_7 //Button for Passenger Seatbelt Sensor
#define IGNITION_BUTTON GPIO_NUM_12

#define GREEN_LED GPIO_NUM_9
#define BLUE_LED GPIO_NUM_10
#define BUZZER GPIO_NUM_11

#define ADC_CHANNEL_1   ADC1_CHANNEL_0  // GPIO1
#define ADC_CHANNEL_2   ADC1_CHANNEL_1  // GPIO2
#define ADC_ATTEN       ADC_ATTEN_DB_11


esp_adc_cal_characteristics_t adc_chars;
uint32_t v1(){ 
        uint32_t raw1 = adc1_get_raw(ADC_CHANNEL_0);
        uint32_t voltage1 = esp_adc_cal_raw_to_voltage(raw1, &adc_chars);
        
        // Display results
        printf("GPIO1: %lumV\n", voltage1);
        return voltage1;
}
uint32_t v2(){ 
        // Calculate averages and convert to voltage
        uint32_t raw2 = adc1_get_raw(ADC_CHANNEL_2);
        uint32_t voltage2 = esp_adc_cal_raw_to_voltage(raw2, &adc_chars);
        
        // Display results
        //printf(" GPIO2: %lumV\n", voltage2);
        return voltage2;
}

    hd44780_t lcd =
    {
        .write_cb = NULL,
        .font = HD44780_FONT_5X8,
        .lines = 2,
        .pins = {
            .rs = GPIO_NUM_38,
            .e  = GPIO_NUM_37,
            .d4 = GPIO_NUM_36,
            .d5 = GPIO_NUM_35,
            .d6 = GPIO_NUM_48,
            .d7 = GPIO_NUM_47,
            .bl = HD44780_NOT_USED
        }
    };


void lcd_print(char* message){
    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, message);

}



void app_main(void)
{
   //setup for pins as outputs
   gpio_reset_pin(GREEN_LED);
   gpio_set_direction(GREEN_LED, GPIO_MODE_OUTPUT);
   gpio_set_level(GREEN_LED, 0);
   gpio_reset_pin(BLUE_LED);
   gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);
   gpio_set_level(BLUE_LED, 0);
   gpio_reset_pin(BUZZER);
   gpio_set_direction(BUZZER, GPIO_MODE_OUTPUT);
   gpio_set_level(BUZZER, 0);

   //setup for pins as inputs
   gpio_reset_pin(D_WEIGHT_BUTTON);
   gpio_set_direction(D_WEIGHT_BUTTON, GPIO_MODE_INPUT);
   gpio_reset_pin(D_SEATBELT_BUTTON);
   gpio_set_direction(D_SEATBELT_BUTTON, GPIO_MODE_INPUT);
   gpio_reset_pin(P_WEIGHT_BUTTON);
   gpio_set_direction(P_WEIGHT_BUTTON, GPIO_MODE_INPUT);
   gpio_reset_pin(P_SEATBELT_BUTTON);
   gpio_set_direction(P_SEATBELT_BUTTON, GPIO_MODE_INPUT);
   gpio_reset_pin(IGNITION_BUTTON);
   gpio_set_direction(IGNITION_BUTTON, GPIO_MODE_INPUT);


    bool HI, LO, INTT, OFF;
    HI = 0;
    LO = 0;
    INTT = 0;
    OFF = 1;

    bool SHORT, MED, LONG;
    SHORT = 1;
    MED = 0;
    LONG = 0;

    hd44780_clear(&lcd);
    ESP_ERROR_CHECK(hd44780_init(&lcd));

    // Setup ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL_1, ADC_ATTEN);
    adc1_config_channel_atten(ADC_CHANNEL_2, ADC_ATTEN);
    
    // Calibration
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_12, 1100, &adc_chars);

   
   //variables to keep track of the green light and the ignition
   bool green_led_ready = true;
   bool green_led_on = false;
   bool welcome_txt = false;
   bool engine_running = false;
   bool ignition_pressed = false;

   //main body loop
   while (1) //An infinite loop to allow for multiple attempts at ignition
   {
       if (!gpio_get_level(D_WEIGHT_BUTTON) && welcome_txt == false) //senses when driver sits down and welcome him/her to the system
       {
           printf("Welcome to enhanced alarm system model 218-W25\n");
           welcome_txt = true;
       }

       if (green_led_ready && (!gpio_get_level(D_WEIGHT_BUTTON) && !gpio_get_level(D_SEATBELT_BUTTON)) &&
           (!gpio_get_level(P_WEIGHT_BUTTON) && !gpio_get_level(P_SEATBELT_BUTTON))) //senses if all conditions are met for ignition
       {
           gpio_set_level(GREEN_LED, 1);
           green_led_on = true;
       }
       else
       {
           green_led_on = false;
           gpio_set_level(GREEN_LED, 0);
       }

       if (!gpio_get_level(IGNITION_BUTTON) && !ignition_pressed) //ignition button pressed
       {
           ignition_pressed = true;

           if (!engine_running && green_led_on) //ignition started and everything ready
           {
               engine_running = true;
               gpio_set_level(BLUE_LED, 1);
               printf("Engine Started\n");
               green_led_ready = false;
               gpio_set_level(GREEN_LED, 0);
           }
           else if (!engine_running && !green_led_on) //ignition started and not everything ready
           {
               gpio_set_level(BUZZER, 1);
               printf("Ignition Prohibited\n");

               if (gpio_get_level(D_WEIGHT_BUTTON))
               {
                   printf("driver seat not occupied\n");
               }
               if (gpio_get_level(D_SEATBELT_BUTTON))
               {
                   printf("driver seatbelt not on\n");
               }
               if (gpio_get_level(P_WEIGHT_BUTTON))
               {
                   printf("passenger seat not occupied\n");
               }
               if (gpio_get_level(P_SEATBELT_BUTTON))
               {
                   printf("passenger seatbelt not on\n");
               }

               printf("Please ensure that both seatbelts are fastened and both seats are occupied before starting the engine\n");
               delay_ms(2000); //Keeps buzzer on
               gpio_set_level(BUZZER, 0); //turns buzzer off
           }
           else if (engine_running) //engine running and ignition pressed again
           {
               engine_running = false;
               gpio_set_level(BLUE_LED, 0);
               green_led_ready = true;
               welcome_txt = false;
               printf("Engine Stopped\n");
           }
       }

       if (gpio_get_level(IGNITION_BUTTON)) //reset ignition button latch
       {
           ignition_pressed = false;
       }

       
        uint32_t mode = v2();
        uint32_t light = v1();

        if(0 <= mode && mode < 1100 ){ON = 0; OFF = 1, AUTO = 0;}
        if(1100 <= mode && mode < 2200 ){ON = 0; OFF = 0, AUTO = 1;}
        if(2200 <= mode && mode < 3300 ){ON = 1; OFF = 0, AUTO = 0;}
        if(0 <= light && light < 1300 ){DAY = 0;}
        if(1900 <= light && light < 3300 ){DAY = 1;}
        if(ON && engine_running){
            head(1);
        }
        if(OFF && engine_running){        
            head(0);
        }
        if(AUTO && engine_running){
            if(DAY){
        
        delay_ms(2000);
        head(0);
            }
            if(!DAY){
        delay_ms(1000);
        head(1);
            }
        }

        if(!engine_running){     
            head(0);
        }
        delay_ms(20);
uint32_t mode = v1();
        if(0    <= mode && mode < 825 ){OFF = 1, LO = 0, HI = 0, INTT = 0; lcd_print("OFF");}
        if(825  <= mode && mode < 1650 ){OFF = 0, LO = 1, HI = 0, INTT = 0; lcd_print("LOW");}
        if(1650 <= mode && mode < 2475 ){OFF = 0, LO = 0, HI = 1, INTT = 0; lcd_print("HI ");}
        if(2475 <= mode && mode < 3300 ){OFF = 0, LO = 0, HI = 0, INTT = 1; lcd_print("INT");}
uint32_t spd = v2();
        if(0    <= spd && spd < 1100 ){SHORT = 1, MED = 0, LONG = 0;}
        if(1100 <= spd && spd < 2200 ){SHORT = 0, MED = 1, LONG = 0;}
        if(2200 <= spd && spd < 3300 ){SHORT = 0, MED = 0, LONG = 1;}
   }
}

//A function to specify delays in milliseconds
void delay_ms(int t)
{
   vTaskDelay(t / portTICK_PERIOD_MS);
}