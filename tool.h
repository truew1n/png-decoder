#pragma once

void bin(char *str, uint8_t num)
{
    for(int i = 0; i < 8; ++i){
        if(num > 0) {
            str[7 - i] = (48 + (num % 2));
            num /= 2;
        } else str[7 - i] = 48;
    }
    str[8] = '\0';
}