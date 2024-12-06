/*
  ==============================================================================

    LabeledComboBox.h
    Created: 6 Dec 2024 2:08:12am
    Author:  bvuong

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LabeledComboBox : public juce::ComboBox
{
public:
private:
    juce::String valueId
    juce::Label label;
}