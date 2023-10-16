

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Corelink_audio_senderAudioProcessor::Corelink_audio_senderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    done = false;
    loading = true;
    
    stream_init = false;
    
//    cert_path = "/Users/zack/Documents/repos/corelink-server/config/ca-crt.pem";
//
//    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);
//
//    info.set_certificate_path(info_temp.client_certificate_path);
////    info.set_hostname("192.168.2.30");
////    info.set_hostname("54.89.170.133");
////    info.set_hostname("corelink.hpc.nyu.edu");
////    info.set_hostname("18.208.159.45");
//    info.set_hostname("127.0.0.1");
//    info.set_username(info_temp.username);
//    info.set_password(info_temp.password);
//    info.set_port_number(20010);
//
//    if(!client.init_protocols())
//    {
//        throw corelink::commons::corelink_exception("Failed to init protocol info!");
//    }
//
//    control_channel_id = client.add_control_channel(
//                                                    info.protocol,
//                                                    info.hostname,
//                                                    info.port_number,
//                                                    info.client_certificate_path,
//                                                    std::bind(&Corelink_audio_senderAudioProcessor::on_error, this, std::placeholders::_1, std::placeholders::_2),
//                                                    std::bind(&Corelink_audio_senderAudioProcessor::on_channel_init, this, std::placeholders::_1),
//                                                    std::bind(&Corelink_audio_senderAudioProcessor::on_channel_uninit, this, std::placeholders::_1));
//
//    while(!done);
//    done = false;
    
    //while(!stream_init);
    
//    DBG("stream init");

//    client.request(control_channel_id,
//                   corelink::client::corelink_functions::authenticate,
//                   std::make_shared<corelink::client::request_response::requests::authenticate_client_request>(info.username, info.password),
//                   [&](corelink::core::network::channel_id_type host_id,
//                       in<std::string>,
//                       in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>>)
//                   {
//                        create_sender(host_id, client);
//                    }
//    );
    std::thread sender_thread(&Corelink_audio_senderAudioProcessor::client_request, this, std::ref(client), std::ref(info), std::ref(control_channel_id));
    sender_thread.detach();
    
}

void Corelink_audio_senderAudioProcessor::client_request(corelink::client::corelink_classic_client& client, corelink::client::corelink_client_connection_info& info, corelink::core::network::channel_id_type& control_channel_id)
{
    while (!stream_init);
        
//    DBG("data size = " <<data.size());
        
    cert_path = "/Users/zack/Documents/repos/corelink-server/config/ca-crt.pem";
    
    // auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);
    
    info.set_certificate_path(cert_path);
//    info.set_hostname("192.168.2.30");
//    info.set_hostname("54.89.170.133");
//    info.set_hostname("corelink.hpc.nyu.edu");
//    info.set_hostname("18.208.159.45");
    
    info.set_endpoint(corelink_host_id.toStdString());
//    info.set_endpoint(corelink_host_id.toStdString());
    info.set_username("Testuser");
    info.set_password("Testpassword");
    info.set_port_number(20010);
    
    if(!client.init_protocols())
    {
        throw std::runtime_error(
                            "Failed to initialize protocol information. Please contact corelink development");

    }
    
    control_channel_id = client.add_control_channel(
                                                    info.protocol,
                                                    info.endpoint,
                                                    info.port_number,
                                                    info.client_certificate_path,
                                                    std::bind(&Corelink_audio_senderAudioProcessor::on_error, this, std::placeholders::_1, std::placeholders::_2),
                                                    std::bind(&Corelink_audio_senderAudioProcessor::on_channel_init, this, std::placeholders::_1),
                                                    std::bind(&Corelink_audio_senderAudioProcessor::on_channel_uninit, this, std::placeholders::_1));
    add_on_subscribe_handler(control_channel_id, client);
    while(!done);
    done = false;
//
//    client.request(control_channel_id,
//                   corelink::client::corelink_functions::authenticate,
//                   std::make_shared<corelink::client::request_response::requests::authenticate_client_request>(info.username, info.password),
//                   [&](corelink::core::network::channel_id_type host_id,
//                       in<std::string>,
//                       in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>>)
//                   {
////                        create_sender(host_id, client);
//                    }
//    );
    
    client_request_create_sender();
    
}

