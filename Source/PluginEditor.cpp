/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "doomgeneric.h"
#include "doomgeneric_vst.h"

#define CONTROLS_HEIGHT 50

extern "C" {
    void D_DoomLoop_SingleFrame(void);
}

void setBoundsForCombobox(juce::ComboBox& combobox, int y, int width, int height, int padding, juce::ComboBox* previousComboBox)
{
    combobox.setBounds(
        padding + (previousComboBox != nullptr 
            ? previousComboBox->getBounds().getWidth() + previousComboBox->getBounds().getX() 
            : 0), 
        y,
        width-padding,
        height);
}

//==============================================================================
DoomVstAudioProcessorEditor::DoomVstAudioProcessorEditor (DoomVstAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    if (audioProcessor.wadFound) {
        Timer::startTimerHz(FRAMES_PER_SEC);
        juce::Image image(juce::Image::PixelFormat::ARGB, DOOMGENERIC_RESX, DOOMGENERIC_RESY, false);
        frameBuffer.setImage(image);
        frameBuffer.setSize(DOOMGENERIC_RESX, DOOMGENERIC_RESY);
        addAndMakeVisible(frameBuffer);
    }
    else {
        juce::String message = juce::String("Could not find a Doom IWAD file in the current directory. Copy doom.wad or doom2.wad into the directory:\r\n\r\n") + juce::File::getCurrentWorkingDirectory().getFullPathName() + "\r\n\r\nand restart the plugin.";
        errorMessage.setText(message, juce::NotificationType::dontSendNotification);
        addAndMakeVisible(errorMessage);
    }

    // setting up combo boxes
    setupComboboxAndLabel("Left", leftArrowComboBox, leftArrowLabel, 1);
    setupComboboxAndLabel("Up", upArrowComboBox, upArrowLabel, 2);
    setupComboboxAndLabel("Right", rightArrowComboBox, rightArrowLabel, 3);
    setupComboboxAndLabel("Down", downArrowComboBox, downArrowLabel, 4);
    setupComboboxAndLabel("Enter", enterComboBox, enterLabel, 5);
    setupComboboxAndLabel("Fire", fireComboBox, fireLabel, 6);
    setupComboboxAndLabel("Use", useComboBox, useLabel, 7);
    setupComboboxAndLabel("R Shift", rshiftComboBox, rshiftLabel, 8);


    setSize (DOOMGENERIC_RESX, DOOMGENERIC_RESY + CONTROLS_HEIGHT);
}

DoomVstAudioProcessorEditor::~DoomVstAudioProcessorEditor()
{
}

//==============================================================================
void DoomVstAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    if (!audioProcessor.wadFound)
        return;

    D_DoomLoop_SingleFrame();
    juce::Image::BitmapData data(frameBuffer.getImage(), juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < DOOMGENERIC_RESY; y++) {
        for (int x = 0; x < DOOMGENERIC_RESX; x++) {
            uint32_t pixel = mainFrameBuffer[y * DOOMGENERIC_RESX + x] | 0xFF000000UL;
            data.setPixelColour(x, y, juce::Colour(pixel));
        }
    }
}

void DoomVstAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    if (audioProcessor.wadFound) {
        auto area = getLocalBounds();
        int comboBoxPadding = 8;
        int comboBoxWidth = DOOMGENERIC_RESX/8;
        int comboBoxHeight = 20;
        frameBuffer.setBounds(0, 0, DOOMGENERIC_RESX, DOOMGENERIC_RESY);

        setBoundsForCombobox(leftArrowComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, nullptr);
        setBoundsForCombobox(upArrowComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, &leftArrowComboBox);
        setBoundsForCombobox(rightArrowComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, &upArrowComboBox);
        setBoundsForCombobox(downArrowComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, &rightArrowComboBox);
        setBoundsForCombobox(enterComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, &downArrowComboBox);
        setBoundsForCombobox(fireComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, &enterComboBox);
        setBoundsForCombobox(useComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, &fireComboBox);
        setBoundsForCombobox(rshiftComboBox, area.getHeight() - comboBoxHeight, comboBoxWidth, comboBoxHeight, comboBoxPadding, &useComboBox);
    }
    else {
        errorMessage.setBounds(0, 0, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
    }
}

void DoomVstAudioProcessorEditor::timerCallback()
{
    frameBuffer.repaint();
}

void DoomVstAudioProcessorEditor::setupComboboxAndLabel(const char* name, juce::ComboBox& combobox, juce::Label& label, int selectedItem)
{
    combobox.addItem("C", 1);
    combobox.addItem("C#", 2);
    combobox.addItem("D", 3);
    combobox.addItem("D#", 4);
    combobox.addItem("E", 5);
    combobox.addItem("F", 6);
    combobox.addItem("F#", 7);
    combobox.addItem("G", 8);
    combobox.addItem("G#", 9);
    combobox.addItem("A", 10);
    combobox.addItem("A#", 11);
    combobox.addItem("B", 12);
    combobox.setSelectedId(selectedItem);
    addAndMakeVisible(combobox);

    label.setText(name, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(label);
    label.attachToComponent(&combobox, false);

    combobox.onChange = [this, &combobox, &label] { 
        this->comboboxChanged(label.getText(), combobox); 
    };
}

void DoomVstAudioProcessorEditor::comboboxChanged(const juce::String& name, juce::ComboBox& combobox)
{
    juce::Logger::getCurrentLogger()->writeToLog(name + ": " + juce::String(combobox.getSelectedId()));
}