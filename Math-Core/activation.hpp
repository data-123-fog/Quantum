#ifndef ACTIVATION_HPP
#define ACTIVATION_HPP

#include "complex_matrix.hpp"
#include <cmath>

namespace MathCore {

class WaveActivation {
public:
    static WaveVector sigmoid(const WaveVector& v) {
        double mag = v.magnitude();
        double activated = 1.0 / (1.0 + std::exp(-mag));
        if (mag < 1e-10) return WaveVector(Complex(activated, 0.0), v.phase);
        Complex newAmp = v.amplitude * (activated / mag);
        return WaveVector(newAmp, v.phase);
    }
    
    static WaveVector relu(const WaveVector& v) {
        if (v.magnitude() < 0) return WaveVector();
        return v;
    }
    
    static WaveVector tanhWave(const WaveVector& v) {
        double mag = v.magnitude();
        double activated = std::tanh(mag);
        if (mag < 1e-10) return WaveVector(Complex(activated, 0.0), v.phase);
        Complex newAmp = v.amplitude * (activated / mag);
        return WaveVector(newAmp, v.phase);
    }
};

} // namespace MathCore

#endif
