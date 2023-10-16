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
class SenderAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SenderAudioProcessorEditor (SenderAudioProcessor&);
    ~SenderAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SenderAudioProcessor& audioProcessor;
    
    // Front end initialization
    
//    juce::Component main_component();
    
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
    
    // ------------------------

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SenderAudioProcessorEditor)
};
