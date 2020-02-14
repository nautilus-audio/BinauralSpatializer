/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>

//==============================================================================
MainComponent::MainComponent() :state(Stopped), loadButton("Process & Load"), playButton("Play"), stopButton("Stop")
{
    // Make sure you set the size of the component after
    // you add any child components.
    
    loadButton.onClick = [this] {  loadButtonClicked();  loadButton.setColour(TextButton::buttonColourId, Colours::yellow);};
    addAndMakeVisible(&loadButton);
    
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.setEnabled(true);
    addAndMakeVisible(&playButton);
    
    stopButton.onClick = [this] { stopButtonClicked(); loadButton.setColour(TextButton::buttonColourId, Colours::grey);};
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled(false);
    addAndMakeVisible(&stopButton);
    
    //auto maxSize = static_cast<size_t> (roundToInt (sampleRate * (8192.0 / 44100.0)));
    hrirLenSlider.setRange(0, 400);
    hrirLenSlider.onValueChange = [this] {updateParameters();};
    addAndMakeVisible(&hrirLenSlider);
    
    formatManager.registerBasicFormats();
    transport1.addChangeListener(this);
    
    setSize (600, 400);
    
    file_01_FL = File(audio_file_dir + "file-01-FL.wav");
    file_02_FR = File(audio_file_dir + "file-02-FR.wav");
    file_03_C = File(audio_file_dir + "file-03-C.wav");
    file_04_LFE = File(audio_file_dir + "file-04-LFE.wav");
    file_05_RL = File(audio_file_dir + "file-05-RL.wav");
    file_06_RR = File(audio_file_dir + "file-06-RR.wav");

    //read the files
    AudioFormatReader* reader_FL = formatManager.createReaderFor(file_01_FL);
    AudioFormatReader* reader_FR = formatManager.createReaderFor(file_02_FR);
    AudioFormatReader* reader_C = formatManager.createReaderFor(file_03_C);
    AudioFormatReader* reader_LFE = formatManager.createReaderFor(file_04_LFE);
    AudioFormatReader* reader_RL = formatManager.createReaderFor(file_05_RL);
    AudioFormatReader* reader_RR = formatManager.createReaderFor(file_06_RR);
    
    //read files into buffers for convolution
    if (reader_FL != nullptr)
    {
        buffer_FL.setSize(2, (int) reader_FL->lengthInSamples); //maybe reader_FL->lengthInSamples
        buffer_chunk_FL.setSize(buffer_FL.getNumChannels(), buffer_size);
        reader_FL->read (&buffer_FL, 0, (int) reader_FL->lengthInSamples, 0, true, true);
    }
    
    if (reader_FR != nullptr)
    {
        buffer_FR.setSize(2, (int) reader_FR->lengthInSamples);
        buffer_chunk_FR.setSize(buffer_FR.getNumChannels(), buffer_size);
        reader_FR->read (&buffer_FR, 0, (int) reader_FR->lengthInSamples, 0, true, true);
    }
    
    if (reader_C != nullptr)
    {
        buffer_C.setSize(2, (int) reader_C->lengthInSamples);
        buffer_chunk_C.setSize(buffer_C.getNumChannels(), buffer_size);
        reader_C->read (&buffer_C, 0, (int) reader_C->lengthInSamples, 0, true, true);
    }
    
    if (reader_LFE != nullptr)
    {
        buffer_LFE.setSize(2, (int) reader_LFE->lengthInSamples);
        buffer_chunk_LFE.setSize(buffer_LFE.getNumChannels(), buffer_size);
        reader_LFE->read (&buffer_LFE, 0, (int) reader_LFE->lengthInSamples, 0, true, true);
    }
    
    if (reader_RL != nullptr)
    {
        buffer_RL.setSize(2, (int) reader_RL->lengthInSamples);
        buffer_chunk_RL.setSize(buffer_RL.getNumChannels(), buffer_size);
        reader_RL->read (&buffer_RL, 0, (int) reader_RL->lengthInSamples, 0, true, true);
    }
    
    if (reader_RR != nullptr)
    {
        buffer_RR.setSize(2, (int) reader_RR->lengthInSamples);
        buffer_chunk_RR.setSize(buffer_RR.getNumChannels(), buffer_size);
        reader_RR->read (&buffer_RR, 0, (int) reader_RR->lengthInSamples, 0, true, true);
    }

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
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = buffer_FL.getNumChannels();
    convFL.prepare(spec);
    convFR.prepare(spec);
    convC.prepare(spec);
    convRL.prepare(spec);
    convRR.prepare(spec);
    updateParameters();
    //transport1.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::loadButtonClicked()
{
    transportStateChanged(Stopped);
    //process();
    auto reader_out = formatManager.createReaderFor(binauralOut);
    std::unique_ptr<AudioFormatReaderSource> Out_Source (new AudioFormatReaderSource (reader_out, true));
    transport1.setSource(Out_Source.get(), 0, nullptr);
    playSource.reset(Out_Source.release());
}

void MainComponent::playButtonClicked()
{
    transportStateChanged(Starting);
}

void MainComponent::stopButtonClicked()
{
    transportStateChanged(Stopping);
}

void MainComponent::transportStateChanged(TransportState newState)
{
    if (newState != state)
    {
        state = newState;
        
        switch (state) {
            case Stopped:
                playButton.setEnabled(true);
                transport1.setPosition(0.0);
                break;
                
            case Playing:
                playButton.setEnabled(true);
                break;
                
            case Starting:
                stopButton.setEnabled(true);
                playButton.setEnabled(false);
                transport1.start();
                break;
                
            case Stopping:
                playButton.setEnabled(true);
                stopButton.setEnabled(false);
                transport1.stop();
                break;
        }
    }
}

void MainComponent::changeListenerCallback (ChangeBroadcaster *source)
{
    if (source == &transport1)
    {
        if (transport1.isPlaying())
        {
            transportStateChanged(Playing);
        }
        else
        {
            transportStateChanged(Stopped);
        }
    }
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!
    
    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
    
    // find max samples from all 6 buffers
    int max_samples_of_front_files = jmax(buffer_FL.getNumSamples(), buffer_FR.getNumSamples(), buffer_C.getNumSamples());
    int max_samples_of_back_files = jmax(buffer_LFE.getNumSamples(), buffer_RL.getNumSamples(), buffer_RR.getNumSamples());
    int max_samples_of_input_files = jmax(max_samples_of_front_files, max_samples_of_back_files);
    
    int numOutputChannels = bufferToFill.buffer->getNumChannels();
    auto outputSamplesRemaining = bufferToFill.numSamples;
    auto outputSamplesOffset = bufferToFill.startSample;
    
    //updateParameters();
    
    // start buffer loop
    // loop max samples into bufferToFill until exhausted
    while(outputSamplesRemaining > 0) {
        
        auto bufferSamplesRemaining = max_samples_of_input_files - position;
        auto samplesThisTime = jmin (outputSamplesRemaining, bufferSamplesRemaining);
        
        // chunk buffers into manageable portions
        for (int channel = 0; channel < numOutputChannels; ++channel) {
            buffer_chunk_FL.copyFrom (channel, 0, buffer_FL, channel, position, samplesThisTime);
            buffer_chunk_FR.copyFrom (channel, 0, buffer_FR, channel, position, samplesThisTime);
            buffer_chunk_C.copyFrom (channel, 0, buffer_C, channel, position, samplesThisTime);
            buffer_chunk_LFE.copyFrom (channel, 0, buffer_LFE, channel, position, samplesThisTime);
            buffer_chunk_RL.copyFrom (channel, 0, buffer_RL, channel, position, samplesThisTime);
            buffer_chunk_RR.copyFrom (channel, 0, buffer_RR, channel, position, samplesThisTime);
        }
    
        // Load buffers into audio blocks
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
        
        //get output block from engine
        dsp::AudioBlock<float> outputBlockFL = context_FL.getOutputBlock();
        dsp::AudioBlock<float> outputBlockFR = context_FR.getOutputBlock();
        dsp::AudioBlock<float> outputBlockC = context_C.getOutputBlock();
        dsp::AudioBlock<float> outputBlockRL = context_RL.getOutputBlock();
        dsp::AudioBlock<float> outputBlockRR = context_RR.getOutputBlock();
        
        // Sum to Out Buffer
        dsp::AudioBlock<float> outBlock (*bufferToFill.buffer);
        outBlock.copy(outputBlockFL.add(outputBlockFR).add(outputBlockC).add(block_LFE).add(outputBlockRL).add(outputBlockRR));
        
        
        outputSamplesRemaining -= samplesThisTime;
        outputSamplesOffset += samplesThisTime;
        position += samplesThisTime;
        
        if (position == max_samples_of_input_files)
            position = 0;
    }
    //end buffer loop
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
    
    convFL.loadImpulseResponse(hrirFL, true, false, hrirLenSlider.getValue(), true);
    convFR.loadImpulseResponse(hrirFR, true, false, hrirLenSlider.getValue(), true);
    convC.loadImpulseResponse(hrirC, true, false, hrirLenSlider.getValue(), true);
    convRL.loadImpulseResponse(hrirRL, true, false, hrirLenSlider.getValue(), true);
    convRR.loadImpulseResponse(hrirRR, true, false, hrirLenSlider.getValue(), true);
}

void MainComponent::reset()
{
    convFL.reset();
    convFR.reset();
    convC.reset();
    convRL.reset();
    convRR.reset();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    transport1.releaseResources();
    reset();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    loadButton.setBounds(10, 10, getWidth() - 20, 30);
    playButton.setBounds(10, 50, getWidth() - 20, 30);
    stopButton.setBounds(10, 90, getWidth() - 20, 30);
    hrirLenSlider.setBounds(10, 120, getWidth() - 20, 30);
}
