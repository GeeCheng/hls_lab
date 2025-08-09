#include "FIR.h"

#ifndef FIR_IMPL
#define FIR_IMPL 5   // choose 1..6
#endif

// ================= fir(): selectable implementations =================
#if FIR_IMPL == 1
// 1) Original: single loop with if-else
acc_t fir(data_t x) {
#pragma HLS INLINE off
    static data_t shift_reg[N];
#pragma HLS RESET variable=shift_reg
    acc_t acc = 0;
    for (int i = N - 1; i >= 0; --i) {
#pragma HLS LOOP_TRIPCOUNT min=11 max=11
        if (i == 0) {
            acc += (acc_t)x * (acc_t)c[0];
            shift_reg[0] = x;
        } else {
            shift_reg[i] = shift_reg[i - 1];
            acc += (acc_t)shift_reg[i] * (acc_t)c[i];
        }
    }
    return acc;
}
#elif FIR_IMPL == 2
// 2) Remove if-else: loop does shift then MAC for i=N-1..1, tap0 handled after
acc_t fir(data_t x) {
#pragma HLS INLINE off
    static data_t shift_reg[N];
#pragma HLS RESET variable=shift_reg
    acc_t acc = 0;

    for (int i = N - 1; i >= 1; --i) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=10
        shift_reg[i] = shift_reg[i - 1];                 // shift
        acc += (acc_t)shift_reg[i] * (acc_t)c[i];         // MAC with shifted value
    }
    acc += (acc_t)x * (acc_t)c[0];                        // tap0
    shift_reg[0] = x;
    return acc;
}
#elif FIR_IMPL == 3
// 3) Separate loops: TDL then MAC
acc_t fir(data_t x) {
#pragma HLS INLINE off
    static data_t shift_reg[N];
#pragma HLS RESET variable=shift_reg

    for (int i = N - 1; i > 0; --i) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=10
        shift_reg[i] = shift_reg[i - 1];
    }
    shift_reg[0] = x;

    acc_t acc = 0;
    for (int i = 0; i < N; ++i) {
#pragma HLS LOOP_TRIPCOUNT min=11 max=11
        acc += (acc_t)shift_reg[i] * (acc_t)c[i];
    }
    return acc;
}
#elif FIR_IMPL == 4
// 4) TDL unroll, MAC pipeline
acc_t fir(data_t x) {
#pragma HLS INLINE off
    static data_t shift_reg[N];
#pragma HLS RESET variable=shift_reg

    for (int i = N - 1; i > 0; --i) {
#pragma HLS UNROLL
        shift_reg[i] = shift_reg[i - 1];
    }
    shift_reg[0] = x;

    acc_t acc = 0;
    for (int i = 0; i < N; ++i) {
#pragma HLS PIPELINE II=1
        acc += (acc_t)shift_reg[i] * (acc_t)c[i];
    }
    return acc;
}
#elif FIR_IMPL == 5
// 5) TDL unroll + ARRAY_PARTITION complete + MAC pipeline
acc_t fir(data_t x) {
#pragma HLS INLINE off
    static data_t shift_reg[N];
#pragma HLS RESET variable=shift_reg
#pragma HLS ARRAY_PARTITION variable=shift_reg complete dim=1

    for (int i = N - 1; i > 0; --i) {
#pragma HLS UNROLL
        shift_reg[i] = shift_reg[i - 1];
    }
    shift_reg[0] = x;

    acc_t acc = 0;
    for (int i = 0; i < N; ++i) {
#pragma HLS PIPELINE II=1
        acc += (acc_t)shift_reg[i] * (acc_t)c[i];
    }
    return acc;
}
#elif FIR_IMPL == 6
// 6) TDL unroll + MAC unroll + ARRAY_PARTITION complete
acc_t fir(data_t x) {
#pragma HLS INLINE off
    static data_t shift_reg[N];
#pragma HLS RESET variable=shift_reg
#pragma HLS ARRAY_PARTITION variable=shift_reg complete dim=1

    for (int i = N - 1; i > 0; --i) {
#pragma HLS UNROLL
        shift_reg[i] = shift_reg[i - 1];
    }
    shift_reg[0] = x;

    acc_t acc = 0;
    for (int i = 0; i < N; ++i) {
#pragma HLS UNROLL
        acc += (acc_t)shift_reg[i] * (acc_t)c[i];
    }
    return acc;
}
#else
#error "FIR_IMPL must be 1..6"
#endif

// ================= top with AXI-Stream =================
void fir_n11_strm(hls::stream<data_t> &in, hls::stream<data_t> &out) {
#pragma HLS INTERFACE axis register both port=in
#pragma HLS INTERFACE axis register both port=out
#pragma HLS INTERFACE ap_ctrl_none port=return

    if (!in.empty()) {
        data_t x = in.read();
        acc_t y  = fir(x);
        out.write((data_t)y);
    }
}
