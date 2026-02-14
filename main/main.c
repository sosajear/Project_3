#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"

//Delay function declaration
void delay_ms(int t);
void low_wiper(void *pvParameter);
void high_wiper(void *pvParameter);
void intermittent_wiper(void *pvParameter);

TaskHandle_t high_handle = NULL; //task handles to remove errors
TaskHandle_t low_handle = NULL;
TaskHandle_t int_handle = NULL;

static volatile int intermittent_delay = 1;

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (16)
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT

#define LEDC_FREQUENCY          (50) // Frequency in Hertz. 
#define LEDC_DUTY_MIN           (307) // Set duty to 3.75%.
#define LEDC_DUTY_MAX           (614) // Set duty to 7.5%


#define D_WEIGHT_BUTTON GPIO_NUM_4 //Button for Driver Weight Sensor
#define D_SEATBELT_BUTTON GPIO_NUM_6 //Button for Driver Seatbelt Sensor
#define P_WEIGHT_BUTTON GPIO_NUM_5 //Button for Passenger Weight Sensor
#define P_SEATBELT_BUTTON GPIO_NUM_7 //Button for Passenger Seatbelt Sensor
#define IGNITION_BUTTON GPIO_NUM_12 

#define GREEN_LED GPIO_NUM_9
#define BLUE_LED GPIO_NUM_10
#define BUZZER GPIO_NUM_11

#define HEAD_1 GPIO_NUM_13
#define HEAD_2 GPIO_NUM_14

#define ADC_CHANNEL_1   ADC1_CHANNEL_0  // GPIO1
#define ADC_CHANNEL_2   ADC1_CHANNEL_1  // GPIO2
#define ADC_ATTEN       ADC_ATTEN_DB_11


esp_adc_cal_characteristics_t adc_chars;
uint32_t v1(){ 
        uint32_t raw1 = adc1_get_raw(ADC_CHANNEL_1);
        uint32_t voltage1 = esp_adc_cal_raw_to_voltage(raw1, &adc_chars);
        
        // Display results
        //printf("GPIO1: %lumV\n", voltage1);
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

void head(bool on){
    gpio_set_level(HEAD_1,on);
    gpio_set_level(HEAD_2,on);
}

static void ledc_init(void);

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

    gpio_reset_pin(HEAD_1);
    gpio_set_direction(HEAD_1, GPIO_MODE_OUTPUT);
    gpio_pullup_en(HEAD_1);

    gpio_reset_pin(HEAD_2);
    gpio_set_direction(HEAD_2, GPIO_MODE_OUTPUT);
    gpio_pullup_en(HEAD_2);




    bool DAY, ON, OFF, AUTO;
    DAY = 1;
    ON = 0;
    OFF = 1;
    AUTO = 0;
    //switching = 0;
    // Setup ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL_1, ADC_ATTEN);
    adc1_config_channel_atten(ADC_CHANNEL_2, ADC_ATTEN);
    
    // Calibration
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    
    ledc_init();
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

   }
}

//A function to specify delays in milliseconds
void delay_ms(int t)
{
   vTaskDelay(t / portTICK_PERIOD_MS);
}

void reg_wiper(){
    for(float i = LEDC_DUTY_MIN; i < LEDC_DUTY_MAX; i = i + 4.093){
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, i);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        delay_ms(20);
    }
    for(float i = LEDC_DUTY_MAX; i > LEDC_DUTY_MIN; i = i - 4.093){
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, i);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        delay_ms(20);
    }
}
void low_wiper(void *pvParameters)
{
    while(1){
        reg_wiper();
        vTaskDelete(NULL);
    }
}

void high_wiper(void *pvParameters)
{
    while(1){
        for(float i = LEDC_DUTY_MIN; i < LEDC_DUTY_MAX; i = i + 10.233){
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, i);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            delay_ms(20);
        }
        for(float i = LEDC_DUTY_MAX; i > LEDC_DUTY_MIN; i = i - 10.233){
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, i);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            delay_ms(20);
        }
        vTaskDelete(NULL);
    }
}

void intermittent_wiper(void *pvParameters){
    while(1){
        reg_wiper();
        delay_ms(intermittent_delay*1000);
        vTaskDelete();
    }
}

static void ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 50 Hz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
ledc_channel_config(&ledc_channel);
}

