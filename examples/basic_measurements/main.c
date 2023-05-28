#include "pzem004tv3.h"
#include "esp_log.h"

/* Declare Functions */
void PMonTask( void * pz );


static const char * TAG = "APP_MAIN";

/* @brief Set ESP32  Serial Configuration */
pzemSetup_t pzConf =
{
    .pzem_uart      = UART_NUM_2,           /*  <== */
    .pzem_rx_pin    = GPIO_NUM_16,          /*  <== */
    .pzem_tx_pin    = GPIO_NUM_17,          /*  <== */
    .pzem_uart_mode = UART_MODE_UART,
    .pzem_addr      = PZ_DEFAULT_ADDRESS,
};

/* Declare destination to store measured values */
_currentValues_t pzValues;            /* Measured values */



void app_main()
{
    esp_log_level_set( "*", ESP_LOG_INFO );
    PzemInit( &pzConf );

    /* if( PzResetEnergy( &pzConf ) ) */
    /* { */
    /*     ESP_LOGI( TAG, "Energy counter succesfully reset" ); */
    /* } */

    xTaskCreate( PMonTask, "PowerMon", ( 256 * 9 ), NULL, 1, NULL );
}


void PMonTask( void * pz )
{
    
    for( ;; )
    {
        PzemGetValues( &pzConf, &pzValues );
        ESP_LOGI( TAG, "Vrms: %.1fV - Irms: %.3fA - Prms: %.1fW -  E: %.0fWh", pzValues.voltage, pzValues.current, pzValues.power, pzValues.energy );
        ESP_LOGI( TAG, "Freq: %.1fHz - PF: %.2f", pzValues.frequency, pzValues.pf );

        /*  ESP_LOGV( DTAG, "HWM: %ld", ( unsigned long int ) uxTaskGetStackHighWaterMark( NULL ) );     / * Show's what's left of the specified stacksize * / */

        vTaskDelay( pdMS_TO_TICKS( 5000 ) );
    }
}
