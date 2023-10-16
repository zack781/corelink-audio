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
class Receiver2AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Receiver2AudioProcessorEditor (Receiver2AudioProcessor&);
    ~Receiver2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    Receiver2AudioProcessor& audioProcessor;
    
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Receiver2AudioProcessorEditor)
};
