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
class MainComponent   : public AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    
    void readAudioFiles();
    void updateParameters();
    void reset();
    void clearBuffers();
    
    dsp::Convolution convFL, convFR, convC, convRL, convRR;
    AudioBuffer<float> buffer_chunk_FL, buffer_chunk_FR, buffer_chunk_C, buffer_chunk_LFE, buffer_chunk_RL, buffer_chunk_RR;
    
    AudioFormatManager formatManager;

private:
    //==============================================================================
    // Your private member variables go here...
    
    double sampleRate = 44100;
    int buffer_size = 512;
    int position = 0;
    bool files_read = false;
    
    Slider hrirLenSliderH;
    Slider hrirLenSliderV;
    TextButton loadButton;
    
    File hrirFL, hrirFR, hrirC, hrirRL, hrirRR;
    
    // Build File Paths
    File app_path = File::getSpecialLocation (File::currentApplicationFile);
    String app_path_string = app_path.getFullPathName();
    int build_index = app_path_string.indexOf("Builds");
    String base_dir = app_path_string.substring(0, build_index);
    String audio_file_dir = base_dir + "data/audio_content/";
    String hrir_file_dir = base_dir + "data/hrir/";
    
    File file_01_FL, file_02_FR, file_03_C, file_04_LFE, file_05_RL,  file_06_RR;
    
    AudioFormatReader* reader_FL, *reader_FR, *reader_C, *reader_LFE, *reader_RL, *reader_RR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
