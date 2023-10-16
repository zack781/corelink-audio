/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Receiver2AudioProcessor::Receiver2AudioProcessor()
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
    write_ready = false;
    
    // set directory for .dat write
    // dir_path
    // dir_path = "~/Documents/myJUCE/temp_storage";
    
    myFile.open("/Users/node/Documents/local_audio_work/data_analysis/buffer_stats.dat", std::ios_base::out | std::ios_base::trunc);
    audio_file.open("/Users/node/Documents/local_audio_work/data_analysis/audio_file.dat", std::ios_base::out | std::ios_base::trunc);
    
    myFile.close();
    audio_file.close();

    std::thread receiver_thread(&Receiver2AudioProcessor::client_request, this, std::ref(client), std::ref(info), std::ref(control_channel_id));
    receiver_thread.detach();
}

void Receiver2AudioProcessor::client_request(corelink::client::corelink_classic_client& client, corelink::client::corelink_client_connection_info& info, corelink::core::network::channel_id_type& control_channel_id)
{
    while (!stream_init);

    cert_path = "/Users/zack/Documents/repos/corelink-server/config/ca-crt.pem";

    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);

    info.set_certificate_path(info_temp.client_certificate_path);
    info.set_hostname(corelink_host_id.toStdString());
//    info.set_hostname("corelink.hpc.nyu.edu");
    info.set_username(info_temp.username);
    info.set_password(info_temp.password);
    info.set_port_number(20010);


    if(!client.init_protocols())
    {
        throw corelink::commons::corelink_exception("Failed to init protocol!");
    }

    control_channel_id = client.add_control_channel(
                                                    info.protocol,
                                                    info.hostname,
                                                    info.port_number,
                                                    info.client_certificate_path,
                                                    std::bind(&Receiver2AudioProcessor::on_error, this, std::placeholders::_1, std::placeholders::_2),
                                                    std::bind(&Receiver2AudioProcessor::on_channel_init, this, std::placeholders::_1),
                                                    std::bind(&Receiver2AudioProcessor::on_channel_uninit, this, std::placeholders::_1));
    add_on_update_handler(control_channel_id, client);
    while(!done);
    done = false;

    client.request(control_channel_id,
                    corelink::client::corelink_functions::authenticate,
                    std::make_shared<corelink::client::request_response::requests::authenticate_client_request>(info.username, info.password),
                    [&](corelink::core::network::channel_id_type host_id,
                        in<std::string>,
                        in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>>)
                    {
                        DBG("creating receiver");
                        create_receiver(host_id, client);
                    }
    );
    DBG("client_request done");
}

Receiver2AudioProcessor::~Receiver2AudioProcessor()
{
    // tbbVector->clear();
    // delete tbbVector;
    DBG("Destructor HERE");
    

    loading = true;
    
    myFile.open("/Users/node/Documents/local_audio_work/data_analysis/buffer_stats.dat", std::ios_base::app);
    audio_file.open("/Users/node/Documents/local_audio_work/data_analysis/audio_file.dat", std::ios_base::app);
    
    for (int i = 0; i < stored_read_pos.size(); i++) {
        myFile << stored_read_pos[i] << " - " << stored_write_pos[i] << std::endl;
    }
    
    myFile.close();
    
    for (std::vector<uint8_t> buffer : *stored_audio_data) {
        for (int i = 0; i < buffer.size(); i+=4)
        {
            unsigned b1 = (unsigned)((buffer)[i]);
            unsigned b2 = (unsigned)((buffer)[i + 1]);
            unsigned b3 = (unsigned)((buffer)[i + 2]);
            unsigned b4 = (unsigned)((buffer)[i + 3]);
            b1 <<= 24;
            b2 <<= 24;
            b2 >>= 8;
            b3 <<= 24;
            b3 >>= 16;
            unsigned res = b1 + b2 + b3 + b4;
            float* p_ = (float*)(&res);
            audio_file << (*p_) << std::endl;
        }
    }
    DBG("Finish writing to files");
    
    audio_file.close();
}

void Receiver2AudioProcessor::set_stream_init()
{
    stream_init = true;
}

bool Receiver2AudioProcessor::get_stream_init()
{
    return stream_init;
}

void Receiver2AudioProcessor::set_audio_workspace(const juce::String& val)
{
    audio_workspace = val;
}

juce::String Receiver2AudioProcessor::get_audio_workspace()
{
    return audio_workspace;
}

void Receiver2AudioProcessor::set_audio_stream_type(const juce::String& val)
{
    audio_stream_type = val;
}

juce::String Receiver2AudioProcessor::get_audio_stream_type()
{
    return audio_stream_type;
}

void Receiver2AudioProcessor::set_host_id(const juce::String& val)
{
    corelink_host_id = val;
}

juce::String Receiver2AudioProcessor::get_host_id()
{
    return corelink_host_id;
}

void Receiver2AudioProcessor::set_buffering_size(const juce::String& val)
{
    std::string temp = val.toStdString();
//    int num = std::stoi(temp);
    buffering_size = std::stoi(temp);
}

int Receiver2AudioProcessor::get_buffering_size()
{
    return buffering_size;
}

void Receiver2AudioProcessor::set_dir_path(const juce::String& val)
{
    dir_path = val.toStdString();
}

std::string Receiver2AudioProcessor::get_dir_path()
{
    return dir_path;
}

void Receiver2AudioProcessor::on_error(corelink::core::network::channel_id_type host_id, in<std::string> err) {
    std::cerr << "Error in host id: " << host_id << "\n" << err << "\n";

    done = true;
}

void Receiver2AudioProcessor::on_channel_init(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " connected\n";
    DBG("Host id: " << std::to_string(host_id) << " connected");
    done = true;
}

