#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "hal/gpio_hal.h"
#include "hal/uart_ll.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "crc16.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RX_BUF_SIZE      8
#define TX_BUF_SIZE      8
#define RESP_BUF_SIZE    25
#define UPDATE_TIME      200

typedef struct pz_conf_t {
    uart_port_t pzem_uart;
    uint8_t pzem_rx_pin;
    uint8_t pzem_tx_pin;
    uint8_t pzem_addr;
} pzem_setup_t;

typedef struct _current_values {
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
    uint16_t alarms;
} _current_values_t;         /* Measured values */

void PzemInit( pzem_setup_t *pzSetup );
bool PzemCheckCRC( const uint8_t *buf, uint16_t len );
uint16_t PzemReceive( pzem_setup_t *pzSetup, uint8_t *resp, uint16_t len );
bool PzemSendCmd8( pzem_setup_t *pzSetup, uint8_t cmd, uint16_t rAddr, uint16_t val, bool check, uint16_t slave_addr );
void PzemSetCRC( uint8_t *buf, uint16_t len );
bool PzemGetValues( pzem_setup_t *pzSetup, _current_values_t *pmonValues );
uint8_t PzReadAddress( pzem_setup_t *pzSetup, bool update );
bool PzResetEnergy( pzem_setup_t *pzSetup );
void PzemZeroValues( _current_values_t *currentValues );

#define millis( x )              ( esp_timer_get_time( x ) / 1000 )
//#define UART_LL_GET_HW( num )    ( ( ( num ) == 0 ) ? ( &UART0 ) : ( ( ( num ) == 1 ) ? ( &UART1 ) : ( &UART2 ) ) )


#define PZ_DEFAULT_ADDRESS    0xF8
#define PZ_BAUD_RATE          9600
#define PZ_READ_TIMEOUT       100

/*
 * REGISTERS
 */
#define RG_VOLTAGE            0x0000
#define RG_CURRENT_L          0x0001 /* Lo Bit */
#define RG_CURRENT_H          0X0002 /* Hi Bit */
#define RG_POWER_L            0x0003 /* Lo Bit */
#define RG_POWER_H            0x0004 /* Hi Bit */
#define RG_ENERGY_L           0x0005 /* Lo Bit */
#define RG_ENERGY_H           0x0006 /* Hi Bit */
#define RG_FREQUENCY          0x0007
#define RG_PF                 0x0008
#define RG_ALARM              0x0009

#define CMD_RHR               0x03
#define CMD_RIR               0X04
#define CMD_WSR               0x06
#define CMD_CAL               0x41
#define CMD_REST              0x42

#define WREG_ALARM_THR        0x0001
#define WREG_ADDR             0x0002

#define INVALID_ADDRESS       0x00


#ifdef __cplusplus
}
#endif
