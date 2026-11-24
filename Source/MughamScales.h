#pragma once
#include <vector>
#include <string>
#include <cmath>

struct ScaleDefinition {
    std::string name;
    std::vector<double> centsFromRoot; // 100 sent = 1 yarımton
};

class MughamScales {
public:
    static std::vector<ScaleDefinition> getAllScales() {
        return {
            // Qərb Ladları (Natural, Harmonik, Melodik)
            {"Major (Natural)",      {0, 200, 400, 500, 700, 900, 1100}},
            {"Major (Harmonik)",     {0, 200, 400, 500, 700, 800, 1100}},
            {"Major (Melodik)",      {0, 200, 400, 500, 700, 800, 1000}},
            {"Minor (Natural)",      {0, 200, 300, 500, 700, 800, 1000}},
            {"Minor (Harmonik)",     {0, 200, 300, 500, 700, 800, 1100}},
            {"Minor (Melodik)",      {0, 200, 300, 500, 700, 900, 1100}},

            // Azərbaycan Muğamları (Mikrotonal / Komma intervallarla)
            {"Muğam: Rast",          {0, 200, 365, 500, 700, 900, 1065}},
            {"Muğam: Şur",           {0, 155, 300, 500, 700, 855, 1000}},
            {"Muğam: Segah",         {0, 145, 345, 500, 645, 845, 1000}},
            {"Muğam: Çahargah",      {0, 135, 400, 500, 700, 835, 1100}},
            {"Muğam: Bayatı-Şiraz",  {0, 200, 300, 500, 700, 800, 1065}},
            {"Muğam: Şüştər",        {0, 145, 300, 445, 700, 800, 1000}},
            {"Muğam: Humayun",       {0, 145, 400, 500, 645, 800, 1000}}
        };
    }

    static double snapFrequencyToScale(double inputFreq, int rootMidi, const ScaleDefinition& scale) {
        if (inputFreq <= 20.0) return inputFreq;

        double exactMidi = 69.0 + 12.0 * std::log2(inputFreq / 440.0);
        double centsFromRoot = (exactMidi - rootMidi) * 100.0;

        double octaveShift = std::floor(centsFromRoot / 1200.0) * 1200.0;
        double noteInOctave = centsFromRoot - octaveShift;

        double closestCent = scale.centsFromRoot[0];
        double minDiff = 9999.0;

        for (double targetCent : scale.centsFromRoot) {
            double diff = std::abs(noteInOctave - targetCent);
            if (diff < minDiff) {
                minDiff = diff;
                closestCent = targetCent;
            }
        }
        if (std::abs(noteInOctave - 1200.0) < minDiff) {
            closestCent = 1200.0;
        }

        double targetMidi = rootMidi + (octaveShift + closestCent) / 100.0;
        return 440.0 * std::pow(2.0, (targetMidi - 69.0) / 12.0);
    }
};
