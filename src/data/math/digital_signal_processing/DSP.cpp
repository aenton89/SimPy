//
// Created by patryk on 11.09.25.
//

#include "math/digital_signal_processing/DSP.h"
#include <numbers>

using cd = std::complex<double>;


/////////////////////////////////////////////////////////////////// ten fragment trzeba przekminic czy wywalic do math czy dac jako metody klasy ///////////
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
dsp::Bode dsp::bode_characteristic(const tf& Tf) {

    double wmin = std::numeric_limits<double>::max();
    double wmax = 0.0;

    // Szukamy minimalnej i maksymalnej wartości modułu biegunów i zer
    for (const auto& p: Tf.poles) {
        double mag = std::abs(p);
        if (mag < wmin) wmin = mag;
        if (mag > wmax) wmax = mag;
    }

    for (const auto& z: Tf.zeros) {
        double mag = std::abs(z);
        if (mag < wmin) wmin = mag;
        if (mag > wmax) wmax = mag;
    }

    // zabezpieczenie w przypadku wmin = 0
    if (wmin == 0.0) wmin = 1e-3;

    wmin *= 0.1;
    wmax *= 10.0;

    int N = static_cast<int>(std::log10(wmax/wmin) * 100) + 1;

    dsp::Bode bode;


    for (int i = 0; i < N; i++) {
        // logarytmicznie rozłożone częstotliwości
        double w = wmin * std::pow(wmax/wmin, static_cast<double>(i)/(N-1));
        cd s(0, w);

        cd num(1.0, 0);
        for (auto z: Tf.zeros) num *= (s - z);

        cd den(1.0, 0);
        for (auto p: Tf.poles) den *= (s - p);

        cd H = Tf.gain * num / den;

        double magnitude_dB = 20.0 * std::log10(std::abs(H));
        double phase_deg = std::arg(H) * 180.0 / std::numbers::pi;

        //std::cout << magnitude_dB << " " << phase_deg << " " << w << std::endl;

        bode.magnitude.push_back(magnitude_dB);
        bode.phase.push_back(phase_deg);
        bode.omega.push_back(w);
    }


    return bode;
}



// filtry

// Zamysl jest taki. Na poaczku projektujemy prototyp filtra (LPF dla cutof = 1) Nastepnie modyfikujemy go jedna funkja na juz ionne typu o konretnych cutoff


dsp::FilterDesigner::FilterDesigner() {
    Tf = dsp::FilterDesigner::butterworth_proto();
    dsp::FilterDesigner::apply_filter_subtype();
}

dsp::tf dsp::FilterDesigner::butterworth_proto() {
    tf Tf;
    std::vector<cd> poles;

    for (int i = 0; i < order; i++) {
        double theta = std::numbers::pi * (2.0 * i + 1.0 + order) / (2.0 * order);
        cd pole = std::exp(cd(0.0, theta));
        if (pole.real() < 0.0) {
            poles.push_back(pole);
        }
    }

    Tf.zeros = {};      // brak zer w prototypie
    Tf.poles = poles;   // bieguny na lewej półpłaszczyźnie
    Tf.gain = 1.0;      // normalizacja

    return Tf;
}

dsp::tf dsp::FilterDesigner::chebyshev_i_proto() {
    tf Tf;
    std::vector<cd> poles;

    double epsilon = std::sqrt(std::pow(10, ripple/10.0) - 1.0);
    double beta = std::asinh(1.0 / epsilon);
    int N = order;

    for (int k = 1; k <= N; k++) {
        double theta = std::numbers::pi * (2.0 * k - 1.0) / (2.0 * N);
        cd pole(
            -std::sinh(beta / N) * std::sin(theta),
             std::cosh(beta / N) * std::cos(theta)
        );
        //if (pole.real() < 0.0)  // tylko lewa półpłaszczyzna
            poles.push_back(pole);
    }

    Tf.zeros = {};
    Tf.poles = poles;
    Tf.gain = 1.0 ;

    return Tf;
}

