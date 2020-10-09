/**
 * 
 *  Detecting Heartbeats using AD8232 sensor
 *  Author: Pedro Linhares <pedrolinhares@unifei.edu.br>
 *  
 */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"
#include "driver/adc.h"
#include "driver/timer.h"
#include <zscore.h>
#include "freertos/queue.h"

#define TIMER_INTR_SEL TIMER_INTR_LEVEL
#define TIMER_GROUP TIMER_GROUP_0
#define TIMER_DIVIDER 80
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_FINE_ADJ (0*(TIMER_BASE_CLK / TIMER_DIVIDER)/1000000)
//Define the timer interrupt interval (in seconds). This will determine the sample rate.
#define TIMER_INTERVAL0_SEC (0.01)

//Queue Handle
xQueueHandle queue;

//Timer ISR
void IRAM_ATTR timer_group0_isr(void *p)
{
    int timer_idx = (int) p;
    int value;
    esp_err_t r = ESP_OK;
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0)
    {
        TIMERG0.hw_timer[timer_idx].update = 1;
        TIMERG0.int_clr_timers.t0 = 1;
        TIMERG0.hw_timer[timer_idx].config.alarm_en = 1;
        if(gpio_get_level(19) == 1 || gpio_get_level(18) == 1)
        {
            value = -1;
        }else { 
            r = adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &value);
        }
        if(r == ESP_OK)
            xQueueSendFromISR(queue, &value, &xHigherPriorityTaskWoken);
    }
}

//Timer init function
void timer0_init()
{
    int timer_group = TIMER_GROUP_0;
    int timer_idx = TIMER_0;
    timer_config_t config;
    config.alarm_en = 1;
    config.auto_reload = 1;
    config.counter_dir = TIMER_COUNT_UP;
    config.divider = TIMER_DIVIDER;
    config.intr_type = TIMER_INTR_SEL;
    config.counter_en = TIMER_PAUSE;
    timer_init(timer_group, timer_idx, &config);
    timer_pause(timer_group, timer_idx);
    timer_set_counter_value(timer_group, timer_idx, 0x00000000ULL);
    timer_set_alarm_value(timer_group, timer_idx, (TIMER_INTERVAL0_SEC * TIMER_SCALE) - TIMER_FINE_ADJ);
    timer_enable_intr(timer_group, timer_idx);
    timer_isr_register(timer_group, timer_idx, timer_group0_isr, (void*) timer_idx, ESP_INTR_FLAG_IRAM, NULL);
    timer_start(timer_group, timer_idx);
}

//Our main task
void task_heart(void *pvParameter)
{
    int value = 0;
    double r;
    bool last_detected = false;
    for(;;)
    {
        //We wait one second to receive sensor data from the timer ISR. This task remains blocked while it waits for the data.
        if(xQueueReceive(queue, &value, (TickType_t)(1000/portTICK_RATE_MS)) == pdTRUE)
        {
            r = zscore_process(value);
            //If the z-score is one, we have detected a R wave
            if(r == 1)
            {
                //We check if a wave was already detected the last round to avoid duplicate detections
                if(!last_detected)
                {
                    printf("Heartbeat detected\n");
                    //Turn on the LED
                    gpio_set_level(2, 1);
                    last_detected = true;
                }
            }else {
                //If no R wave was detected we turn off the LED
                gpio_set_level(2, 0);
                last_detected = false;
            }
        }
    }
}

//Program starting point
void app_main(void)
{
    //Creating the queue
    queue = xQueueCreate(10, sizeof(int));
    //Initializing the z-score library. You can change the parameters according to your results
    zscore_init(30, 5, 1);
    //Configuring IO pins
    gpio_pad_select_gpio(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(19);
    gpio_set_direction(19, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(18);
    gpio_set_direction(18, GPIO_MODE_INPUT);
    gpio_set_level(2, 0);
    //Configuring ADC attenuation
    adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_11db);
    //Initializing Timer0
    timer0_init();
    //Creating the main processing task
    xTaskCreate(&task_heart, "heartbeat_detector", 2048, NULL, 5, NULL);
}