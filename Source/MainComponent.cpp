/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include "FileReader.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    
    hrirLenSliderH.setRange(0, 400);
    hrirLenSliderH.onValueChange = [this] {updateParameters();};
    addAndMakeVisible(&hrirLenSliderH);
    
    buffer_chunk_FL.setSize(2, buffer_size);
    buffer_chunk_FR.setSize(2, buffer_size);
    buffer_chunk_C.setSize(2, buffer_size);
    buffer_chunk_LFE.setSize(2, buffer_size);
    buffer_chunk_RL.setSize(2, buffer_size);
    buffer_chunk_RR.setSize(2, buffer_size);
    
    setSize (600, 600);
    
    formatManager.registerBasicFormats();
    
    // Initalize audio files via paths
    file_01_FL = File(audio_file_dir + "file-01-FL.wav");
    file_02_FR = File(audio_file_dir + "file-02-FR.wav");
    file_03_C = File(audio_file_dir + "file-03-C.wav");
    file_04_LFE = File(audio_file_dir + "file-04-LFE.wav");
    file_05_RL = File(audio_file_dir + "file-05-RL.wav");
    file_06_RR = File(audio_file_dir + "file-06-RR.wav");
    
    // init readers for audio files
    reader_FL = formatManager.createReaderFor(file_01_FL);
    reader_FR = formatManager.createReaderFor(file_02_FR);
    reader_C = formatManager.createReaderFor(file_03_C);
    reader_LFE = formatManager.createReaderFor(file_04_LFE);
    reader_RL = formatManager.createReaderFor(file_05_RL);
    reader_RR = formatManager.createReaderFor(file_06_RR);
    
    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    
    buffer_chunk_FL.clear();
    buffer_chunk_FR.clear();
    buffer_chunk_C.clear();
    buffer_chunk_LFE.clear();
    buffer_chunk_RL.clear();
    buffer_chunk_RR.clear();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    int numOutChannels = deviceManager.getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();
    
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = numOutChannels;
    convFL.prepare(spec);
    convFR.prepare(spec);
    convC.prepare(spec);
    convRL.prepare(spec);
    convRR.prepare(spec);
    updateParameters();
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!
    
    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
    
    
    // find max samples from all 6 buffers
    int max_samples_of_front_files = jmax(int(reader_FL->lengthInSamples), int(reader_FR->lengthInSamples), int(reader_C->lengthInSamples));
    int max_samples_of_back_files = jmax(int(reader_LFE->lengthInSamples), int(reader_RL->lengthInSamples), int(reader_RR->lengthInSamples));
    int max_samples_of_input_files = jmax(max_samples_of_front_files, max_samples_of_back_files);
    
    // int numOutputChannels = bufferToFill.buffer->getNumChannels();
    auto outputSamplesRemaining = bufferToFill.numSamples;
    
    // loop max samples into bufferToFill until exhausted
    auto bufferSamplesRemaining = max_samples_of_input_files - position;
    auto samplesToProcess = jmin (outputSamplesRemaining, bufferSamplesRemaining);
    
    //read file chunks into buffers for convolution
    if (reader_FL != nullptr)
    {
        // buffer, startSampleDestbuffer, numSamples, readerStartSample
        reader_FL->read (&buffer_chunk_FL, 0, samplesToProcess, position, true, true);
    }
    
    if (reader_FR != nullptr)
    {
        reader_FR->read (&buffer_chunk_FR, 0, samplesToProcess, position, true, true);
    }
    
    if (reader_C != nullptr)
    {
        reader_C->read (&buffer_chunk_C, 0, samplesToProcess, position, true, true);
    }
    
    if (reader_LFE != nullptr)
    {
        reader_LFE->read (&buffer_chunk_LFE, 0, samplesToProcess, position, true, true);
    }
    
    if (reader_RL != nullptr)
    {
        reader_RL->read (&buffer_chunk_RL, 0, samplesToProcess, position, true, true);
    }
    
    if (reader_RR != nullptr)
    {
        reader_RR->read (&buffer_chunk_RR, 0, samplesToProcess, position, true, true);
    }
    
    // Load buffer chunks into audio blocks, for performing convolutions in the frequency domain
    dsp::AudioBlock<float> block_FL (buffer_chunk_FL);
    dsp::AudioBlock<float> block_FR (buffer_chunk_FR);
    dsp::AudioBlock<float> block_C (buffer_chunk_C);
    dsp::AudioBlock<float> block_LFE (buffer_chunk_LFE);
    dsp::AudioBlock<float> block_RL (buffer_chunk_RL);
    dsp::AudioBlock<float> block_RR (buffer_chunk_RR);
    
    // Get contexts
    dsp::ProcessContextReplacing<float> context_FL = dsp::ProcessContextReplacing<float>(block_FL);
    dsp::ProcessContextReplacing<float> context_FR = dsp::ProcessContextReplacing<float>(block_FR);
    dsp::ProcessContextReplacing<float> context_C = dsp::ProcessContextReplacing<float>(block_C);
    dsp::ProcessContextReplacing<float> context_RL = dsp::ProcessContextReplacing<float>(block_RL);
    dsp::ProcessContextReplacing<float> context_RR = dsp::ProcessContextReplacing<float>(block_RR);
    
    // Perform Covolutions
    convFL.process(context_FL);
    convFR.process(context_FR);
    convC.process(context_C);
    convRL.process(context_RL);
    convRR.process(context_RR);
    
    //get output blocks from engine
    dsp::AudioBlock<float> outputBlockFL = context_FL.getOutputBlock();
    dsp::AudioBlock<float> outputBlockFR = context_FR.getOutputBlock();
    dsp::AudioBlock<float> outputBlockC = context_C.getOutputBlock();
    dsp::AudioBlock<float> outputBlockRL = context_RL.getOutputBlock();
    dsp::AudioBlock<float> outputBlockRR = context_RR.getOutputBlock();
    
    // Sum to Out Buffer
    dsp::AudioBlock<float> outBlock (*bufferToFill.buffer);
    outBlock.copyFrom(outputBlockFL.add(outputBlockFR).add(outputBlockC).add(block_LFE).add(outputBlockRL).add(outputBlockRR));
    
    position += samplesToProcess;
    
    // Loop when audio is finished
    if (position == max_samples_of_input_files)
        position = 0;
}

