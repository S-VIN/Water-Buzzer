inline void highChirp(int intensity, int chirpsNumber) {
    for(int veces = 0; veces <= chirpsNumber; veces++) {
        for (int i = 100; i > 0; i--) {
            for  (int x = 0; x < intensity;  x++) {
                PB1_HIGH;
                delay_us(i);
                PB1_LOW;
                delay_us(i);
            }
        }
    }
}

inline void lowChirp(int intensity, int chirpsNumber) {
    int i;
    int x;

    for(int veces = 0; veces <= chirpsNumber; veces++) {

        for (i = 0; i < 200; i++) {
            PB1_HIGH;
            delay_us(i);
            PB1_LOW;
            delay_us(i);
        }

        for (i = 90; i > 80; i--) {
            for  ( x = 0; x < 5;  x++) {
                PB1_HIGH;
                delay_us(i);
                PB1_LOW;
                delay_us(i);
            }
        }
    }
}

inline void tweet(int intensity, int chirpsNumber) {
    //normal chirpsNumber 3, normal intensity 5
    for(int veces = 0; veces < chirpsNumber; veces++) {

        for (int i = 80; i > 0; i--) {
            for  (int x = 0; x < intensity;  x++) {
                PB1_HIGH;
                delay_us(i);
                PB1_LOW;
                delay_us(i);
            }
        }
    }
}

inline void alarmSound() {
    highChirp(1, 4);
    // delay_ms(100);
    // lowChirp(50, 2);
    // delay_ms(100);
    tweet(2, 1);
}
