# PzSetAddress()  

The address change is only valid until the next restart.  
  
Update the modules configuration struct to match your current setup.  


```c
pzem_setup_t pzConf = {
    .pzem_uart   = UART_NUM_2,   /* <= Specify the UART you want to use, UART_NUM_0, UART_NUM_1, UART_NUM_2 (ESP32 specific) */
    .pzem_rx_pin = GPIO_NUM_16,    /*  <== GPIO for RX */
    .pzem_tx_pin = GPIO_NUM_17,    /*  <== GPIO for TX */
    .pzem_addr   = 0xF8, 
};
```
Update the RX/TX GPIO's that are connected to your PZEM-004T module rx/tx pins.  

