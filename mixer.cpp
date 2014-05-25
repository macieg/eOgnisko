#include <cstdio>
#include <climits>
#include <algorithm>
#include "mixer.h"

#define MAGIC_NUMBER 176

void mixer(struct mixer_input* inputs, size_t n, void* output_buf,
        size_t* output_size, unsigned long tx_interval_ms
        ) {
    fprintf(stderr, "MIXER\n");
    unsigned long output_buffer_size = tx_interval_ms * MAGIC_NUMBER / 2;
    
    for (size_t i = 0; i < output_buffer_size; i++) {
        int sum = 0;
        for (size_t j = 0; j < n; j++) {
            if (inputs[i].len >= j)
            {
                if (sum >= 0)
                    sum = std::min(sum + ((short int*) inputs[i].data)[j], (int) SHRT_MAX);
                else
                    sum = std::max(sum + ((short int*) inputs[i].data)[j], (int) SHRT_MIN);
            }
        }
    }
}


