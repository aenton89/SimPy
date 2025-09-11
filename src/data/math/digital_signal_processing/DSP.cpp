//
// Created by patryk on 11.09.25.
//

#include "math/digital_signal_processing/DSP.h"

using cd = std::complex<double>;


/////////////////////////////////////////////////////////////////// Ten fragment trzebna przekminic czy wywalic do math czy dac jako metody kalsy ///////////
// FFT dla długości potęgi 2
void dsp::fft(std::vector<cd> &a, bool invert) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * std::numbers::pi / len * (invert ? -1 : 1);
        cd wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = a[i+j], v = a[i+j+len/2]*w;
                a[i+j] = u+v;
                a[i+j+len/2] = u-v;
                w *= wlen;
            }
        }
    }
    if (invert) for (cd &x : a) x /= n;
}

// Konwolucja przez FFT
std::vector<cd> dsp::convolve(std::vector<cd> const& a, std::vector<cd> const& b) {
    std::vector<cd> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    int n = 1;
    while (n < static_cast<int>(a.size() + b.size())) n <<= 1;
    fa.resize(n); fb.resize(n);

    dsp::fft(fa, false);
    dsp::fft(fb, false);
    for (int i = 0; i < n; i++) fa[i] *= fb[i];
    dsp::fft(fa, true);
    return fa;
}

// Algorytm Bluesteina
std::vector<cd> dsp::bluestein(std::vector<cd> const& x) {
    int N = x.size();
    std::vector<cd> a(N), b(2*N-1);
    for (int n = 0; n < N; n++) {
        a[n] = x[n] * std::exp(cd(0, -std::numbers::pi*n*n/N));
    }
    for (int n = -(N-1); n <= N-1; n++) {
        b[n+N-1] = std::exp(cd(0, std::numbers::pi*n*n/N));
    }
    std::vector<cd> c = convolve(a, b);
    std::vector<cd> X(N);
    for (int k = 0; k < N; k++) {
        X[k] = c[k + N-1];
    }
    return X;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////