dsp::tf dsp::FilterDesigner::chebyshev_ii_proto() { // ni huja on nie dziala
    tf Tf;
    std::vector<cd> poles;
    std::vector<cd> zeros;

    int N = order;

    // epsilon dla Chebyshev II (z tłumieniem w paśmie zaporowym)
    double Rs = ripple; // tłumienie w paśmie zaporowym w dB
    double epsilon = 1.0 / std::sqrt(std::pow(10.0, Rs / 10.0) - 1.0);
    double beta = std::asinh(1.0 / epsilon) / N;

    for (int k = 1; k <= N; k++) {
        double theta = std::numbers::pi * (2.0 * k - 1.0) / (2.0 * N);

        // poprawne bieguny Chebyshev II
        double sigma = -std::sinh(beta) * std::sin(theta);
        double omega = std::cosh(beta) * std::cos(theta);
        cd pole(sigma, omega);

        // zera leżą na osi urojonej
        cd zero(0.0, 1.0 / std::cos(theta));

        poles.push_back(pole);
        zeros.push_back(zero);
    }

    Tf.zeros = zeros;
    Tf.poles = poles;

    // Wzmocnienie prototypu Chebyshev II
    Tf.gain = 1.0;

    return Tf;
}

dsp::tf dsp::FilterDesigner::besel_proto() {
    tf Tf;
    std::vector<cd> poles;
    std::vector<cd> zeros;

    int N = order;
    if (N == 0) {
        poles.push_back(cd(0, 0));
        zeros.push_back(cd(1.0, 0));
    }
    if (N == 1)
    {

    }

    Tf.zeros = zeros;
    Tf.poles = poles;

    // Wzmocnienie prototypu Chebyshev II
    Tf.gain = 1.0;

    return Tf;
}

// idk czy ja mam w cutoff robic normalziacje czy w pkt odniesienia
double eval_H(const dsp::tf& Tf, double omega) {
    cd s(0, omega);

    cd num(1.0, 0);
    for (auto z: Tf.zeros) num *= (s - z);

    cd den(1.0, 0);
    for (auto p: Tf.poles) den *= (s - p);

    cd H = Tf.gain * num / den;

    return std::abs(H);
}


void dsp::FilterDesigner::apply_filter_subtype() {
    std::vector<cd> new_poles;
    std::vector<cd> new_zeros;
    double gain = 1.0;

    if (filter_subtype == LPF) {
        double wc = cutoff[0];
        // Skala biegunów
        for (auto& p : Tf.poles)
            new_poles.push_back(p * wc);
        // Skala zer jeśli są w prototypie
        for (auto& z : Tf.zeros)
            new_zeros.push_back(z * wc);

        Tf.poles = new_poles;
        Tf.zeros = new_zeros;

        // Normalizacja w ω = 0
        double H0 = eval_H(Tf, 0.0);
        Tf.gain /= H0;

    } else if (filter_subtype == HPF) {
        double wc = cutoff[0];
        // Transformacja biegunów LPF → HPF
        for (auto& p : Tf.poles)
            new_poles.push_back(wc / p);

        // Transformacja zer: jeśli prototyp ma zera, przekształcamy je; jeśli nie, dodajemy w 0
        if (Tf.zeros.empty()) {
            for (size_t i = 0; i < Tf.poles.size(); i++)
                new_zeros.push_back(cd(0, 0));
        } else {
            for (auto& z : Tf.zeros)
                new_zeros.push_back(wc / z);
        }

        Tf.poles = new_poles;
        Tf.zeros = new_zeros;

        // Normalizacja w ω → ∞
        double Hinf = eval_H(Tf, 1e6);
        Tf.gain /= Hinf;

    } else if (filter_subtype == BPF) {
        double w1 = cutoff[0], w2 = cutoff[1];
        double B = w2 - w1;
        double w0 = std::sqrt(w1 * w2);

        // Transformacja biegunów LPF → BPF
        for (auto& p : Tf.poles) {
            cd A = p * B / 2.0;
            cd delta = std::sqrt(A*A - w0*w0);
            new_poles.push_back(A + delta);
            new_poles.push_back(A - delta);
        }

        // Transformacja zer: jeśli prototyp ma zera, przekształcamy je; jeśli nie, tworzymy w 0
        if (Tf.zeros.empty()) {
            for (size_t i = 0; i < Tf.poles.size() / 2; i++) {
                new_zeros.push_back(cd(0, 0));
                new_zeros.push_back(cd(0, 0));
            }
        } else {
            for (auto& z : Tf.zeros) {
                cd A = z * B / 2.0;
                cd delta = std::sqrt(A*A - w0*w0);
                new_zeros.push_back(A + delta);
                new_zeros.push_back(A - delta);
            }
        }

        Tf.poles = new_poles;
        Tf.zeros = new_zeros;

        // Normalizacja w ω = w0 (środek pasma)
        double Hmid = eval_H(Tf, w0);
        Tf.gain /= Hmid;

    } else if (filter_subtype == BSF) {
        double w1 = cutoff[0], w2 = cutoff[1];
        double B = w2 - w1;
        double w0 = std::sqrt(w1 * w2);

        // Transformacja biegunów LPF → BSF
        for (auto& p : Tf.poles) {
            cd A = p * B / 2.0;
            cd delta = std::sqrt(A*A - w0*w0);
            // Odwrócenie transformacji dla BSF
            new_poles.push_back(w0*w0 / (A + delta));
            new_poles.push_back(w0*w0 / (A - delta));
        }

        // Transformacja zer: jeśli prototyp ma zera, przekształcamy je; jeśli nie, dodajemy ±j*w0
        if (Tf.zeros.empty()) {
            for (size_t i = 0; i < Tf.poles.size() / 2; i++) {
                new_zeros.push_back(cd(0, w0));
                new_zeros.push_back(cd(0, -w0));
            }
        } else {
            for (auto& z : Tf.zeros) {
                cd A = z * B / 2.0;
                cd delta = std::sqrt(A*A - w0*w0);
                new_zeros.push_back(w0*w0 / (A + delta));
                new_zeros.push_back(w0*w0 / (A - delta));
            }
        }

        Tf.poles = new_poles;
        Tf.zeros = new_zeros;

        // Normalizacja w ω = 0
        double H0 = eval_H(Tf, 0.0);
        Tf.gain /= H0;
    }
}

