//PB0 - питание фоторезистора
//PB1 - питание пищалки
//PB2 - питание водного датчика
//PB3 - данные с фоторезистора
//PB4 - данные водного датчика
//PB5 - ресет пин

#define MEASUREMENT_TIME 100

#define PB0_HIGH PORTB |= 0b00000001
#define PB1_HIGH PORTB |= 0b00000010
#define PB2_HIGH PORTB |= 0b00000100
#define PB3_HIGH PORTB |= 0b00001000
#define PB4_HIGH PORTB |= 0b00010000
#define PB5_HIGH PORTB |= 0b00100000

#define PB0_LOW PORTB &= 0b11111110
#define PB1_LOW PORTB &= 0b11111101
#define PB2_LOW PORTB &= 0b11111011
#define PB3_LOW PORTB &= 0b11110111
#define PB4_LOW PORTB &= 0b11101111
#define PB5_LOW PORTB &= 0b11011111

inline void SetupPins() {
    //       x              - Биты отвечают за назначение пинов
    //       |x             - 1 - OUTPUT; 0 - INPUT
    //       ||DDB5         - PB5
    //       |||DDB4        - PB4
    //       ||||DDB3       - PB3
    //       |||||DDB2      - PB2
    //       ||||||DDB1     - PB1
    //       |||||||DDB0    - PB0
    //       ||||||||
    DDRB = 0b00100111;
}

inline void SetupADCSRA() {
    //         ADEN           - Подать питание на АЦП
    //         |ADSC          - Запуск АЦП
    //         ||ADATE        - Оцифровка по срабатываению триггера ADTS[2:0]
    //         |||ADIF        - флаг прерывания от компаратора
    //         ||||ADIE       - разрешение прерывания от компаратора
    //         |||||ADPS2     - настройка пределителя (влияет на время измерения)
    //         ||||||ADPS1    -
    //         |||||||ADPS0   -
    //         ||||||||
    ADCSRA = 0b10000111;
}

inline void SetupADMUX(int src) {
    // таблица 1
    //  REFS2  REFS1  REFS0  - настраивает источник опорного напряжения
    //    x      0      0    - VCC опорное напряжение
    //    x      0      1    - опорное напряжение на AREF
    //    0      1      0    - внутреннее 1.1В опорное напряжение
    //    0      1      1    - зарезервировано
    //    1      1      0    - внутреннее 2.56В опорное напряжение без конденсатора
    //    1      1      1    - внутреннее 2.56В опорное напряжение с конденсатором на пине AREF

    // таблица 2
    //  MUX3 MUX2 MUX1 MUX0 -  биты отвечающие за мультиплексор (к чему подключен АЦП)
    //    0    0    0    0  - PB5
    //    0    0    0    1  - PB2
    //    0    0    1    0  - PB4
    //    0    0    1    1  - PB3
    // следующие нужны для калибровки (не использовать)
    //    0    1    0    0  - PB4 PB4 x1
    //    0    1    0    1  - PB4 PB4 x20
    //    0    1    1    0  - PB4 PB3 x1
    //    0    1    1    1  - PB4 PB3 x20
    //    1    0    0    0  - PB5 PB5 x1
    //    1    0    0    1  - PB5 PB5 x20
    //    1    0    1    0  - PB5 PB2 x1
    //    1    0    1    1  - PB5 PB2 x20
    //    1    1    0    0  - Vbg
    //    1    1    0    1  - GND
    //    1    1    1    0  - N/A
    //    1    1    1    1  - ADC4

    //        REFS1           - таблица 1
    //        |REFS0          - таблица 1
    //        ||ADLAR         - бит отвечающий за порядок записи битов результата в регистры (0 - 10 бит; 1 - 8 бит)
    //        |||REFS2        - таблица 1
    //        ||||MUX3        - таблица 2
    //        |||||MUX2       - таблица 2
    //        ||||||MUX1      - таблица 2
    //        |||||||MUX0     - таблица 2
    //        ||||||||
    ADMUX = 0b00100000 | (src & 0b00001111);
}

inline int ReadADC(int number_of_port) {
    SetupADMUX(number_of_port);
    SetupADCSRA();
    ADCSRA |= (1 << ADSC);          // начали измерение
    while(ADCSRA & (1 << ADSC));    // ждем когда помереет
    int result = ADCH;              // отбросили шумные незначащие биты
    ADCSRA = 0b00000111;            // выключили АЦП
    return result;
}

inline int ReadWater() {
    PB2_HIGH;
    delay_ms(MEASUREMENT_TIME);
    int result = ReadADC(2);
    PB2_LOW;
    return result;
}

inline int ReadLight() {
    PB0_HIGH;
    delay_ms(MEASUREMENT_TIME);
    int result = ReadADC(3);
    PB0_LOW;
    return result;
}