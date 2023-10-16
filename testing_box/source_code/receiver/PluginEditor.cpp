/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Receiver2AudioProcessorEditor::Receiver2AudioProcessorEditor (Receiver2AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    corelink_logo = juce::ImageCache::getFromMemory(BinaryData::CorelinkLogo_jpg, BinaryData::CorelinkLogo_jpgSize);
    
    addAndMakeVisible(workspace_label);
    workspace_label.setText("Workspace", juce::dontSendNotification);
    workspace_label.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(workspace_edit);
    
    addAndMakeVisible(stream_type_label);
    stream_type_label.setText("Stream Type", juce::dontSendNotification);
    stream_type_label.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(stream_type_edit);
    
    addAndMakeVisible(host_id_label);
    host_id_label.setText("Host ID", juce::dontSendNotification);
    host_id_label.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(host_id_edit);
    
    addAndMakeVisible(buffer_size_label);
    buffer_size_label.setText("Buffering Size", juce::dontSendNotification);
    buffer_size_label.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(buffer_size_edit);
    
    addAndMakeVisible(dir_path_label);
    dir_path_label.setText("Dir Path", juce::dontSendNotification);
    dir_path_label.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(dir_path_edit);
    
    addAndMakeVisible(submitBtn);
    submitBtn.setButtonText("Connect Receiver");
    
    submitBtn.onClick = [this] () -> void {
        audioProcessor.set_audio_workspace(workspace_edit.getText());
        audioProcessor.set_audio_stream_type(stream_type_edit.getText());
        audioProcessor.set_host_id(host_id_edit.getText());
        audioProcessor.set_buffering_size(buffer_size_edit.getText());
        audioProcessor.set_dir_path(dir_path_edit.getText());
        audioProcessor.set_stream_init();
        // repaint() triggers the paint function to be called again
        this->repaint();
        DBG("onClick");
    };
        
    setSize (400, 300);
}

Receiver2AudioProcessorEditor::~Receiver2AudioProcessorEditor()
{
}

//==============================================================================
void Receiver2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::white);

    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    
    juce::Rectangle<int> bounds = getLocalBounds();
    
    g.drawImageWithin(corelink_logo, bounds.getWidth()/2 - 50, bounds.getHeight()/2 - 120, 100, 40, juce::RectanglePlacement::stretchToFit);
    
    if (!audioProcessor.get_stream_init())
    {
//        DBG("stream_init = false");
        this->host_id_label.attachToComponent(&host_id_edit, true);
        host_id_edit.setBounds(bounds.getWidth()/2, bounds.getHeight()/2 - 60, 100, 20);
        
        this->workspace_label.attachToComponent(&workspace_edit, true);
        workspace_edit.setBounds(bounds.getWidth()/2, bounds.getHeight()/2 - 30, 100, 20);
        
        this->stream_type_label.attachToComponent(&stream_type_edit, true);
        stream_type_edit.setBounds(bounds.getWidth()/2, bounds.getHeight()/2, 100, 20);
        
        this->buffer_size_label.attachToComponent(&buffer_size_edit, true);
        buffer_size_edit.setBounds(bounds.getWidth()/2, bounds.getHeight()/2 + 30, 100, 20);
        
        this->dir_path_label.attachToComponent(&dir_path_edit, true);
        dir_path_edit.setBounds(bounds.getWidth()/2, bounds.getHeight()/2 + 60, 100, 20);
        
        submitBtn.setBounds(bounds.getWidth()/2 - 75/2, bounds.getHeight()/2 + 90, 75, 30);
    }else{
        host_id_edit.clear();
        host_id_edit.setVisible(false);
        
        workspace_edit.clear();
        workspace_edit.setVisible(false);
        
        stream_type_edit.clear();
        stream_type_edit.setVisible(false);
        
        buffer_size_edit.clear();
        buffer_size_edit.setVisible(false);
        
        dir_path_edit.clear();
        dir_path_edit.setVisible(false);
        
        submitBtn.setVisible(false);
        
        g.drawText("Host ID:", bounds.getWidth()/2 - 100, bounds.getHeight()/2 - 60, 100, 20, juce::Justification::left, true);
        g.drawText(audioProcessor.get_host_id(), bounds.getWidth()/2 - 25, bounds.getHeight()/2 - 60, 100, 20, juce::Justification::right, true);
        
        g.drawText("Workspace:", bounds.getWidth()/2 - 100, bounds.getHeight()/2 - 30, 100, 20, juce::Justification::left, true);
        g.drawText(audioProcessor.get_audio_workspace(), bounds.getWidth()/2 - 25, bounds.getHeight()/2 - 30, 100, 20, juce::Justification::right, true);
        
        g.drawText("Stream Type:", bounds.getWidth()/2 - 100, bounds.getHeight()/2, 100, 20, juce::Justification::left, true);
        g.drawText(audioProcessor.get_audio_stream_type(), bounds.getWidth()/2 - 25, bounds.getHeight()/2, 100, 20, juce::Justification::right, true);
        
        g.drawText("Dir Path:", bounds.getWidth()/2 - 100, bounds.getHeight()/2 + 30, 100, 20, juce::Justification::left, true);
        juce::String txt = audioProcessor.get_dir_path();
        g.drawText(txt, bounds.getWidth()/2 - 25, bounds.getHeight()/2 + 30, 200, 20, juce::Justification::right, true);
        
        g.drawText("Buffering Size:", bounds.getWidth()/2 - 100, bounds.getHeight()/2 + 60, 100, 20, juce::Justification::left, true);
        g.drawText(std::to_string(audioProcessor.get_buffering_size()), bounds.getWidth()/2 - 25, bounds.getHeight()/2 + 60, 100, 20, juce::Justification::right, true);
    }
    
}

void Receiver2AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