void dsp::FilterDesigner::apply_setting(int order, int filter_type, int filter_subtype, float ripple, std::vector<double> cutoff) {
    this->order = order;
    this->filter_type = filter_type;
    this->filter_subtype = filter_subtype;
    this->ripple = ripple;
    this->cutoff = cutoff;

    if (filter_subtype == LPF || filter_subtype == HPF)
        this->cutoff[1] = this->cutoff[0];

    //std::cout << this->filter_type << std::endl;

    if (this->filter_type == BUTTERWORTH)
        Tf = FilterDesigner::butterworth_proto();
    else if (this->filter_type == CHEBYSHEV_I)
        Tf = FilterDesigner::chebyshev_i_proto();
    else if (this->filter_type == CHEBYSHEV_II)
        Tf = FilterDesigner::chebyshev_ii_proto();

    dsp::FilterDesigner::apply_filter_subtype();
}

dsp::tf dsp::FilterDesigner::get_tf() {
    return Tf;
}









// // filtr butherwortha
// dsp::tf dsp::butterworth_proto(const int& order) {
//     dsp::tf Tf;
//     std::vector<cd> poles;
//
//     for (int i=0; i<order; i++) {
//         double theta = std::numbers::pi * (2.0*i + 1.0 + order) / (2.0*order);
//         cd pole = std::exp(cd(0.0, theta));
//         if (pole.real() < 0.0) {
//             poles.push_back(pole);
//         }
//     }
//     Tf.zeros = {};         // brak zer w prototypie
//     Tf.poles = poles;      // bieguny na półpłaszczyźnie lewej
//     Tf.gain = 1.0;         // normalizacja
//
//     return Tf;
// }
//
// // filtr czebyszewicza
// dsp::tf dsp::chebyshev_1_proto(const int& order) {
//     dsp::tf Tf;
//     std::vector<cd> poles;
//
//     for (int i = 0; i < order; i++) {
//
//     }
// }
//
//
//
// dsp::tf dsp::apply_filter_subtype(const int& filter_type, dsp::tf tf_zp, const std::vector<double>& cutoff) {
//     cd gain = cd(1.0, 0);
//     std::vector<cd> new_poles;
//     std::vector<cd> new_zeros;
//
//     if (filter_type == LPF) {
//         double wc = cutoff[0];
//         for (auto& p : tf_zp.poles) {
//             new_poles.push_back(p * wc);
//             gain *= wc;
//         }
//         // brak zer
//
//     } else if (filter_type == HPF) {
//         double wc = cutoff[0];
//         for (auto& p : tf_zp.poles) {
//             new_poles.push_back(wc / p);
//             gain *= -1.0 / p;
//         }
//         for (int i=0; i<tf_zp.poles.size(); i++)
//             new_zeros.push_back(cd(0,0));
//
//     } else if (filter_type == BPF) {
//         double w1 = cutoff[0], w2 = cutoff[1];
//         double B = w2 - w1;
//         double w0 = std::sqrt(w1 * w2);
//
//         for (auto& p : tf_zp.poles) {
//             cd A = p * B / 2.0;
//             cd delta = std::sqrt(A*A - w0*w0);
//             new_poles.push_back(A + delta);
//             new_poles.push_back(A - delta);
//         }
//         for (int i=0; i<tf_zp.poles.size(); i++) {
//             new_zeros.push_back(cd(0,0));
//             new_zeros.push_back(cd(0,0));
//         }
//         gain = 1;
//
//     } else if (filter_type == BSF) {
//         double w1 = cutoff[0], w2 = cutoff[1];
//         double B = w2 - w1;
//         double w0 = std::sqrt(w1 * w2);
//
//         for (auto& p : tf_zp.poles) {
//             cd A = p * B / 2.0;
//             cd delta = std::sqrt(A*A - w0*w0);
//             new_poles.push_back(A + delta);
//             new_poles.push_back(A - delta);
//             gain *= p;
//         }
//         for (int i=0; i<tf_zp.poles.size(); i++) {
//             new_zeros.push_back(cd(0, w0));
//             new_zeros.push_back(cd(0,-w0));
//         }
//     }
//
//     tf_zp.poles = new_poles;
//     tf_zp.zeros = new_zeros;
//     tf_zp.gain = gain.real();
//     return tf_zp;
// }



