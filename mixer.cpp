#include <iostream>
#include <climits>
#include <algorithm>
#include "mixer.h"

#define MAGIC_NUMBER 176.4f

void mixer(
        struct mixer_input* inputs,
        size_t n,
        void* output_buf,
        size_t* output_size,
        unsigned long tx_interval_ms
        )
{
    if (n == 0)
    {
        *output_size = 0;
        return;
    }
    if (*output_size > tx_interval_ms * MAGIC_NUMBER)
        *output_size = tx_interval_ms * MAGIC_NUMBER;

    for (size_t i = 0; i < *output_size; i += sizeof (int16_t))
    {
        int res = 0;
        for (size_t j = 0; j < n; j++)
            if (i < inputs[j].len)
                res += ((int16_t*) inputs[j].data)[i / 2];
        if (res > SHRT_MAX)
            res = SHRT_MAX;
        else if (res < SHRT_MIN)
            res = SHRT_MIN;
        ((int16_t*) output_buf)[i / 2] = (int16_t) res;
    }
    for (size_t i = 0; i < n; i++)
        inputs[i].consumed = std::min(*output_size, inputs[i].len);

}
