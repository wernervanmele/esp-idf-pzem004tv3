/**
 * Based on Espressif examples: https://github.com/espressif/esp-idf/blob/master/examples/peripherals/uart/uart_echo/main/uart_echo_example_main.c
 * And ported library from https://github.com/mandulaj/PZEM-004T-v30
 *
 */
#include "pzem004tv3.h"



uint64_t _lastRead = 0; /* Last time values were updated */

/**
 * @brief Initialize the UART, configured via struct pzemSetup_t
 * @param pzSetup
 */
void PzemInit( pzemSetup_t * pzSetup )
{
    static const char * LOG_TAG = "PZ_INIT";


    ESP_LOGI( LOG_TAG, "Initializing UART" );

    const uart_port_t _uart_num = pzSetup->pzem_uart;
    const int uart_buffer_size = ( 1024 * 2 );

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config =
    {
        .baud_rate  = PZ_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    int intr_alloc_flags = 0;

    #if CONFIG_UART_ISR_IN_IRAM
        intr_alloc_flags = ESP_INTR_FLAG_IRAM;
    #endif

    ESP_LOGI( LOG_TAG, "UART set pins, mode and install driver." );

/* Install UART driver using an event queue here */
    ESP_ERROR_CHECK( uart_driver_install( _uart_num, uart_buffer_size, 0, 0, NULL, intr_alloc_flags ) );

    /* Configure UART parameters */
    ESP_ERROR_CHECK( uart_param_config( _uart_num, &uart_config ) );

    /* Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19) */
    ESP_ERROR_CHECK( uart_set_pin( _uart_num, pzSetup->pzem_tx_pin, pzSetup->pzem_rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE ) );
}


/**
 * @brief Read response
 * @param pzSetup
 * @param resp
 * @param len
 * @return
 */
uint16_t PzemReceive( pzemSetup_t * pzSetup,
                      uint8_t * resp,
                      uint16_t len )
{
    static const char * LOG_TAG = "PZ_RECEIVE";

    uint16_t rxBytes = uart_read_bytes( pzSetup->pzem_uart, resp, len, pdMS_TO_TICKS( 500 ) );

    if( rxBytes > 0 )
    {
        resp[ rxBytes ] = 0;
        ESP_LOGV( LOG_TAG, "Read %d  bytes: '%s'", rxBytes, resp );
        ESP_LOG_BUFFER_HEXDUMP( LOG_TAG, resp, rxBytes, ESP_LOG_VERBOSE );
    }

    return rxBytes;
}


/**
 * @brief In case you forgot the address
 * @param update
 * @return
 */
uint8_t PzReadAddress( pzemSetup_t * pzSetup,
                       bool update )
{
    static uint8_t response[ 7 ];
    uint8_t addr = 0;

    /* Read 1 register */
    if( !PzemSendCmd8( pzSetup, CMD_RHR, WREG_ADDR, 0x01, false, 0xFFFF ) )
    {
        return INVALID_ADDRESS;
    }

    if( PzemReceive( pzSetup, response, 7 ) != 7 ) /* Something went wrong */
    {
        return INVALID_ADDRESS;
    }

    /* Get the current address */
    addr = ( ( uint32_t ) response[ 3 ] << 8 | /* Raw address */
             ( uint32_t ) response[ 4 ] );

    /* Update the internal address if desired */
    if( update )
    {
        pzSetup->pzem_addr = addr;
    }

    return addr;
}

bool PzResetEnergy( pzemSetup_t * pzSetup )
{
    uint8_t buffer[] = { 0x00, CMD_REST, 0x00, 0x00 };
    uint8_t reply[ 5 ];

    buffer[ 0 ] = pzSetup->pzem_addr;

    PzemSetCRC( buffer, 4 );
    uart_write_bytes( pzSetup->pzem_uart, buffer, 4 );

    uint16_t length = PzemReceive( pzSetup, reply, 5 );

    if( ( length == 0 ) || ( length == 5 ) )
    {
        return false;
    }

    return true;
}

/**
 * @brief Send 8Bit command
 * @param pzSetup
 * @param cmd
 * @param regAddr
 * @param regVal
 * @param check
 * @param slave_addr
 * @return
 */
bool PzemSendCmd8( pzemSetup_t * pzSetup,
                   uint8_t cmd,
                   uint16_t regAddr,
                   uint16_t regVal,
                   bool check,
                   uint16_t slave_addr )
{
    static const char * LOG_TAG = "PZ_SEND8";

    uint8_t txdata[ TX_BUF_SIZE ];
    uint8_t rxdata[ RX_BUF_SIZE ];

    if( ( slave_addr == 0xFFFF ) ||
        ( slave_addr < 0x01 ) ||
        ( slave_addr > 0xF7 ) )
    {
        slave_addr = pzSetup->pzem_addr;
    }

    txdata[ 0 ] = slave_addr;
    txdata[ 1 ] = cmd;
    txdata[ 2 ] = ( regAddr >> 8 ) & 0xFF;
    txdata[ 3 ] = ( regAddr ) & 0xFF;
    txdata[ 4 ] = ( regVal >> 8 ) & 0xFF;
    txdata[ 5 ] = ( regVal ) & 0xFF;

    PzemSetCRC( txdata, TX_BUF_SIZE );

    const int txBytes = uart_write_bytes( pzSetup->pzem_uart, txdata, TX_BUF_SIZE );

    ESP_LOGV( LOG_TAG, "Wrote %d bytes", txBytes );
    ESP_LOG_BUFFER_HEXDUMP( LOG_TAG, txdata, txBytes, ESP_LOG_VERBOSE );

    if( check )
    {
        if( !PzemReceive( pzSetup, rxdata, RESP_BUF_SIZE ) ) /* if check enabled, read the response */
        {
            return false;
        }

        /* Check if response is same as send */
        for(uint8_t i = 0; i < 8; i++)
        {
            if( txdata[ i ] != rxdata[ i ] )
            {
                return false;
            }
        }
    }

    return true;
}


/**
 * @brief Retreive all measurements
 * @param pzSetup
 * @param currentValues
 * @return
 */
bool PzemGetValues( pzemSetup_t * pzSetup,
                    _currentValues_t * currentValues )
{
    uint8_t respbuff[ RESP_BUF_SIZE ];

    if( ( unsigned long ) ( millis() - _lastRead ) > UPDATE_TIME )
    {
        _lastRead = millis();
    }
    else
    {
        return true;
    }

    /* Read 10 Registers from 0x00 to 0x0A without checking */
    PzemSendCmd8( pzSetup, CMD_RIR, 0x00, 0x0A, false, 0xFFFF );

    if( PzemReceive( pzSetup, respbuff, RESP_BUF_SIZE ) != RESP_BUF_SIZE )  /* Something went wrong */
    {
        return false;
    }

    currentValues->voltage = ( ( uint32_t ) respbuff[ 3 ] << 8 | /* Raw voltage in 0.1V */
                               ( uint32_t ) respbuff[ 4 ] ) / 10.0;

    currentValues->current = ( ( uint32_t ) respbuff[ 5 ] << 8 | /* Raw current in 0.001A */
                               ( uint32_t ) respbuff[ 6 ] |
                               ( uint32_t ) respbuff[ 7 ] << 24 |
                               ( uint32_t ) respbuff[ 8 ] << 16 ) / 1000.0;

    currentValues->power = ( ( uint32_t ) respbuff[ 9 ] << 8 | /* Raw power in 0.1W */
                             ( uint32_t ) respbuff[ 10 ] |
                             ( uint32_t ) respbuff[ 11 ] << 24 |
                             ( uint32_t ) respbuff[ 12 ] << 16 ) / 10.0;

    currentValues->energy = ( ( uint32_t ) respbuff[ 13 ] << 8 | /* Raw Energy in 1Wh */
                              ( uint32_t ) respbuff[ 14 ] |
                              ( uint32_t ) respbuff[ 15 ] << 24 |
                              ( uint32_t ) respbuff[ 16 ] << 16 ) / 1000.0;

    currentValues->frequency = ( ( uint32_t ) respbuff[ 17 ] << 8 | /* Raw Frequency in 0.1Hz */
                                 ( uint32_t ) respbuff[ 18 ] ) / 10.0;

    currentValues->pf = ( ( uint32_t ) respbuff[ 19 ] << 8 | /* Raw pf in 0.01 */
                          ( uint32_t ) respbuff[ 20 ] ) / 100.0;

    currentValues->alarms = ( ( uint32_t ) respbuff[ 21 ] << 8 | /* Raw alarm value */
                              ( uint32_t ) respbuff[ 22 ] );


    return true;
}

/**
 * @brief Add CRC to 8Bit command
 * @param buf
 * @param len
 */
void PzemSetCRC( uint8_t * buf,
                 uint16_t len )
{
    if( len <= 2 ) /* sanity check */
    {
        return;
    }

    uint16_t crc = crc16( buf, len - 2 ); /* CRC of data */

    /* Write high and low byte to last two positions */
    buf[ len - 2 ] = crc & 0xFF;          /* Low byte first */
    buf[ len - 1 ] = ( crc >> 8 ) & 0xFF; /* High byte second */
}

/**
 * @brief Validate CRC
 * @param buf
 * @param len
 * @return
 */
bool PzemCheckCRC( const uint8_t * buf,
                   uint16_t len )
{
    if( len <= 2 )   /* Sanity check */
    {
        return false;
    }

    uint16_t crc = crc16( buf, len - 2 ); /* Compute CRC of data */
    return ( ( uint16_t ) buf[ len - 2 ] | ( uint16_t ) buf[ len - 1 ] << 8 ) == crc;
}