// dsp::tf dsp::butterworth(int order, int filter_type, const std::vector<double>& cutoff) {
//     std::vector<std::complex<double>> zeros;
//     std::vector<std::complex<double>> poles;
//     cd gain = cd(1.0, 0);
//
//     tf Tf;
//
//     std::vector<std::complex<double>> poles_proto;
//     for (int i = 0; i < order; i++) {
//         double theta = std::numbers::pi * (2.0*i + 1.0 + order) / (2.0*order);
//         std::complex<double> pole = std::exp(std::complex<double>(0.0, theta));
//         if (pole.real() < 0)
//             poles_proto.push_back(pole);
//     }
//
//     if (filter_type == LPF) {
//         double wc = cutoff[0];
//         for (auto& p : poles_proto) {
//             poles.push_back(p * wc);
//             gain *= wc;
//         }
//
//     } else if (filter_type == HPF) {
//         double wc = cutoff[0];
//         for (auto& p : poles_proto) {
//             poles.push_back(wc / p);
//             gain /= -p;
//         }
//         for (int i = 0; i < order; i++)
//             zeros.push_back(std::complex<double>(0, 0));
//
//     } else if (filter_type == BPF) {
//         double w1 = cutoff[0], w2 = cutoff[1];
//         double B = w2 - w1;
//         double w0 = std::sqrt(w1 * w2);
//
//         for (auto& p : poles_proto) {
//             std::complex<double> A = p * B / 2.0;
//             std::complex<double> delta = std::sqrt(A*A - w0*w0);
//             poles.push_back(A + delta);
//             poles.push_back(A - delta);
//         }
//
//         for (int i = 0; i < order; i++) {
//             zeros.push_back(std::complex<double>(0, 0));
//             zeros.push_back(std::complex<double>(0, 0));
//             gain = cd(1.0, 0.0);
//         }
//
//     } else if (filter_type == BSF) {
//         double w1 = cutoff[0], w2 = cutoff[1];
//         double B = w2 - w1;
//         double w0 = std::sqrt(w1 * w2);
//
//         for (auto& p : poles_proto) {
//             std::complex<double> A = p * B / 2.0;
//             std::complex<double> delta = std::sqrt(A*A - w0*w0);
//             poles.push_back(A + delta);
//             poles.push_back(A - delta);
//             gain *= p;  // Transformacja LP->BSF
//         }
//
//         for (int i = 0; i < order; i++) {
//             zeros.push_back(std::complex<double>(0, w0));
//             zeros.push_back(std::complex<double>(0, -w0));
//         }
//     }
//
//     Tf.zeros = zeros;
//     Tf.poles = poles;
//     Tf.gain = gain.real();
//
//     return Tf;
// }



