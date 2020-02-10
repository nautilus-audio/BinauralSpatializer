/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include <stdlib.h>
#include <string>
#include <iostream>

//==============================================================================
MainComponent::MainComponent() :state(Stopped), loadButton("Process & Load File"), playButton("Play"), stopButton("Stop")
{
    // Make sure you set the size of the component after
    // you add any child components.
    
    loadButton.onClick = [this] {  loadButtonClicked();  loadButton.setColour(TextButton::buttonColourId, Colours::yellow);};
    addAndMakeVisible(&loadButton);
    
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.setEnabled(true);
    addAndMakeVisible(&playButton);
    
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled(false);
    addAndMakeVisible(&stopButton);
    
    formatManager.registerBasicFormats();
    transport1.addChangeListener(this);
    
    setSize (600, 400);
    
    file_01_FL = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/audio_content/file-01-FL.wav");
    file_02_FR = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/audio_content/file-02-FR.wav");
    file_03_C = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/audio_content/file-03-C.wav");
    file_04_LFE = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/audio_content/file-04-LFE.wav");
    file_05_RL = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/audio_content/file-05-RL.wav");
    file_06_RR = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/audio_content/file-06-RR.wav");
    
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
        buffer_out.setSize(2, (int) reader_FL->lengthInSamples);
        buffer_FL.setSize(2, (int) reader_FL->lengthInSamples); //maybe reader_FL->lengthInSamples
        reader_FL->read (&buffer_FL, 0, (int) reader_FL->lengthInSamples, 0, true, true);
    }
    
    if (reader_FR != nullptr)
    {
        buffer_FR.setSize(2, (int) reader_FR->lengthInSamples);
        reader_FR->read (&buffer_FR, 0, (int) reader_FR->lengthInSamples, 0, true, true);
    }
    
    if (reader_C != nullptr)
    {
        buffer_C.setSize(2, (int) reader_C->lengthInSamples);
        reader_C->read (&buffer_C, 0, (int) reader_C->lengthInSamples, 0, true, true);
    }
    
    if (reader_LFE != nullptr)
    {
        buffer_LFE.setSize(2, (int) reader_LFE->lengthInSamples);
        reader_LFE->read (&buffer_LFE, 0, (int) reader_LFE->lengthInSamples, 0, true, true);
    }
    
    if (reader_RL != nullptr)
    {
        buffer_RL.setSize(2, (int) reader_RL->lengthInSamples);
        reader_RL->read (&buffer_RL, 0, (int) reader_RL->lengthInSamples, 0, true, true);
    }
    
    if (reader_RR != nullptr)
    {
        buffer_RR.setSize(2, (int) reader_RR->lengthInSamples);
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
    spec.numChannels = buffer_out.getNumChannels();
    convFL.prepare(spec);
    convFR.prepare(spec);
    convC.prepare(spec);
    convRL.prepare(spec);
    convRR.prepare(spec);
    updateParameters();
    
    //mixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    //player.prepareToPlay(sampleRate, samplesPerBlockExpected);
    transport1.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::loadButtonClicked()
{
    //process();
    transportStateChanged(Stopped);
    process();
    auto reader_out = formatManager.createReaderFor(binauralOut);
    std::unique_ptr<AudioFormatReaderSource> Out_Source (new AudioFormatReaderSource (reader_out, true));
    transport1.setSource(Out_Source.get(), 0, nullptr);
    playSource.reset(Out_Source.release());
}

void MainComponent::loadFile(AudioFormatReader* reader, File file)
{
    if (reader != nullptr)
    {
        String file_name = file.getFileName();
        
        std::cout << file_name << std::endl; //Trace
        
        // Pass into new variable with position+_Source
        std::unique_ptr<AudioFormatReaderSource> source (new AudioFormatReaderSource (reader, true));
        
        //get the file ready to playx
        transportStateChanged(Stopped);
        playSource.reset(source.release());
    }
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
    
    // For more details, see the help for AudioProcessor::getNextAudioBlock()
    
    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
    auto samp2 = bufferToFill.numSamples;
    if (!written) {
//        process();
        written = true;
    }
    transport1.getNextAudioBlock(bufferToFill);
}

void MainComponent::process()
{
    // start buffer loop
//    for (int sample = 0; sample < buffer_FL.getNumSamples(); sample++) {
//
//    }
    
    // Load buffers into audio blocks
    dsp::AudioBlock<float> block_FL (buffer_FL);
    dsp::AudioBlock<float> block_FR (buffer_FR);
    dsp::AudioBlock<float> block_C (buffer_C);
    dsp::AudioBlock<float> block_LFE (buffer_LFE);
    dsp::AudioBlock<float> block_RL (buffer_RL);
    dsp::AudioBlock<float> block_RR (buffer_RR);
    
    // Get contexts
    dsp::ProcessContextReplacing<float> context_FL = dsp::ProcessContextReplacing<float>(block_FL);
    dsp::ProcessContextReplacing<float> context_FR = dsp::ProcessContextReplacing<float>(block_FR);
    dsp::ProcessContextReplacing<float> context_C = dsp::ProcessContextReplacing<float>(block_C);
    dsp::ProcessContextReplacing<float> context_RL = dsp::ProcessContextReplacing<float>(block_RL);
    dsp::ProcessContextReplacing<float> context_RR = dsp::ProcessContextReplacing<float>(block_RR);
    
    // Perform Covolutions
    convFL.process(dsp::ProcessContextReplacing<float> (context_FL));
    convFR.process(dsp::ProcessContextReplacing<float> (context_FR));
    convC.process(dsp::ProcessContextReplacing<float> (context_C));
    convRL.process(dsp::ProcessContextReplacing<float> (context_RL));
    convRR.process(dsp::ProcessContextReplacing<float> (context_RR));
    //end buffer loop
    
    //get output block from engine
    dsp::AudioBlock<float> outputBlockFL = context_FL.getOutputBlock();
    dsp::AudioBlock<float> outputBlockFR = context_FR.getOutputBlock();
    dsp::AudioBlock<float> outputBlockC = context_C.getOutputBlock();
    dsp::AudioBlock<float> outputBlockRL = context_RL.getOutputBlock();
    dsp::AudioBlock<float> outputBlockRR = context_RR.getOutputBlock();

    // Sum to Out Buffer and write to wav file
    dsp::AudioBlock<float> outBlock (buffer_out);
    outBlock.copy(outputBlockFL.add(outputBlockFR).add(outputBlockC).add(block_LFE).add(outputBlockRL).add(outputBlockRR));
    AudioFormatWriter* writer_out;
    WavAudioFormat waf;
    StringPairArray meta;
    binauralOut = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/output/binaural_output.wav");
    FileOutputStream* outputTo = binauralOut.createOutputStream();
    writer_out = waf.createWriterFor(outputTo,sampleRate,2,16,meta,true);
    writer_out->writeFromAudioSampleBuffer (buffer_out, 0, buffer_out.getNumSamples());
    delete writer_out;
}

void MainComponent::updateParameters()
{
    // update process params
    // load impulse responses
    hrirFL = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/hrir/hrirFL.wav");
    hrirFR = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/hrir/hrirFR.wav");
    hrirC = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/hrir/hrirC.wav");
    hrirRL = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/hrir/hrirRL.wav");
    hrirRR = File("/Users/kamilahmitchell/Desktop/JUCE/EmbodyChallenge/data/hrir/hrirRR.wav");
    
    auto maxSize = static_cast<size_t> (roundToInt (sampleRate * (8192.0 / 44100.0)));
    
    convFL.loadImpulseResponse(hrirFL, true, false, maxSize, true);
    convFR.loadImpulseResponse(hrirFR, true, false, maxSize, true);
    convC.loadImpulseResponse(hrirC, true, false, maxSize, true);
    convRL.loadImpulseResponse(hrirRL, true, false, maxSize, true);
    convRR.loadImpulseResponse(hrirRR, true, false, maxSize, true);
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
    mixer.releaseResources();
    transport1.releaseResources();
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
}
