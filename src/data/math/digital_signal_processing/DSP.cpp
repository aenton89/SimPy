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

// funkja do debugowania
#include <iomanip>  // std::setprecision, std::fixed

void dsp::printStateSpace(const MatOp::StateSpace& ss) {
    std::cout << std::fixed << std::setprecision(10);  // 10 miejsc po przecinku

    std::cout << "A:\n";
    for (const auto& row : ss.A) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "B:\n";
    for (const auto& row : ss.B) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "C:\n";
    for (const auto& row : ss.C) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "D:\n";
    for (const auto& row : ss.D) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "x (stan początkowy): ";
    for (double v : ss.x) std::cout << v << " ";
    std::cout << "\n";
}


// transmitancja operaotorowa
MatOp::StateSpace dsp::tf2ss(std::vector<float> numerator, std::vector<float> denominator) {

    // if (denominator.empty()) {
    //     throw std::invalid_argument("Mianownik nie może być pusty.");
    // }

    if (numerator.size() > denominator.size()) {
        numerator = {1, 0};
        denominator = {1, 1};
    }
    // konwersja float -> double
    std::vector<double> num(numerator.begin(), numerator.end());
    std::vector<double> den(denominator.begin(), denominator.end());

    // usunięcie zer wiodących
    while (!num.empty() && std::fabs(num.front()) < 1e-12) num.erase(num.begin());
    while (!den.empty() && std::fabs(den.front()) < 1e-12) den.erase(den.begin());



    int deg_num = (int)num.size() - 1;
    int deg_den = (int)den.size() - 1;
    int n = deg_den;

    // normalizacja do wiodącego 1 w mianowniku
    double lead = den[0];
    for (auto &v : den) v /= lead;
    for (auto &v : num) v /= lead;

    // wyzerowanie wektora num do długości n+1
    std::vector<double> num_padded(n+1, 0.0);
    for (int i = 0; i < (int)num.size(); i++) {
        num_padded[n+1 - num.size() + i] = num[i];
    }

    std::vector<double> a(den.begin() + 1, den.end());

    double Dval = num_padded[0];

    std::vector<double> cvec(n, 0.0);
    for (int i = 0; i < n; i++) {
        cvec[i] = num_padded[i+1] - a[i] * Dval;
    }

    // macierz A (n x n)
    std::vector<std::vector<double>> A(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n-1; i++) {
        A[i][i+1] = 1.0;
    }
    for (int j = 0; j < n; j++) {
        A[n-1][j] = -a[n-1-j];
    }

    // macierz B (n x 1)
    std::vector<std::vector<double>> B(n, std::vector<double>(1, 0.0));
    B[n-1][0] = 1.0;

    // macierz C (1 x n)
    std::vector<std::vector<double>> C(1, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        C[0][i] = cvec[n-1-i];
    }

    // macierz D (1 x 1)
    std::vector<std::vector<double>> D(1, std::vector<double>(1, Dval));

    // wektor stanu x (n x 1), inicjalizacja zerami
    std::vector<double> x(n, 0.0);

    MatOp::StateSpace ss{A, B, C, D, x};

    //printStateSpace(ss);
    return ss;
}

// charakterystyka Bodego (trzeba podac zera i bieguny)
dsp::Bode dsp::bode_characteristic(const dsp::tf Tf) {

    float wmin = std::numeric_limits<float>::max();
    float wmax = 0.0;
    dsp::Bode bode;

    for (const auto& p: Tf.poles) {
        float mag = abs(p);
        if (mag < wmin) wmin = mag;
        if (mag > wmax) wmax = mag;
    }

    for (const auto& z: Tf.zeros) {
        float mag = abs(z);
        if (mag < wmin) wmin = mag;
        if (mag > wmax) wmax = mag;
    }

    // margines dla dekady
    wmin *= 0.1;
    wmax *= 10.0;

    int N = (std::log10(wmax/wmin) * 20) + 1;

    for (int i=0; i<N; i++) {
        double w = std::pow(wmin*(wmax/wmin), i/(N-1)); // pkt na charaterystuce
        cd s(0, w);

        cd num = cd(1.0, 0);
        for (auto z: Tf.zeros) num *= (s - z);

        cd den = cd(1.0, 0);
        for (auto p: Tf.poles) den *= (s - p);

        cd H = Tf.gain*num/den;

        double magnitude_dB = 20*std::log10(abs(H));
        double phase_deg = std::arg(H)*180.0/M_PI;

        bode.magnitude.push_back(magnitude_dB);
        bode.phase.push_back(phase_deg);
        bode.omega.push_back(w);
    }

    return bode;
}


// filtry

dsp::tf dsp::butterworth(int order, int filter_type, const std::vector<double>& cutoff) {
    std::vector<std::complex<double>> zeros;
    std::vector<std::complex<double>> poles;
    cd gain = cd(1.0, 0);

    tf Tf;

    // 1️⃣ Bieguny prototypowego LPF (ωc = 1)
    std::vector<std::complex<double>> poles_proto;
    for (int i = 0; i < order; i++) {
        double theta = M_PI * (2.0*i + 1.0 + order) / (2.0*order);
        std::complex<double> pole = std::exp(std::complex<double>(0.0, theta));
        if (pole.real() < 0)
            poles_proto.push_back(pole);
    }

    if (filter_type == LPF) {
        double wc = cutoff[0];
        for (auto& p : poles_proto) {
            poles.push_back(p * wc);
            gain *= wc;
        }

    } else if (filter_type == HPF) {
        double wc = cutoff[0];
        for (auto& p : poles_proto) {
            poles.push_back(wc / p);
            gain /= -p;
        }
        for (int i = 0; i < order; i++)
            zeros.push_back(std::complex<double>(0, 0));

    } else if (filter_type == BPF) {
        double w1 = cutoff[0], w2 = cutoff[1];
        double B = w2 - w1;
        double w0 = std::sqrt(w1 * w2);

        for (auto& p : poles_proto) {
            std::complex<double> A = p * B / 2.0;
            std::complex<double> delta = std::sqrt(A*A - w0*w0);
            poles.push_back(A + delta);
            poles.push_back(A - delta);
        }

        for (int i = 0; i < order; i++) {
            zeros.push_back(std::complex<double>(0, 0));
            zeros.push_back(std::complex<double>(0, 0));
            gain = cd(1.0, 0.0);
        }

    } else if (filter_type == BSF) {
        double w1 = cutoff[0], w2 = cutoff[1];
        double B = w2 - w1;
        double w0 = std::sqrt(w1 * w2);

        for (auto& p : poles_proto) {
            std::complex<double> A = p * B / 2.0;
            std::complex<double> delta = std::sqrt(A*A - w0*w0);
            poles.push_back(A + delta);
            poles.push_back(A - delta);
            gain *= p;  // Transformacja LP->BSF
        }

        for (int i = 0; i < order; i++) {
            zeros.push_back(std::complex<double>(0, w0));
            zeros.push_back(std::complex<double>(0, -w0));
        }
    }

    Tf.zeros = zeros;
    Tf.poles = poles;
    Tf.gain = gain.real();

    return Tf;
}



