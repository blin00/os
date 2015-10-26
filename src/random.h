#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>
#include <stddef.h>
#include "aes.h"
#include "sha256.h"

struct fortuna_generator {
    uint8_t key[32]; // 256-bit AES key
    union {
        struct {
            uint64_t l; // 128-bit counter
            uint64_t h;
        } counter;
        uint8_t counter_bytes[16];
    };
};
typedef struct fortuna_generator fortuna_generator_t;

struct fortuna_pool {
    SHA256_CTX ctx;
    uint32_t len;
};
typedef struct fortuna_pool fortuna_pool_t;

struct fortuna_prng {
    fortuna_generator_t gen;
    uint32_t reseeds;
    fortuna_pool_t pools[32];
};
typedef struct fortuna_prng fortuna_prng_t;

void rand_init(void);
int rand_data(uint8_t* out, size_t bytes);
void rand_on_rtc(uint32_t timer_ticks, uint32_t rtc_ticks);
void rand_on_kbd(uint32_t timer_ticks);
void rand_add_random_event(uint8_t* data, uint8_t length, uint8_t source, uint8_t pool);

#endif
