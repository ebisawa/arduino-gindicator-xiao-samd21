//
// Created by ebisawa on 2024/03/10.
//

#ifndef GSENSOR_LED_GDEMO_H
#define GSENSOR_LED_GDEMO_H

class GSensorDemo {
    int index;

public:
    void begin();
    void fetch();

    float gx() const;
    float gy() const;
    float gtotal() const;
};

#endif //GSENSOR_LED_GDEMO_H
