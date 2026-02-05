#include "Sequencer.h"

#include <random>

Sequencer::Sequencer()
{
    clear();
}

void Sequencer::clear()
{
    for (auto& pad : pattern)
        pad.fill(false);
}

void Sequencer::generate(float density,
                         float fills,
                         unsigned int seed,
                         const std::array<bool, kPads>& activePads)
{
    clear();

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    auto chance = [&](float p)
    {
        if (p <= 0.0f)
            return false;
        if (p >= 1.0f)
            return true;
        return dist(rng) < p;
    };

    auto padIsActive = [&](int pad)
    {
        return pad >= 0 && pad < kPads && activePads[pad];
    };

    const float hatProb = 0.25f + 0.65f * density;
    const float percProb = 0.05f + 0.20f * density;
    const float openHatProb = 0.10f + 0.35f * density;
    const float extraLayerProb = 0.08f + 0.5f * density;

    // Kick (pad 0) - four on the floor
    if (padIsActive(0))
    {
        for (int step : { 0, 8, 16, 24 })
            setStepActive(0, step, true);
    }

    // Clap/Snare (pad 1) - backbeats
    if (padIsActive(1))
    {
        for (int step : { 4, 12, 20, 28 })
            setStepActive(1, step, true);
    }

    // Closed hat (pad 2) - offbeat 8ths
    if (padIsActive(2))
    {
        for (int step = 2; step < kSteps; step += 4)
        {
            if (chance(hatProb))
                setStepActive(2, step, true);
        }
    }

    // Open hat (pad 3) - occasional lift
    if (padIsActive(3))
    {
        for (int step : { 6, 22 })
        {
            if (chance(openHatProb))
                setStepActive(3, step, true);
        }
    }

    // Percs (pad 4) - sparse 16ths
    if (padIsActive(4))
    {
        for (int step = 1; step < kSteps; ++step)
        {
            if (chance(percProb))
                setStepActive(4, step, true);
        }
    }

    // Additional active pads get rhythmic layers influenced by density
    int layerIndex = 0;
    for (int pad = 0; pad < kPads; ++pad)
    {
        if (!padIsActive(pad) || pad <= 4)
            continue;

        const bool emphasiseDownbeats = (layerIndex++ % 2 == 0);

        for (int step = 0; step < kSteps; ++step)
        {
            float probability = extraLayerProb;
            if (emphasiseDownbeats && step % 4 == 0)
                probability += 0.2f;
            else if (!emphasiseDownbeats && step % 4 == 2)
                probability += 0.15f;

            if (chance(probability))
                setStepActive(pad, step, true);
        }
    }

    auto applyFills = [&](int padIndex, float weight)
    {
        if (!padIsActive(padIndex))
            return;

        for (int step = 28; step < kSteps; ++step)
        {
            if (chance(fills * weight))
                setStepActive(padIndex, step, true);
        }
    };

    applyFills(2, 1.0f);
    applyFills(4, 0.6f);

    // Liven up any other active pads with fills toward the end of bar two
    for (int pad = 0; pad < kPads; ++pad)
    {
        if (!padIsActive(pad) || pad <= 4)
            continue;

        for (int step = 28; step < kSteps; ++step)
        {
            if (chance(fills * 0.4f))
                setStepActive(pad, step, true);
        }
    }
}

bool Sequencer::isStepActive(int pad, int step) const
{
    return pattern[pad][step];
}

void Sequencer::setStepActive(int pad, int step, bool active)
{
    pattern[pad][step] = active;
}
