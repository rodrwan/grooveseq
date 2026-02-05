#include "SequencerGrid.h"

SequencerGrid::SequencerGrid(DataProvider& provider)
    : data(provider)
{
    startTimerHz(30);
}

void SequencerGrid::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1b1b1f));

    const int pads = data.getPadCount();
    const int steps = data.getStepCount();
    if (pads <= 0 || steps <= 0)
        return;

    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    const float cellW = bounds.getWidth() / static_cast<float>(steps);
    const float cellH = bounds.getHeight() / static_cast<float>(pads);

    const int playStep = data.getCurrentStep();

    for (int row = 0; row < pads; ++row)
    {
        for (int col = 0; col < steps; ++col)
        {
            juce::Rectangle<float> cell(bounds.getX() + col * cellW,
                                        bounds.getY() + row * cellH,
                                        cellW,
                                        cellH);

            const bool active = data.getStepState(row, col);
            const bool isPlayhead = (col == playStep);

            juce::Colour fill = active ? juce::Colour(0xff4fd1c5) : juce::Colour(0xff26262c);
            if (isPlayhead)
                fill = active ? juce::Colour(0xfff7b500) : juce::Colour(0xff3a2f1a);

            g.setColour(fill);
            g.fillRect(cell.reduced(1.0f));
        }
    }

    g.setColour(juce::Colour(0xff3f3f46));
    g.drawRect(bounds, 1.0f);
}

void SequencerGrid::mouseDown(const juce::MouseEvent& event)
{
    const int pads = data.getPadCount();
    const int steps = data.getStepCount();
    if (pads <= 0 || steps <= 0)
        return;

    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    if (!bounds.contains(event.position))
        return;

    const float cellW = bounds.getWidth() / static_cast<float>(steps);
    const float cellH = bounds.getHeight() / static_cast<float>(pads);

    const int col = static_cast<int>((event.position.x - bounds.getX()) / cellW);
    const int row = static_cast<int>((event.position.y - bounds.getY()) / cellH);

    if (row < 0 || row >= pads || col < 0 || col >= steps)
        return;

    const bool current = data.getStepState(row, col);
    data.setStepState(row, col, !current);
    repaint();
}

void SequencerGrid::resized()
{
}

void SequencerGrid::timerCallback()
{
    const int step = data.getCurrentStep();
    if (step != lastStep)
    {
        lastStep = step;
        repaint();
    }
}
