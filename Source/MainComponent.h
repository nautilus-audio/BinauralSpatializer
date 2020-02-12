/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent,
                        public ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    AudioTransportSource transport1;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    
    enum TransportState
    {
        Stopped,
        Starting,
        Stopping,
        Playing
    };
    
    TransportState state;
    
    void loadButtonClicked();
    void loadFile(AudioFormatReader* reader, File file);
    void playButtonClicked();
    void stopButtonClicked();
    void transportStateChanged(TransportState newState);
    void changeListenerCallback (ChangeBroadcaster *source) override;
    void process();
    void updateParameters();
    void reset();
    
    AudioFormatManager formatManager;
    
    std::unique_ptr<AudioFormatReaderSource> playSource, playSource2;
    
    dsp::Convolution convFL, convFR, convC, convRL, convRR;
    AudioBuffer<float> buffer_FL, buffer_FR, buffer_C, buffer_LFE, buffer_RL, buffer_RR;
    AudioBuffer<float> buffer_out;
    double sampleRate = 44100;
    bool written = false;
    MixerAudioSource mixer;
    AudioSourcePlayer player;
    
    Slider hrirLenSlider;
    TextButton loadButton;
    TextButton playButton;
    TextButton stopButton;
    
    File hrirFL, hrirFR, hrirC, hrirRL, hrirRR;
    File file_01_FL, file_02_FR, file_03_C, file_04_LFE, file_05_RL, file_06_RR;
    File binauralOut;
    
    // Build File Paths
    File app_path = File::getSpecialLocation (File::currentApplicationFile);
    String app_path_string = app_path.getFullPathName();
    int build_index = app_path_string.indexOf("Builds");
    String base_dir = app_path_string.substring(0, build_index);
    String audio_file_dir = base_dir + "data/audio_content/";
    String hrir_file_dir = base_dir + "data/hrir/";
    
    enum Channels {
        LEFT = 0,
        RIGHT = 1
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
