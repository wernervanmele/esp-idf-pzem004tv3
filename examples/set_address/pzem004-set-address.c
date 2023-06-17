#include <stdio.h>
#include "esp_log.h"
#include "pzem004tv3.h"

#define NEW_ADDRESS     0x03            // <= New Device Address (change is not permanent)

/**
 * @brief ESP32 Uart configuration
 */
pzem_setup_t pzConf = {
    .pzem_uart   = UART_NUM_2,              /*  <== Specify the UART you want to use, UART_NUM_0, UART_NUM_1, UART_NUM_2 (ESP32 specific) */
    .pzem_rx_pin = GPIO_NUM_16,             /*  <== GPIO for RX */
    .pzem_tx_pin = GPIO_NUM_17,             /*  <== GPIO for TX */
    .pzem_addr   = 0xF8,      /*  If your module has a different address, specify here or update the variable in pzem004tv3.h */
};


pzem_setup_t    pzConf;             // Pzem-004t parameter struct

void app_main(void)
{
    static uint8_t new_address = 0;         // new address

    PzemInit( &pzConf );                    // Initialize the module

    // Write the new device address into the register
    if (PzSetAddress(&pzConf, NEW_ADDRESS)) {

        // Update config struct with new address
        pzConf.pzem_addr = NEW_ADDRESS;

        // Validate the address by reading again from the register !
        new_address = PzReadAddress(&pzConf);
        ESP_LOGI("APP_MAIN", "Module's new  address is: 0x%02x", new_address);

    } else {
        ESP_LOGI("[APP_MAIN]", "Address update failed !!");
    }
}