void Corelink_audio_senderAudioProcessor::client_request_create_sender()
{
    client.request(control_channel_id,
                   corelink::client::corelink_functions::authenticate,
                   std::make_shared<corelink::client::request_response::requests::authenticate_client_request>(info.username, info.password),
                   [&](corelink::core::network::channel_id_type host_id,
                       in<std::string>,
                       in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>>)
                   {
                        create_sender(host_id, client);
//                        create_receiver(host_id, client);
                    }
    );
}

void Corelink_audio_senderAudioProcessor::add_on_subscribe_handler(ns_cl_core::network::channel_id_type control_channel_id,
                           out<corelink::client::corelink_classic_client> client)
{
  client.request(
      control_channel_id,
      corelink::client::corelink_functions::server_callback_on_subscribed, nullptr,
      [&](corelink::core::network::channel_id_type host_id, in<std::string>,
          in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>> response) {
              DBG("subscribed to stream");
              packet_counter = 0;

      });
}

Corelink_audio_senderAudioProcessor::~Corelink_audio_senderAudioProcessor()
{
//    client.destroy();
//    if (client.init_protocols())
//    {
//        client.destroy();
//    }

}

void Corelink_audio_senderAudioProcessor::set_stream_init()
{
    stream_init = true;
}

bool Corelink_audio_senderAudioProcessor::get_stream_init()
{
    return stream_init;
}

void Corelink_audio_senderAudioProcessor::set_host_id(const juce::String& val)
{
    corelink_host_id = val;
}

void Corelink_audio_senderAudioProcessor::on_channel_init(corelink::core::network::channel_id_type host_id)
{
    done = true;
}

void Corelink_audio_senderAudioProcessor::on_channel_uninit(corelink::core::network::channel_id_type host_id)
{
    done = true;
}

void Corelink_audio_senderAudioProcessor::on_error(corelink::core::network::channel_id_type host_id, in<std::string> err)
{
    DBG("Error in host id: " << host_id);
    done = true;
}

juce::String Corelink_audio_senderAudioProcessor::get_audio_workspace()
{
    return audio_workspace;
}

juce::String Corelink_audio_senderAudioProcessor::get_audio_stream_type()
{
    return audio_stream_type;
}

juce::String Corelink_audio_senderAudioProcessor::get_host_id()
{
    return corelink_host_id;
}

void Corelink_audio_senderAudioProcessor::set_buffering_size(const juce::String& val)
{
    std::string temp = val.toStdString();
//    int num = std::stoi(temp);
    buffering_size = std::stoi(temp);;
    
}

int Corelink_audio_senderAudioProcessor::get_buffering_size()
{
    return buffering_size;
}

void Corelink_audio_senderAudioProcessor::set_dir_path(const juce::String& val)
{
    dir_path = val.toStdString();
}

std::string Corelink_audio_senderAudioProcessor::get_dir_path()
{
    return dir_path;
}

void Corelink_audio_senderAudioProcessor::create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
                     out<corelink::client::corelink_classic_client> client)
{
    // we are creating a TCP sender
    auto request = std::make_shared<corelink::client::request_response::requests::modify_receiver_stream_request>(
            ns_cl_core::network::constants::protocols::udp);

    // only websockets
    request->client_certificate_path = cert_path;
    request->alert = true;
    request->echo = true;
    //    request->workspace = "Zack";
    //    request->stream_type = "['audiotesting']\n";

    //DBG("['" + audio_stream_type.toStdString() + "']\n");

    request->workspace = "Holodeck";
    request->stream_type = "['loopback']\n";
    //    request->stream_type = "['audio']\n";

    request->
        on_init = [this](ns_cl_core::network::channel_id_type /*host_id*/)
    {
        std::cout << "Receiver init\n";
    };
    request->
        on_uninit = [](ns_cl_core::network::channel_id_type /*host_id*/)
    {
        std::cout << "Receiver uninit\n";
        DBG("Receiver uninit");
    };


    request->
        on_error = [](corelink::core::network::channel_id_type /*host_id*/, in<std::string> err)
    {
        std::cerr << "Error while receiving data on the data channel: " << err << "\n";
        /*DBG("Error while receiving data on the data channel: " << err);*/
    };
    request->
        on_receive = [this](ns_cl_core::network::channel_id_type host_id,
            in<corelink::client::constants::corelink_stream_id_type> stream_id,
            in<corelink::utils::json> headers,
            in<std::vector<uint8_t>> data)
    {
    //            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        if (!data.empty())
        {
           
        }
            

        };
        client.request(
                control_channel_id,
                ns_cl_client::corelink_functions::create_receiver,
                request,
                [&](corelink::core::network::channel_id_type /*host_id*/,
                    in<std::string> /*msg*/,
                    in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>> response)
                {
                    //std::cout << "Created Receiver\n";
                    DBG("Create Receiver");
    //                    std::this_thread::sleep_for(std::chrono::milliseconds());
                }
        );
}