void Receiver2AudioProcessor::on_channel_uninit(corelink::core::network::channel_id_type host_id)
{
    std::cout << "Host id: " << host_id << " disconnected\n";
    DBG("Host id: " << std::to_string(host_id) << " disconnected");
    done = true;
}

void Receiver2AudioProcessor::add_on_update_handler(ns_cl_core::network::channel_id_type control_channel_id,
                           out<corelink::client::corelink_classic_client> client)
{
  client.request(
      control_channel_id,
      corelink::client::corelink_functions::server_callback_on_update, nullptr,
      [&](corelink::core::network::channel_id_type host_id, in<std::string>,
        in<std::shared_ptr<corelink::client::request_response::responses::corelink_server_response_base>> response) {
            DBG("add_on_update_handler");
            create_receiver(host_id, client);
      });
}

void Receiver2AudioProcessor::create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
                     out<corelink::client::corelink_classic_client> client)
{
    auto request = std::make_shared<corelink::client::request_response::requests::modify_receiver_stream_request>(
                ns_cl_core::network::constants::protocols::udp);

    request->client_certificate_path = cert_path;
    request->alert = true;
    request->echo = true;

    request->workspace = audio_workspace.toStdString();
    request->stream_type = "['" + audio_stream_type.toStdString() + "']\n";
    
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
        if (!data.empty())
        {
            if (!loading)
            {
                /* if (write_ready)
                {
                    int w_pos = headers.get_int("counter_value") % 150;
                    (*tbbVector)[w_pos] = std::move(data);
                    write_pos = w_pos;
                } else {
                    if (headers.get_int("counter_value") % 150 == 0)
                    {
                        write_ready = true;
                    }
                } */
                int w_pos = headers.get_int("counter_value") % JITTER_BUFFER_SIZE;
                *(jitterBuffer + w_pos) = std::move(data);
                write_pos = w_pos;
            }
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
                    DBG("Create Receiver");
                    receiverInit = true;
                }
        );
}

/* void Receiver2AudioProcessor::write_latency(long long val)
{
write_pos = w_pos;
    std::string filename = "/Users/zack/Documents/turnup_audio_testing/latencies.txt";
    std::ofstream myFile;
    
    myFile.open(filename,  std::ios::out | std::ios::app);
    myFile << val << std::endl;
    myFile.close();
} */

template<class T>
void Receiver2AudioProcessor::swapMove(T& a, T& b)
{
    T tmp = { std::move(a) };
    a = std::move(b);
    b = std::move(tmp);
}

//==============================================================================
const juce::String Receiver2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Receiver2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Receiver2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Receiver2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Receiver2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Receiver2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Receiver2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Receiver2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Receiver2AudioProcessor::getProgramName (int index)
{
    return {};
}

void Receiver2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Receiver2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    // tbbVector->clear();
    
    /* for (int i=0; i<150; i++)
    {
        tbbVector->push_back(std::vector<uint8_t>(samplesPerBlock*8));
    } */
    
    for (int i=0; i<150; i++)
    {
        *(jitterBuffer + i) = std::move(std::vector<uint8_t>(samplesPerBlock*8));
    }
    
    audioBufferSize = samplesPerBlock;

    read_pos = 0;
    write_pos = 0;
    
    loading = false;
    ready = false;
}

void Receiver2AudioProcessor::releaseResources()
{
    // this method should after you exit the plugin process
    // jitterBuffer->clear();
    // delete jitterBuffer;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Receiver2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Receiver2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int counter = 0;

    if (write_pos > get_buffering_size() && stream_init) ready = true;

    if (!loading && receiverInit)
    {
        if (ready)
        {
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                auto* channelData = buffer.getWritePointer (channel);
                for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
                {
                    unsigned b1 = (unsigned)(*(jitterBuffer + read_pos))[counter];
                    unsigned b2 = (unsigned)(*(jitterBuffer + read_pos))[counter + 1];
                    unsigned b3 = (unsigned)(*(jitterBuffer + read_pos))[counter + 2];
                    unsigned b4 = (unsigned)(*(jitterBuffer + read_pos))[counter + 3];
                    b1 <<= 24;
                    b2 <<= 24;
                    b2 >>= 8;
                    b3 <<= 24;
                    b3 >>= 16;
                    unsigned res = b1 + b2 + b3 + b4;
                    float* p_ = (float*)(&res);
                    channelData[sample] = *p_;
                    counter += 4;
                }
            }
            // std::async(std::launch::async, &Receiver2AudioProcessor::write_to_dat, this, read_pos.load(), write_pos.load(), std::move((*tbbVector)[read_pos]));
            std::async(std::launch::async, &Receiver2AudioProcessor::write_to_dat, this, read_pos.load(), write_pos.load(), std::move(*(jitterBuffer + read_pos)));
            read_pos++;
            read_pos = read_pos % JITTER_BUFFER_SIZE;
        }
    }
}

void Receiver2AudioProcessor::write_to_dat(std::atomic<int> read_pos, std::atomic<int> write_pos, std::vector<uint8_t> data)
{
    stored_read_pos.push_back(read_pos);
    stored_write_pos.push_back(write_pos);
    
    if (buffer_ctr + 1 < stored_audio_data->size())
    {
        // (*stored_audio_data)[buffer_ctr] = std::move(data);
        swapMove((*stored_audio_data)[buffer_ctr], data);
        buffer_ctr++;
    }
}

//==============================================================================
bool Receiver2AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Receiver2AudioProcessor::createEditor()
{
    return new Receiver2AudioProcessorEditor (*this);
}

//==============================================================================
void Receiver2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Receiver2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Receiver2AudioProcessor();
}
