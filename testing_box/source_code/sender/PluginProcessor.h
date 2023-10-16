

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

//#define CORELINK_USE_WEBSOCKET
#define CORELINK_USE_CONCURRENT_COUNTER
#define CORELINK_USE_CONCURRENT_QUEUE
#define CORELINK_USE_CONCURRENT_MAP
#define CORELINK_ENABLE_STRING_UTIL_FUNCTIONS


//#define CONNECT_TO_LOCAL
//
//#ifdef CONNECT_TO_LOCAL
//#define CORELINK_WEBSOCKET_CONNECT_LOCAL_SERVER
//#endif

#include <JuceHeader.h>
#include "corelink_all.hpp"
#include <iostream>
#include <future>



const size_t OUTPUT_SIZE = 1 * 1024 * 1024 * 1024;

namespace ns_cl_client = corelink::client;
namespace ns_cl_core = corelink::core;
namespace ns_cl_req_resp = ns_cl_client::request_response;
template<typename t> using in = corelink::in<t>;
template<typename t> using out = corelink::out<t>;

//==============================================================================
/**
*/
class SenderAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SenderAudioProcessor();
    ~SenderAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void on_channel_init(corelink::core::network::channel_id_type host_id);
    void on_channel_uninit(corelink::core::network::channel_id_type host_id);
    void on_error(corelink::core::network::channel_id_type host_id, in<std::string> err);
    void create_sender(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client);
    
    template<class T>
    void swapMove(T& a, T& b);
    
    void sendData(juce::AudioBuffer<float> buffer, int bufferSize, std::vector<uint8_t>& data);
    
    void set_audio_workspace(const juce::String& val);
    
    void set_audio_stream_type(const juce::String& val);
    
    void set_stream_init();
    
    bool get_stream_init();
    
    void set_host_id(const juce::String& val);
    
    void set_buffering_size(const juce::String& val);
    
    void client_request_create_sender();
    
    juce::String get_audio_workspace();
    juce::String get_audio_stream_type();
    juce::String get_host_id();
    int get_buffering_size();
    
    void client_request(corelink::client::corelink_classic_client& client, corelink::client::corelink_client_connection_info& info, corelink::core::network::channel_id_type& control_channel_id);
    
    void write_to_dat(std::vector<float> data, int counter, std::string& dir_path);
    
    void set_dir_path(const juce::String& val);
    
    std::string get_dir_path();

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SenderAudioProcessor)
    
    corelink::client::corelink_classic_client client;
    corelink::client::corelink_client_connection_info info;
    corelink::core::network::channel_id_type control_channel_id;
    
    std::vector<float> floatBuffer;
//    corelink::utils::json meta;
    int bufferSize;
    float sampleRate;
    
    std::string cert_path;
    
    bool done;
    bool loading;
    
    corelink::core::network::channel_id_type hostId;
    
    bool stream_init;
    
    juce::String audio_workspace = "ZackAudio";
    juce::String audio_stream_type = "audiotesting";
    juce::String corelink_host_id = "127.0.0.0";
    int buffering_size = 25;
    
    int counter = 0;
    std::string dir_path = "";
    
    std::vector<uint8_t> data;
    std::vector<uint8_t> data2;
    std::vector<uint8_t> data3;
    std::vector<float> data_to_write;
    
};
