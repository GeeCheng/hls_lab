#ifndef FIR_H
#define FIR_H

#include <ap_int.h>
#include <hls_stream.h>

typedef ap_int<16>  data_t;   // input/output
typedef ap_int<16>  coef_t;   // coefficient
typedef ap_int<32>  acc_t;    // accumulator

const int N = 11;

// Example 11-tap symmetric coefficients
static const coef_t c[N] = {53, 0, -91, 0, 313, 500, 313, 0, -91, 0, 52};

// Pure function: one sample in, one sample out (keeps internal state)
acc_t fir(data_t x);

// Top-level: AXI-Stream interface
void fir_n11_strm(hls::stream<data_t> &in, hls::stream<data_t> &out);

#endif