void Corelink_audio_senderAudioProcessor::create_sender(ns_cl_core::network::channel_id_type control_channel_id, out<corelink::client::corelink_classic_client> client)
{
    // we are creating a TCP sender
    auto request =
        std::make_shared<corelink::client::request_response::requests::modify_sender_stream_request>(ns_cl_core::network::constants::protocols::udp);

    // only websockets
    request->client_certificate_path = cert_path;
    request->alert = true;
    request->echo = true;
//    request->workspace = "ZackAudio";
//    request->stream_type = "audiotesting";
    request->workspace = audio_workspace.toStdString();
    request->stream_type = audio_stream_type.toStdString();
    
    request->on_error = [](
        corelink::core::network::channel_id_type host_id,
        in<std::string> err)
    {
        DBG("Error while sending data on the data channel: " << err);
    };
    request->
        on_send = [](ns_cl_core::network::channel_id_type host_id, size_t bytes_sent)
    {
        //std::cout << "Sent out [" << bytes_sent << "] bytes on channel id" << host_id << "\n";
    };
    request->
        on_init = [&](ns_cl_core::network::channel_id_type host_id)
    {
        /*std::thread sender_thread(std::bind(&Client::send_timed_data, this, std::placeholders::_1,
        std::placeholders::_2), host_id, std::ref(client));
        sender_thread.detach();*/

    };
    client.
        request(
        control_channel_id,
        ns_cl_client::corelink_functions::create_sender,
        request,
        [&](corelink::core::network::channel_id_type host_id,
            in<std::string> /*msg*/,
            in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>> response)
            {
               
                hostId = host_id;
                loading = false;

//                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        );
}
    

//==============================================================================
const juce::String Corelink_audio_senderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Corelink_audio_senderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Corelink_audio_senderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Corelink_audio_senderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Corelink_audio_senderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Corelink_audio_senderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Corelink_audio_senderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Corelink_audio_senderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Corelink_audio_senderAudioProcessor::getProgramName (int index)
{
    return {};
}

void Corelink_audio_senderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Corelink_audio_senderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    bufferSize = samplesPerBlock;
    
    
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void Corelink_audio_senderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Corelink_audio_senderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Corelink_audio_senderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
//    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();
//
//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear (i, 0, buffer.getNumSamples());
    
    if (!loading && totalNumInputChannels >= 2)
    {
//        juce::AudioBuffer<float> newBuffer;
//        swapMove(newBuffer, buffer);
        
        std::async(std::launch::async, &Corelink_audio_senderAudioProcessor::sendData, this, std::move(buffer), buffer.getNumSamples(), std::ref(data));
        
//        std::thread send_thread(&Corelink_audio_senderAudioProcessor::sendData, this, std::move(buffer), bufferSize);
//        send_thread.detach();
    }
    
}

template<class T>
void Corelink_audio_senderAudioProcessor::swapMove(T &a, T &b)
{
    T tmp{ std::move(a) };
    a = std::move(b);
    b = std::move(tmp);
}
//
void Corelink_audio_senderAudioProcessor::sendData(juce::AudioBuffer<float> buffer, int bufferSize, std::vector<uint8_t>& data)
{
    if (!loading)
    {
        corelink::utils::json meta;
//        corelink::utils::json meta2;
//        corelink::utils::json meta3;
        //DBG("sending data");
//        std::vector<uint8_t> data(buffer.getNumSamples() * 8);
        

//        meta.clear();
        
//        DBG("Headers: " << meta.to_string(true) << '\n');
//        DBG("sender counter = " << counter);
//        meta.append("bufferSize", buffer.getNumSamples());
        
//        std::chrono::time_point<std::chrono::system_clock> now =
//            std::chrono::system_clock::now();
//        auto duration = now.time_since_epoch();
//        long long millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
//
//
        meta.append("counter_value", packet_counter);
//        meta.append("start_time", millis);
//
//        meta2.append("bufferSize", buffer.getNumSamples());
//        meta2.append("start_time", millis);
        
//        meta3.append("bufferSize", buffer.getNumSamples());
//        meta3.append("counter_value", counter);

        auto totalNumInputChannels = getTotalNumInputChannels();
        
//        DBG("buffer.getNumSamples() = " << buffer.getNumSamples());
        
       

        for (int channel = 0; channel < totalNumInputChannels; channel++)
        {
//            int buffer_index = buffer.getNumSamples() * 4 * channel;
//            auto* inBuffer = buffer->getReadPointer(channel);
            auto* inBuffer = buffer.getReadPointer(channel);
            //auto* channelData = buffer->getWritePointer(channel);
//            for (int sample = 0; sample < buffer->getNumSamples(); sample++)
            for (int sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                float val = inBuffer[sample];
                int* p = (int*)(&val);
                unsigned byte1 = (*p >> 24);
                unsigned byte2 = (*p >> 16);
                unsigned byte3 = (*p >> 8);
                unsigned byte4 = *p;
                data.push_back(byte1);
                data.push_back(byte2);
                data.push_back(byte3);
                data.push_back(byte4);
                
//                data2.push_back(byte1);
//                data2.push_back(byte2);
//                data2.push_back(byte3);
//                data2.push_back(byte4);
////
//                data3.push_back(byte1);
//                data3.push_back(byte2);
//                data3.push_back(byte3);
//                data3.push_back(byte4);
//                data.at(buffer_index + (sample * 4) + 0) = byte1;
//                data.at(buffer_index + (sample * 4) + 1) = byte2;
//                data.at(buffer_index + (sample * 4) + 2) = byte3;
//                data.at(buffer_index + (sample * 4) + 3) = byte4;

                
//                data_to_write.push_back(val);
            }
        }
        
//        DBG("data.size() = " << data.size());
        if (data.size() > 0 )
        {
            client.send_data(hostId, std::move(data), std::move(meta));
//            client.send_data(hostId, std::move(data2), std::move(meta2));
//            client.send_data(hostId, std::move(data3), std::move(meta3));
        }
                
        // Write to disk
        
//        if (dir_path != "")
//        {
//            std::thread write_to_dat_thread(&Corelink_audio_senderAudioProcessor::write_to_dat, this, std::move(data_to_write), packet_counter, std::ref(dir_path));
//            write_to_dat_thread.detach();
////            counter++;
//
//        }
        packet_counter++;
        
        
        // ------------
    }
}
    

void Corelink_audio_senderAudioProcessor::write_to_dat(std::vector<float> data, int counter, std::string& dir_path)
{
    std::string filename = dir_path + "/" + std::to_string(counter) + ".dat";
//    DBG(filename);
    std::ofstream myFile(filename);
            
//        const char* data_  = reinterpret_cast<char*>(data.data());
        
    for (int i = 0; i < data.size(); i++)
    {
//        DBG(data[i]);
//        myFile << data[i];
        myFile << data.at(i) << std::endl;
    }

//        int num[5] = {4, 3, 6, 7, 12};
//        for (int i=0; i<5; ++i)
//              myFile << num[i] << std::endl;
    myFile.close();
}

void Corelink_audio_senderAudioProcessor::set_audio_workspace(const juce::String& val)
{
    audio_workspace = val;
}

void Corelink_audio_senderAudioProcessor::set_audio_stream_type(const juce::String& val)
{
    audio_stream_type = val;
}

//==============================================================================
bool Corelink_audio_senderAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Corelink_audio_senderAudioProcessor::createEditor()
{
//    juce::AudioProcessorEditor* editor;
    return new Corelink_audio_senderAudioProcessorEditor (*this);
//    return editor;
}

//==============================================================================
void Corelink_audio_senderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Corelink_audio_senderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//=========================X=====================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Corelink_audio_senderAudioProcessor();
}
