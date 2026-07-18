/**
 * kk_ps2_device_test.c: Unit tests for PS/2 device I/O. AI-generated.
 *
 * The `main()` is generated automatically by running every function in this
 * file with a name starting `test_`. The state is cleared before each test,
 * DO NOT RESET STATE MANUALLY IN TESTS.
 *
 * Tests bypass the real AVR header and provide fully mocked pin functions.
 * Every CLK/DATA edge is logged with microsecond timing for symmetry and
 * setup/hold verification.
 *
 * The mock acts as a PS/2 host on the same bus: it changes DATA on the
 * falling clock edge (per-protocol), and the device reads on the rising
 * edge. Timing sentinels catch reads with insufficient setup time.
 *
 * DO NOT bypass the mocked pin functions — the whole point is testing
 * the real device code's bit-level behavior through the real call chain.
 *
 * DO NOT `grep` OR `tail` THE OUTPUT! The non-verbose output is all important!
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long now_us = 0;
static bool pin_clk = true, pin_data = true;
static bool drv_clk = false, drv_data = false;
static unsigned long last_data_change = 0, last_clk_rise = 0;

#define T_DATA_SETUP_MIN 5  // minimum µs data stable before clock rising edge
#define T_CLK_LOW_MIN    29 // minimum µs clock low
#define T_CLK_LOW_MAX    50 // maximum µs clock low
#define T_CLK_HIGH_MIN   29 // minimum µs clock high

// Bit data for simulated host — set on each falling edge by ps2_clk_set_low
static uint8_t mock_bit_data[11];
static int mock_bit_idx = 0;

typedef struct {
    unsigned long t;
    const char *label;
    bool clk, data;
} edge_t;
#define MAX_EDGES 1024
static edge_t edges[MAX_EDGES];
static int edge_n = 0;

static void
log_edge (const char *label) {
    if (edge_n < MAX_EDGES) {
        edges[edge_n].t = now_us;
        edges[edge_n].label = label;
        edges[edge_n].clk = pin_clk;
        edges[edge_n].data = pin_data;
        edge_n++;
    }
}

#define KK_PS2_AVR_H // Prevent inclusion of the real header

#define _BV(x) (1U << (x))

static uint8_t mock_ddr = 0, mock_port = 0;
#define PS2_PORT     D
#define PS2_CLK_PIN  0
#define PS2_DATA_PIN 1
#define DDRD         mock_ddr
#define PORTD        mock_port
#define PIND \
    (((mock_ddr & 1) ? (mock_port & 1) : (pin_clk ? 1 : 0)) \
        | ((mock_ddr & 2) ? (mock_port & 2) : (pin_data ? 2 : 0)))
#define PASTE_(a, b)     a##b
#define PASTE(a, b)      PASTE_(a, b)
#define PS2_DDR          DDRD
#define PS2_PORT_REG     PORTD
#define PS2_PIN_REG      PIND
#define PS2_CLK_BIT      0x01
#define PS2_DATA_BIT     0x02
#define PS2_BIT_MASK     0x03
#define INTERNAL_PULL_UP 1

#define ps2_set_pin_state(p, s) \
    do { \
        if (s) \
            PS2_PORT_REG |= _BV(p); \
        else \
            PS2_PORT_REG &= ~_BV(p); \
    } while (0)
#define ps2_clk_set(s)  ps2_set_pin_state(PS2_CLK_PIN, (s))
#define ps2_data_set(s) ps2_set_pin_state(PS2_DATA_PIN, (s))

#define ps2_clk_set_input() \
    do { \
        PS2_DDR &= ~PS2_CLK_BIT; \
        drv_clk = false; \
    } while (0)
#define ps2_clk_set_output() \
    do { \
        PS2_DDR |= PS2_CLK_BIT; \
        drv_clk = true; \
    } while (0)
#define ps2_data_set_input() \
    do { \
        PS2_DDR &= ~PS2_DATA_BIT; \
        drv_data = false; \
    } while (0)
#define ps2_data_set_output() \
    do { \
        PS2_DDR |= PS2_DATA_BIT; \
        drv_data = true; \
    } while (0)

static unsigned long host_inhibit_after = 0;
static uint8_t inhibit_after_pulse = 0;
static uint8_t send_clock_count = 0;

static inline void
ps2_clk_set_low (void) {
    ps2_clk_set(0);
    ps2_clk_set_output();
    pin_clk = false;
    log_edge("CLK↓");
    // Host changes data on falling edge — set next bit value
    if (mock_bit_idx < 11) {
        pin_data = mock_bit_data[mock_bit_idx] ? true : false;
        last_data_change = now_us;
        mock_bit_idx++;
    }
}
static inline void
ps2_clk_release (void) {
    ps2_clk_set_input();
    ps2_clk_set(INTERNAL_PULL_UP);
    pin_clk = true;
    last_clk_rise = now_us;
    log_edge("CLK↑");
    ++send_clock_count;
    if (inhibit_after_pulse && send_clock_count >= inhibit_after_pulse) {
        pin_clk = false;
        inhibit_after_pulse = 0;
    }
}
static inline void
ps2_data_set_low (void) {
    ps2_data_set(0);
    ps2_data_set_output();
    pin_data = false;
    last_data_change = now_us;
    log_edge("DATA↓");
}
static inline void
ps2_data_release (void) {
    ps2_data_set_input();
    ps2_data_set(INTERNAL_PULL_UP);
    pin_data = true;
    last_data_change = now_us;
    log_edge("DATA↑");
}
static inline void
ps2_data_set_value (bool high) {
    if (high) {
        ps2_data_release();
    } else {
        ps2_data_set_low();
    }
}
#define are_ps2_lines_high() ((PIND & PS2_BIT_MASK) == PS2_BIT_MASK)
#define is_ps2_clk_high()    ((uint8_t) (PIND & PS2_CLK_BIT) ? 1U : 0U)
#define is_ps2_clk_low()     ((uint8_t) (PIND & PS2_CLK_BIT) ? 0U : 1U)

// ps2_data_read — returns what the host set on the last falling clock edge,
// with timing validation. If data was changed less than T_DATA_SETUP_MIN µs
// before the read, returns a sentinel (2) to flag a timing violation.
static uint8_t
ps2_data_read_func (void) {
    if (now_us - last_data_change < T_DATA_SETUP_MIN) {
        return 2; // sentinel: data not stable yet
    }
    return pin_data ? 1U : 0U;
}
#define ps2_data_read ps2_data_read_func

static void
setup_recv_bits (uint8_t byte) {
    uint8_t p = 1;
    for (int b = 0; b < 8; b++) {
        if (byte & (1 << b)) {
            p ^= 1;
        }
    }
    for (int b = 0; b < 8; b++) {
        mock_bit_data[b] = (byte >> b) & 1; // LSB first
    }
    mock_bit_data[8] = p & 1; // odd parity
    mock_bit_data[9] = 1;     // stop
    mock_bit_idx = 0;
}

#define disable_interrupts() \
    do { \
    } while (0)
#define enable_interrupts() \
    do { \
    } while (0)

#define _delay_us(u) \
    do { \
        now_us += (u); \
        if (host_inhibit_after && now_us >= host_inhibit_after) { \
            pin_clk = false; \
            host_inhibit_after = 0; \
        } \
    } while (0)
#define ps2_delay_us(u) _delay_us(u)

// Do NOT define PS2_STATUS_PIN or PS2_ENABLE_PIN — the device code
// uses #ifdef to conditionally compile status/enable pin code.
// Keeping them undefined skips those code paths.

#define PS2_BUS_IDLE_TIME_US      ((uint8_t) 50U)
#define PS2_BUS_READY_TIMEOUT_US  ((uint8_t) 200U)
#define DATA_SETUP_US             15
#define CLOCK_PULSE_US            39
#define PROCESSING_AFTER_PULSE_US 2

#include "kk_ps2_device.c"

static int verbose = 0, tests_run = 0, tests_failed = 0;

static void
reset (void) {
    now_us = 0;
    pin_clk = true;
    pin_data = true;
    drv_clk = false;
    drv_data = false;
    edge_n = 0;
    mock_ddr = 0;
    mock_port = 0;
    ps2_device_error = 0;
    ps2_buffer_head = 0;
    ps2_buffer_tail = 0;
    mock_bit_idx = 99;
    host_inhibit_after = 0;
    inhibit_after_pulse = 0;
    send_clock_count = 0;
}

static void
check (bool c, const char *m) {
    tests_run++;
    if (!c) {
        tests_failed++;
        printf("FAIL: %s\n", m);
    } else if (verbose) {
        printf("PASS: %s\n", m);
    }
}

static void
test_send_byte (void) {
    reset();
    check(ps2_device_send(0xFA), "queue");
    unsigned long t0 = now_us;
    check(ps2_device_flush(), "flush");
    check(ps2_buffer_head == ps2_buffer_tail, "drained");
    check(now_us - t0 > 600, "send takes time");
    int clk = 0;
    for (int i = 0; i < edge_n; i++) {
        if (strstr(edges[i].label, "CLK")) {
            clk++;
        }
    }
    check(clk == 22, "11 pulses (22 edges)");
    bool start = false;
    for (int i = 0; i < edge_n; i++) {
        if (strstr(edges[i].label, "DATA↓") && i > 0) {
            start = true;
        }
    }
    check(start, "start bit");
}

static void
test_send_with_host_inhibit_after_stop (void) {
    reset();
    inhibit_after_pulse = 11; // inhibit after stop bit's clock rise
    check(ps2_device_send(0xFA), "queue");
    check(ps2_device_flush(), "flush succeeds despite host inhibit");
    check(ps2_buffer_head == ps2_buffer_tail, "byte sent despite inhibit");
}

static void
test_send_overflow (void) {
    reset();
    for (int i = 0; i < KK_PS2_BUFFER_SIZE; i++) {
        check(ps2_device_send(0x55), "fill");
    }
    check(!ps2_device_send(0x55), "overflow");
}

static void
host_start (void) {
    pin_clk = false;
    now_us += 160;
    pin_data = false;
    pin_clk = true;
}

static int
do_recv (uint8_t byte) {
    reset();
    setup_recv_bits(byte);
    host_start();
    return ps2_device_recv();
}

static void
test_recv_byte (void) {
    int r = do_recv(0xFA);
    check(r == 0xFA, "recv 0xFA");
}
static void
test_recv_00 (void) {
    check(do_recv(0x00) == 0x00, "recv 0x00");
}
static void
test_recv_FF (void) {
    check(do_recv(0xFF) == 0xFF, "recv 0xFF");
}
static void
test_recv_55 (void) {
    check(do_recv(0x55) == 0x55, "recv 0x55");
}
static void
test_recv_many (void) {
    uint8_t bytes[] = { 0x00, 0xFF, 0x55, 0xAA, 0x01, 0x80, 0x7F, 0xFA, 0xFE, 0x12, 0x34, 0x56, 0x78,
        0x9A };
    for (size_t i = 0; i < sizeof bytes; i++) {
        int r = do_recv(bytes[i]);
        if (r != (int) bytes[i]) {
            printf("FAIL: many[%zu]: expect 0x%02X got 0x%02X\n", i, bytes[i], r);
            tests_failed++;
            tests_run++;
            return;
        }
        tests_run++;
    }
}

static void
test_recv_start_err (void) {
    reset();
    pin_clk = false;
    now_us += 160;
    pin_data = true;
    pin_clk = true;
    check(ps2_device_recv() == EOF, "start error");
}

static void
test_recv_inhibit (void) {
    reset();
    pin_clk = false;
    check(ps2_device_recv() == EOF, "inhibit timeout");
}

static void
test_timing_send (void) {
    reset();
    ps2_device_send(0x55);
    ps2_device_flush();
    int bad_lo = 0, bad_asym = 0, bad_setup = 0, pulses = 0;
    unsigned long ft = 0, rt = 0, last_lo = 0, last_data_t = 0;
    int n_bad_asym = 0, pulse_bad = 0;
    unsigned long bad_lo_v = 0, bad_hi_v = 0;
    for (int i = 0; i < edge_n; i++) {
        if (strstr(edges[i].label, "CLK↓")) {
            ft = edges[i].t;
            if (last_data_t && ft - last_data_t < T_DATA_SETUP_MIN) {
                bad_setup++;
            }
            pulses++;
            if (rt && last_lo) {
                unsigned long hi = ft - rt;
                unsigned long diff = last_lo > hi ? last_lo - hi : hi - last_lo;
                if (diff > PROCESSING_AFTER_PULSE_US) {
                    bad_asym++;
                    if (n_bad_asym == 0) {
                        pulse_bad = pulses;
                        bad_lo_v = last_lo;
                        bad_hi_v = hi;
                    }
                    n_bad_asym++;
                }
            }
        }
        if (strstr(edges[i].label, "CLK↑") && ft) {
            rt = edges[i].t;
            last_lo = rt - ft;
            if (last_lo < T_CLK_LOW_MIN || last_lo > T_CLK_LOW_MAX) {
                bad_lo++;
            }
        }
        if (strstr(edges[i].label, "DATA")) {
            last_data_t = edges[i].t;
        }
    }
    check(bad_lo == 0, "send clock low in range");
    if (bad_asym) {
        printf("  asym pulse %d: low=%lu hi=%lu (limit %uu)\n", pulse_bad, bad_lo_v, bad_hi_v,
            PROCESSING_AFTER_PULSE_US);
    }
    check(bad_asym == 0, "send clock low≈high");
    check(bad_setup == 0, "send data setup >= min");
    check(pulses == 11, "send 11 pulses");
}

static void
test_timing_recv (void) {
    do_recv(0x55);
    int bad_lo = 0, bad_asym = 0, bad_setup = 0, pulses = 0;
    unsigned long ft = 0, rt = 0, last_lo = 0, last_data_t = 0;
    int n_bad_asym = 0, pulse_bad = 0;
    unsigned long bad_lo_v = 0, bad_hi_v = 0;
    for (int i = 0; i < edge_n; i++) {
        if (strstr(edges[i].label, "CLK↓")) {
            ft = edges[i].t;
            if (last_data_t && ft - last_data_t < T_DATA_SETUP_MIN) {
                bad_setup++;
            }
            pulses++;
            if (rt && last_lo) {
                unsigned long hi = ft - rt;
                unsigned long diff = last_lo > hi ? last_lo - hi : hi - last_lo;
                if (diff > PROCESSING_AFTER_PULSE_US) {
                    bad_asym++;
                    if (n_bad_asym == 0) {
                        pulse_bad = pulses;
                        bad_lo_v = last_lo;
                        bad_hi_v = hi;
                    }
                    n_bad_asym++;
                }
            }
        }
        if (strstr(edges[i].label, "CLK↑") && ft) {
            rt = edges[i].t;
            last_lo = rt - ft;
            if (last_lo < T_CLK_LOW_MIN || last_lo > T_CLK_LOW_MAX) {
                bad_lo++;
            }
        }
        if (strstr(edges[i].label, "DATA")) {
            last_data_t = edges[i].t;
        }
    }
    check(bad_lo == 0, "recv clock low in range");
    if (bad_asym) {
        printf("  asym pulse %d: low=%lu hi=%lu (limit %uu)\n", pulse_bad, bad_lo_v, bad_hi_v,
            PROCESSING_AFTER_PULSE_US);
    }
    check(bad_asym == 0, "recv clock low≈high");
    check(bad_setup == 0, "recv data setup >= min");
    check(pulses == 11, "recv 11 pulses");
}

static void
test_verbose_send (void) {
    reset();
    ps2_device_send(0x5A);
    ps2_device_flush();
    if (verbose) {
        printf("SEND 0x5A — %d edges:\n", edge_n);
        unsigned long ft = 0, rt = 0;
        for (int i = 0; i < edge_n; i++) {
            printf("  T=%5lu  %-12s", edges[i].t, edges[i].label);
            if (strstr(edges[i].label, "CLK↓")) {
                ft = edges[i].t;
                if (rt) {
                    printf("  hi=%lu", edges[i].t - rt);
                }
            }
            if (strstr(edges[i].label, "CLK↑") && ft) {
                rt = edges[i].t;
                printf("  lo=%lu", rt - ft);
            }
            if (strstr(edges[i].label, "DATA")) {
                printf("  DATA=%d", edges[i].data ? 1 : 0);
            }
            printf("\n");
        }
    }
}

static void
test_verbose_recv (void) {
    int val = do_recv(0xA5);
    if (verbose) {
        printf("RECV 0xA5 → 0x%02X — %d edges:\n", (unsigned) val, edge_n);
        unsigned long ft = 0, rt = 0;
        for (int i = 0; i < edge_n; i++) {
            printf("  T=%5lu  %-12s", edges[i].t, edges[i].label);
            if (strstr(edges[i].label, "CLK↓")) {
                ft = edges[i].t;
                if (rt) {
                    printf("  hi=%lu", edges[i].t - rt);
                }
            }
            if (strstr(edges[i].label, "CLK↑") && ft) {
                rt = edges[i].t;
                printf("  lo=%lu", rt - ft);
            }
            printf("\n");
        }
        printf("  received 0x%02X\n", (unsigned) val);
    }
}

#include "device_test_runner.c"
