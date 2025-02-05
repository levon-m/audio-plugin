#include "PluginProcessor.h"
#include "PluginEditor.h"

// Construction and Deconstruction
// adds inputs/outputs depending on if the plugin is synth, midi, or neither
// Constructor runs when plugin opens, Deconstructor runs with it closes
//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
    // Initialize our 'parameters' member using the parameter layout
     , parameters (*this, nullptr, "PARAMETER_STATE", createParameterLayout())
{
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================

// This function creates our parameter(s). We create a "gain" float parameter here.
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    // We'll store parameters in a vector, then return it
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Create a float parameter, with an ID, name, range, default value, etc.
    // ID must match what you'll use in the editor's attachment
    auto gainParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"gain", 1},  // Parameter ID + version
        "Gain",                         // Parameter name (shown in DAW)
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 1.0f),  // range: 0.0 to 2.0
        1.0f                            // default value
    );
    params.push_back(std::move(gainParam));

    return { params.begin(), params.end() };
}

// Name
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

// Midi
//------------------------------------------
bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
//------------------------------------------

// Defines audio tail (reverb, delay, etc.) after audio stops
double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

// Mostly placeholders for presets/programs
//------------------------------------------------------------
int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}
//------------------------------------------------------------

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    // Prepare for floating-point arithmetic
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);
    
    // Get the number of channels and samples
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    // This code clears any output channels that didn't contain
    // input data to avoid feedback, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    // Get the current value of the gain parameter in real time
    float currentGain = *parameters.getRawParameterValue("gain");

    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    // Channel Loop
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // Pointer to this channel's sample data
        auto* channelData = buffer.getWritePointer(channel);
        
        // Sample Loop
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Sample application
            channelData[sample] *= currentGain;
        }
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    // Let the AudioProcessorValueTreeState handle serialization
    // WTF does this do exactly??
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // Let the AudioProcessorValueTreeState handle deserialization
    // ??????
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml.get() != nullptr)
    {
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
