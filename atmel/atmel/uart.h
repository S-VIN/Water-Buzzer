#define SOFT_TX_PIN (1<<PB1) // PB1 будет работать как TXD
#define SOFT_TX_PORT PORTB
#define SOFT_TX_DDR DDRB

inline void uart_tx_init () {
    TCCR0A = 1 << WGM01;        // compare mode
    TCCR0B = 1 << CS01;     // prescaler 8
    SOFT_TX_PORT |= SOFT_TX_PIN;
    SOFT_TX_DDR |= SOFT_TX_PIN;
    OCR0A = 100;            //9600 baudrate at prescaler 8
}

//bitbanged UART transmit byte
inline void uart_send_byte (unsigned char data) {
    unsigned char i;
    TCCR0B = 0;
    TCNT0 = 0;
    TIFR |= 1 << OCF0A;
    TCCR0B |= (1 << CS00);
    TIFR |= 1 << OCF0A;
    SOFT_TX_PORT &= ~SOFT_TX_PIN;
    while (!(TIFR & (1 << OCF0A)));
    TIFR |= 1 << OCF0A;
    for (i = 0; i < 8; i++) {
        if (data & 1) {
            SOFT_TX_PORT |= SOFT_TX_PIN;
        } else {
            SOFT_TX_PORT &= ~SOFT_TX_PIN;
        }
        data >>= 1;
        while (!(TIFR & (1 << OCF0A)));
        TIFR |= 1 << OCF0A;
    }
    SOFT_TX_PORT |= SOFT_TX_PIN;
    while (!(TIFR & (1 << OCF0A)));
    TIFR |= 1 << OCF0A;
}

// Функция вывода содержимого переменной
inline void num_to_str(unsigned int value, unsigned char nDigit) {
    switch (nDigit) {
    case 4:
        uart_send_byte((value / 1000) + '0');
    case 3:
        uart_send_byte(((value / 100) % 10) + '0');
    case 2:
        uart_send_byte(((value / 10) % 10) + '0');
    case 1:
        uart_send_byte((value % 10) + '0');
    }
}

inline void uart_print(char* str) {
    uint8_t i = 0;
    while (str[i]) {
        uart_send_byte(str[i++]);
    }
}

