#include <cstdio>
#include <vector>
#include <iostream>
#include "FIR.h"

static void maybe_compare_with_golden(const char *out_file, const char *gold_file) {
    FILE *fg = std::fopen(gold_file, "r");
    FILE *fo = std::fopen(out_file, "r");
    if (!fg || !fo) {
        std::cout << "[TB] 找不到 " << gold_file << " 或 " << out_file << "，略過比對。\n";
        if (fg) std::fclose(fg);
        if (fo) std::fclose(fo);
        return;
    }
    long long g, o;
    int n = 0, mism = 0;
    while (std::fscanf(fg, "%lld", &g) == 1 && std::fscanf(fo, "%lld", &o) == 1) {
        if (g != o) ++mism;
        ++n;
    }
    std::fclose(fg);
    std::fclose(fo);
    if (mism == 0) std::cout << "[TB] 與 golden 完全一致（" << n << " 筆）。\n";
    else std::cout << "[TB] 與 golden 不一致：" << mism << "/" << n << " 筆不同。\n";
}

int main() {
    const int NUM = 256;
    std::vector<data_t> xs(NUM), ys(NUM);

    xs[0] = 1;
    for (int i = 1; i < NUM; ++i) xs[i] = (i < 32) ? 0 : (i - 31);

    hls::stream<data_t> in, out;

    for (int i = 0; i < NUM; ++i) {
#pragma HLS LOOP_TRIPCOUNT min=256 max=256
        in.write(xs[i]);
        fir_n11_strm(in, out);
        if (!out.empty()) ys[i] = out.read();
        else ys[i] = 0;
    }

    const char *out_file = "out.dat";
    FILE *f = std::fopen(out_file, "w");
    for (int i = 0; i < NUM; ++i) {
        std::fprintf(f, "%d\n", (int)ys[i]);
    }
    std::fclose(f);
    std::cout << "[TB] 產生 out.dat 完成。\n";
    maybe_compare_with_golden("out.dat", "out_gold.dat");
    return 0;
}