void MainComponent::updateParameters()
{
    // update process params
    // load impulse responses
    hrirFL = File(hrir_file_dir + "hrirFL.wav");
    hrirFR = File(hrir_file_dir + "hrirFR.wav");
    hrirC = File(hrir_file_dir + "hrirC.wav");
    hrirRL = File(hrir_file_dir + "hrirRL.wav");
    hrirRR = File(hrir_file_dir + "hrirRR.wav");
    
    size_t currentSliderHValue = hrirLenSliderH.getValue();
    
    convFL.loadImpulseResponse(hrirFL, true, false, currentSliderHValue, true);
    convFR.loadImpulseResponse(hrirFR, true, false, currentSliderHValue, true);
    convC.loadImpulseResponse(hrirC, true, false, currentSliderHValue, true);
    convRL.loadImpulseResponse(hrirRL, true, false, currentSliderHValue, true);
    convRR.loadImpulseResponse(hrirRR, true, false, currentSliderHValue, true);
}

void MainComponent::reset()
{
    convFL.reset();
    convFR.reset();
    convC.reset();
    convRL.reset();
    convRR.reset();
}

void MainComponent::clearBuffers()
{
    buffer_chunk_FL.clear();
    buffer_chunk_FR.clear();
    buffer_chunk_C.clear();
    buffer_chunk_LFE.clear();
    buffer_chunk_RL.clear();
    buffer_chunk_RR.clear();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    reset();
    clearBuffers();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setOpacity(1.0f);

}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    hrirLenSliderH.setBounds(10, 120, getWidth() - 20, 30);
}
