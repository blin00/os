#include <cpuid.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "string.h"
#include "util.h"
#include "sha256.h"
#include "interrupt.h"
#include "random.h"

static fortuna_prng_t prng_state;
static bool has_rdrand = false;
static bool has_rdseed = false;

static inline void inc_counter(void) {
    prng_state.gen.counter.l++;
    if (prng_state.gen.counter.l == 0) prng_state.gen.counter.h++;
}

void rand_init(void) {
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        if (ecx & (1 << 30)) has_rdrand = true;
        if (ebx & (1 << 18)) has_rdseed = true;
    }
    memset(&prng_state, 0, sizeof(prng_state));
    for (size_t i = 0; i < 32; i++) {
        sha256_init(&prng_state.pools[i].ctx);
    }
}

static void rand_reseed(uint8_t* seed, size_t length) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, prng_state.gen.key, 32);
    sha256_update(&ctx, seed, length);
    sha256_final(&ctx, prng_state.gen.key);
    inc_counter();
}

static int rand_generate_blocks(uint8_t* out, size_t blocks) {
    if (prng_state.gen.counter.l == 0 && prng_state.gen.counter.h == 0) return 1;
    uint32_t key_setup[60];
    aes_key_setup(prng_state.gen.key, key_setup, 256);
    for (size_t i = 0; i < blocks; i++) {
        aes_encrypt(prng_state.gen.counter_bytes, out + i * 16, key_setup, 256);
        inc_counter();
    }
    return 0;
}

static int rand_gen_data(uint8_t* out, size_t bytes) {
    uint8_t buf[16];
    if (!bytes) return 0;
    if (bytes > (1 << 20)) return 1;
    size_t blocks = bytes >> 4;
    size_t extra = bytes & 0xf;
    if (rand_generate_blocks(out, blocks)) return 1;
    if (extra) {
        rand_generate_blocks(buf, 1);
        memcpy(out + (blocks << 4), buf, extra);
    }
    rand_generate_blocks(prng_state.gen.key, 2);
    return 0;
}

int rand_data(uint8_t* out, size_t bytes) {
    static uint32_t last_reseed = (uint32_t) -42;
    uint8_t buf[32 * 32];
    if (prng_state.pools[0].len >= 64 && rtc_ticks - last_reseed >= 7) {
        last_reseed = rtc_ticks;
        prng_state.reseeds++;
        size_t len = 0;
        for (size_t i = 0; i < 32; i++) {
            if (prng_state.reseeds | (1 << i)) {
                sha256_final(&prng_state.pools[i].ctx, &buf[len]);
                sha256_init(&prng_state.pools[i].ctx);
                prng_state.pools[i].len = 0;
                len += 32;
            }
        }
        rand_reseed(buf, len);
    }
    if (!prng_state.reseeds) return 1;
    return rand_gen_data(out, bytes);
}

void rand_add_random_event(uint8_t* data, uint8_t length, uint8_t source, uint8_t pool) {
    if (length < 1 || length > 32 || pool > 31) return;
    sha256_update(&prng_state.pools[pool].ctx, &source, 1);
    sha256_update(&prng_state.pools[pool].ctx, &length, 1);
    sha256_update(&prng_state.pools[pool].ctx, data, length);
    prng_state.pools[pool].len += length;
}

static void rand_rdseed(void) {
    static uint8_t pool = 0;
    uint32_t seed;
    if (has_rdseed) {
        if (!_rdseed(&seed)) {
            rand_add_random_event((uint8_t*) &seed, sizeof(seed), 2, pool);
            pool = (pool + 1) % 32;
        }
    } else if (has_rdrand) {
        if (!_rdrand(&seed)) {
            rand_add_random_event((uint8_t*) &seed, sizeof(seed), 2, pool);
            pool = (pool + 1) % 32;
        }
    }
}

static void rand_rdtsc(void) {
    static uint8_t pool = 0;
    uint16_t t = (uint16_t) _rdtsc();
    rand_add_random_event((uint8_t*) &t, sizeof(t), 1, pool);
    pool = (pool + 1) % 32;
}

void rand_on_rtc(uint32_t timer_ticks, uint32_t rtc_ticks) {
    //static uint8_t last = 0xff;
    static uint32_t num = 0;
    static uint8_t total = 0;
    static uint8_t pool = 0;
    uint8_t bits = timer_ticks & 0b11;
    num = (num << 2) | bits;
    total += 2;
    if (total >= sizeof(num) * 8) {
        rand_add_random_event((uint8_t*) &num, sizeof(num), 0, pool);
        pool = (pool + 1) % 32;
        num = 0;
        total = 0;
    }
    if (!(rtc_ticks & 0b11111)) {
        rand_rdseed();
        rand_rdtsc();
    }
}

void rand_on_kbd(uint32_t timer_ticks) {
    static uint32_t num = 0;
    static uint8_t total = 0;
    static uint8_t pool = 0;
    uint8_t bits = timer_ticks & 0b1111;
    num = (num << 4) | bits;
    total += 4;
    if (total >= sizeof(num) * 8) {
        rand_add_random_event((uint8_t*) &num, sizeof(num), 3, pool);
        pool = (pool + 1) % 32;
        num = 0;
        total = 0;
    }
    rand_rdtsc();
}