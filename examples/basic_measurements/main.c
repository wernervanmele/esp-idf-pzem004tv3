#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pzem004tv3.h"

/* @brief Set ESP32  Serial Configuration */
pzemSetup_t pzConf =
{
    .pzem_uart   = UART_NUM_2,              /*  <== Specify the UART you want to use, UART_NUM_0, UART_NUM_1, UART_NUM_2 (ESP32 specific) */
    .pzem_rx_pin = GPIO_NUM_16,             /*  <== GPIO for RX */
    .pzem_tx_pin = GPIO_NUM_17,             /*  <== GPIO for TX */
    .pzem_addr   = PZ_DEFAULT_ADDRESS,      /*  If your module has a different address, specify here or update the variable in pzem004tv3.h */
};

static const char * TAG = "APP_MAIN";
TaskHandle_t PMonTHandle = NULL;
_currentValues_t pzValues;            /* Measured values */

void PMonTask( void * pz );

void app_main()
{
    /* Initialize/Configure UART */
    PzemInit( &pzConf );

    xTaskCreate( PMonTask, "PowerMon", ( 256 * 8 ), NULL, tskIDLE_PRIORITY, &PMonTHandle );
}



void PMonTask( void * pz )
{
    for( ;; )
    {
        PzemGetValues( &pzConf, &pzValues );
        printf( "Vrms: %.1fV - Irms: %.3fA - P: %.1fW - E: %.2fWh\n", pzValues.voltage, pzValues.current, pzValues.power, pzValues.energy );
        printf( "Freq: %.1fHz - PF: %.2f\n", pzValues.frequency, pzValues.pf );

        ESP_LOGI( TAG, "Stack High Water Mark: %ld Bytes free", ( unsigned long int ) uxTaskGetStackHighWaterMark( NULL ) );     /* Show's what's left of the specified stacksize */

        vTaskDelay( pdMS_TO_TICKS( 2500 ) );
    }

    vTaskDelete( NULL );
}
