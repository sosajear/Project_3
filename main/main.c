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



void app_main()
{
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
 
    while(1){

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



