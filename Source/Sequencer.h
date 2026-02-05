#pragma once

#include <array>

class Sequencer
{
public:
    static constexpr int kPads = 16;
    static constexpr int kSteps = 32; // 2 bars of 16th notes

    Sequencer();

    void clear();
    void generate(float density,
                  float fills,
                  unsigned int seed,
                  const std::array<bool, kPads>& activePads);

    bool isStepActive(int pad, int step) const;
    void setStepActive(int pad, int step, bool active);
    const std::array<std::array<bool, kSteps>, kPads>& getPattern() const { return pattern; }

private:
    std::array<std::array<bool, kSteps>, kPads> pattern{};
};
