/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Corelink_audio_receiverAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Corelink_audio_receiverAudioProcessorEditor (Corelink_audio_receiverAudioProcessor&);
    ~Corelink_audio_receiverAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    Corelink_audio_receiverAudioProcessor& audioProcessor;
    
    // UI HERE
    
    juce::Image corelink_logo;
    
    juce::Label workspace_label;
    
    juce::Label stream_type_label;
    
    juce::Label host_id_label;
    
    juce::Label buffer_size_label;
    
    juce::Label dir_path_label;

        
    juce::TextEditor workspace_edit;
    
    juce::TextEditor stream_type_edit;
    
    juce::TextEditor host_id_edit;
    
    juce::TextEditor buffer_size_edit;
    
    juce::TextEditor dir_path_edit;
    
    juce::TextButton submitBtn;
    
    // ------------------------------------

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Corelink_audio_receiverAudioProcessorEditor)
};
