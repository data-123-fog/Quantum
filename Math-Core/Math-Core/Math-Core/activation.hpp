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
        Complex newAmp = v.amplitude * (activated / (mag + 1e-10));
        return WaveVector(newAmp, v.phase);
    }
    
    static WaveVector relu(const WaveVector& v) {
        if (v.magnitude() < 0) {
            return WaveVector();
        }
        return v;
    }
    
    static WaveVector tanh_wave(const WaveVector& v) {
        double mag = v.magnitude();
        double activated = std::tanh(mag);
        Complex newAmp = v.amplitude * (activated / (mag + 1e-10));
        return WaveVector(newAmp, v.phase);
    }
    
    static WaveVector phaseShift(const WaveVector& v, double shift) {
        return WaveVector(v.amplitude, v.phase + shift);
    }
};

} // namespace MathCore

#endif
