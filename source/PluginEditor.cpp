#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Configure the slider
    gainSlider.setSliderStyle (juce::Slider::Rotary);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);

    // Create an attachment to link the slider to the AudioProcessorValueTreeState parameter
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processorRef.parameters, 
            "gain",   // this ID must match the parameter ID
            gainSlider);

    addAndMakeVisible (inspectButton);
    addAndMakeVisible (gainSlider);

    // this chunk of code instantiates and opens the melatonin inspector
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }

        inspector->setVisible (true);
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //auto area = getLocalBounds();
    //g.setColour (juce::Colours::white);
    //g.setFont (16.0f);
    //auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    //g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    auto area = getLocalBounds();
    area.removeFromBottom(50);
    //inspectButton.setBounds (getLocalBounds().withSizeKeepingCentre(100, 50));
    inspectButton.setBounds(0, getHeight() - 50, 100, 50);

    // Place the slider in the editor
    gainSlider.setBounds (getLocalBounds().reduced (100));
}
