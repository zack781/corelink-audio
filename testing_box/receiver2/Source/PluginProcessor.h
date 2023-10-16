/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#define CORELINK_USE_CONCURRENT_QUEUE
#define CORELINK_ENABLE_STRING_UTIL_FUNCTIONS
#define CORELINK_USE_CONCURRENT_COUNTER

//#define CORELINK_ENABLE_ALL_FEATURES

//#define CONNECT_TO_LOCAL

#include <JuceHeader.h>
#include "corelink_all.hpp"
#include <iostream>
#include <future>
#include "tbb.h"


namespace ns_cl_client = corelink::client;
namespace ns_cl_core = corelink::core;
namespace ns_cl_req_resp = ns_cl_client::request_response;
template<typename t> using in = corelink::in<t>;
template<typename t> using out = corelink::out<t>;

//==============================================================================
/**
*/
class Receiver2AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Receiver2AudioProcessor();
    ~Receiver2AudioProcessor() override;

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
    
    void on_error(corelink::core::network::channel_id_type host_id, in<std::string> err);
    
    void on_channel_init(corelink::core::network::channel_id_type host_id);
    
    void on_channel_uninit(corelink::core::network::channel_id_type host_id);
    
    void add_on_update_handler(ns_cl_core::network::channel_id_type control_channel_id,
                               out<corelink::client::corelink_classic_client> client);
    
    void create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
                         out<corelink::client::corelink_classic_client> client);
            
    template<class T>
    void swapMove(T& a, T& b);
    
    void set_audio_workspace(const juce::String& val);
    
    void set_audio_stream_type(const juce::String& val);
    
    void set_stream_init();
    
    bool get_stream_init();
    
    void set_host_id(const juce::String& val);
    
    void set_buffering_size(const juce::String& val);
    
    juce::String get_audio_workspace();
    juce::String get_audio_stream_type();
    juce::String get_host_id();
    int get_buffering_size();
    
    void client_request(corelink::client::corelink_classic_client& client, corelink::client::corelink_client_connection_info& info, corelink::core::network::channel_id_type& control_channel_id);
    
    void watermarking(std::atomic<int>& read_pos, std::atomic<int>& write_pos, bool& ready);
    
    void write_to_dat(std::vector<float> data, int counter, std::string& dir_path);

    void set_dir_path(const juce::String& val);
    
    std::string get_dir_path();
    
    void write_latency(long long val);
    
//    void write_packet_label(int val);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Receiver2AudioProcessor)
    
    corelink::client::corelink_classic_client client;
    std::string cert_path;
    
    corelink::client::corelink_client_connection_info info = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);
    
    corelink::core::network::channel_id_type control_channel_id;
        
    std::atomic<bool> done;
    std::atomic<bool> loading;
    std::atomic<bool> receiverInit = false;
    std::atomic<int> bufferSize_ = 256;
    std::atomic<bool> write_ready;
    
    tbb::detail::d1::concurrent_vector<std::vector<uint8_t>>* tbbVector = new tbb::detail::d1::concurrent_vector<std::vector<uint8_t>>();
    
    //std::vector<std::vector<uint8_t>*>* tbbVector = new std::vector<std::vector<uint8_t>*>;
    
    std::atomic<int> read_pos;
    std::atomic<int> write_pos;
    
    corelink::utils::concurrent_counter<int> c_write_counter;
    corelink::utils::concurrent_counter<int> c_read_counter;
    
    std::vector<uint8_t> test;
    
    std::mutex bufferMutex;
    
    std::vector<uint8_t> to_float;
        
    std::chrono::steady_clock::time_point time_point;
    std::chrono::steady_clock::time_point time_point_;
    
    std::vector<float> arrival_intervals;
    std::vector<float> process_intervals;
    std::vector<float> elapsed_time;
//    std::queue<long long> arrival_queue;
    
    std::atomic<long long> read_time = 0;
    
    bool ready = false;
    
    int write_counter = 0;
    int read_counter = 0;

    
    // Stream INIT
    
    bool stream_init;
    
    juce::String audio_workspace = "ZackAudio";
    juce::String audio_stream_type = "audio";
    juce::String corelink_host_id = "127.0.0.0";
    int buffering_size = 25;
    
    std::vector<float> data_to_write;
    std::vector<int> counter_lst;
    
    std::string dir_path = "";
    
    corelink::core::network::channel_id_type hostId;
    
    
    
//    std::vector<float> myVector;
    // -------------------------------
    
};
