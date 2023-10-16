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
                       ), c_write_counter(149, 0), c_read_counter(149, 0)
#endif
{
    done = false;
    loading = true;
    stream_init = false;
    write_ready = false;

    std::thread receiver_thread(&Receiver2AudioProcessor::client_request, this, std::ref(client), std::ref(info), std::ref(control_channel_id));
    receiver_thread.detach();
//
////    time_point = std::chrono::steady_clock::now();
////    time_point_ = std::chrono::steady_clock::now();
////
    
//    std::thread watermarking_thread(&Receiver2AudioProcessor::watermarking, this, std::ref(read_pos), std::ref(write_pos), std::ref(ready));
//    watermarking_thread.detach();
}
//

void Receiver2AudioProcessor::watermarking(std::atomic<int>& read_pos, std::atomic<int>& write_pos, bool& ready)
{
    while(true)
    {
//        DBG("read_pos = " << c_read_counter());
        
        if (ready)
        {
//            if (c_read_counter() < c_write_counter())
//            {
//                if (c_write_counter() - c_read_counter() < 5)
//                {
//                    DBG("c_write_counter() - read_pos = " << c_write_counter() - c_read_counter());
//                }
//                else if (write_pos - read_pos > 45)
//                {
//                    DBG("c_write_counter() - read_pos = " << c_write_counter() - c_read_counter());
//                }
//            }
//
//            else if (c_write_counter() < c_read_counter())
//            {
//                if (c_read_counter() - c_write_counter() < 5)
//                {
//                    DBG("read_pos - c_write_counter() = " << c_read_counter() - c_write_counter() );
//                }
//                else if (read_pos- write_pos > 45)
//                {
//                    DBG("read_pos - c_write_counter() = " << c_read_counter() - c_write_counter());
//                }
//            }
            if (read_pos < write_pos)
            {
                if (write_pos - read_pos < 5)
                {
                    std::ofstream myFile;
                    std::string filename = "Users/zack/Documents/turnup_audio_testing/dropouts.txt";
                    myFile.open(filename,  std::ios::out | std::ios::app);
                    myFile << "write_pos - read_pos < 5" << std::endl;
                    myFile.close();
                }
                else if (write_pos - read_pos > 270)
                {
                    std::ofstream myFile;
                    std::string filename = "Users/zack/Documents/turnup_audio_testing/dropouts.txt";
                    myFile.open(filename,  std::ios::out | std::ios::app);
                    myFile << "write_pos - read_pos > 270" << std::endl;
                    myFile.close();
                }
            }
            
            else if (write_pos < read_pos)
            {
                if (read_pos - write_pos < 5)
                {
                    std::ofstream myFile;
                    std::string filename = "Users/zack/Documents/turnup_audio_testing/dropouts.txt";
                    myFile.open(filename,  std::ios::out | std::ios::app);
                    myFile << "read_pos - write_pos < 5" << std::endl;
                    myFile.close();
                }else if (read_pos - write_pos > 270)
                {
                    std::ofstream myFile;
                    std::string filename = "Users/zack/Documents/turnup_audio_testing/dropouts.txt";
                    myFile.open(filename,  std::ios::out | std::ios::app);
                    myFile <<  "read_pos - write_pos > 270" << std::endl;
                    myFile.close();
                }
        
            }
        }
    }
}

void Receiver2AudioProcessor::client_request(corelink::client::corelink_classic_client& client, corelink::client::corelink_client_connection_info& info, corelink::core::network::channel_id_type& control_channel_id)
{
    while (!stream_init);

    // Sizing Double Buffer

    // --------------------

    cert_path = "/Users/zack/Documents/repos/corelink-server/config/ca-crt.pem";

    auto info_temp = ns_cl_client::corelink_client_connection_info(corelink::core::network::constants::protocols::tcp).set_certificate_path(cert_path);

    info.set_certificate_path(info_temp.client_certificate_path);
//    info.set_hostname("192.168.2.30");
//    info.set_hostname("44.200.47.88");
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
//                        create_sender(host_id, client);
                    }
    );
    DBG("client_request done");
}

Receiver2AudioProcessor::~Receiver2AudioProcessor()
{
    
//    if (process_intervals.size() > arrival_intervals.size())
//    {
//        for (int i=0; i<arrival_intervals.size(); i++)
//        {
//            DBG("val_process = " << process_intervals[i]);
//            DBG("val_packet = " << arrival_intervals[i]);
//        }
//    }else{
//        for (int i=0; i<arrival_intervals.size(); i++)
//        {
//            DBG("val_process = " << process_intervals[i]);
//            DBG("val_packet = " << arrival_intervals[i]);
//        }
//    }
//
//

    tbbVector->clear();
    
    delete tbbVector;
    
    
    //client.destroy();
    
//    for (float val : elapsed_time)
//    {
//        DBG("val = " << val);
//    }
    
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
                
//        if (response) {
//          auto server_response = std::static_pointer_cast<
//              corelink::client::request_response::responses::
//                  server_cb_on_update_response>(response);
//          auto receiver_id = server_response->receiver_id;
//          std::cout << receiver_id << std::endl;
//          std::cout << (std::string)server_response->meta << std::endl;
//          std::cout << (std::string)server_response->type << std::endl;
//          std::cout << (std::string)server_response->user << std::endl;
//          std::cout << (std::string)server_response->message << std::endl;
//          std::cout << (std::string)server_response->function << std::endl;
//        }
      });
}

void Receiver2AudioProcessor::create_receiver(ns_cl_core::network::channel_id_type control_channel_id,
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
    
    request->workspace = audio_workspace.toStdString();
    request->stream_type = "['" + audio_stream_type.toStdString() + "']\n";
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
////
//        DBG("write_pos = " << write_pos);
//        DBG("read_pos = " << read_pos);
        
        if (!data.empty())
        {
            if (!loading)
            {
                
                if (write_ready)
                {
                    
                    //                DBG(data.size());
                    //                DBG("val = " << data[0]);
                    //                std::chrono::steady_clock::time_point packet_arrived = std::chrono::steady_clock::now();
                    //                    bufferMutex.lock();
                    
                    int w_pos = headers.get_int("counter_value") % 150;
//                    DBG("w_pos = " << w_pos);
                    (*tbbVector)[w_pos] = std::move(data);
                    
//                                    std::chrono::time_point<std::chrono::system_clock> now =
//                                        std::chrono::system_clock::now();
//                                    auto duration = now.time_since_epoch();
//                                    long long millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
//
//                                    std::async(std::launch::async, &Receiver2AudioProcessor::write_latency, this, millis -  headers.get_int64("start_time"));
                    //                        (*tbbVector).at(c_write_counter()) = std::move(data);
                    
                    //                    DBG("vector size = " << (*tbbVector).size());
                    //                    DBG("c_write_counter = " << c_write_counter());
                    
                    //                    (*tbbVector)[write_pos] = std::move(data);
                    //
                    //                    DBG("counter_val = " <<  headers.get_int("counter_value"));
                    //                    DBG("w_pos = " << w_pos);
                    //write_pos = headers.get_int("counter_value") % 50;
                    
                    //                    (*tbbVector).at(w_pos) = std::move(data);
                    //                    bufferMutex.unlock();
                    
                    //                    counter_lst.at(w_pos) = headers.get_int("counter_value");
//                    counter_lst[write_pos] = headers.get_int("counter_value");
                    
                    //DBG(counter_lst[write_pos]);
                    //                    c_write_counter++;
                    write_pos++;
                    write_pos = write_pos % tbbVector->size();
                    
                }else {
//                    DBG("write_ready is false");
//                    DBG(headers.get_int("counter_value") % 150);
                    if (headers.get_int("counter_value") % 150 == 0)
                    {

                        write_ready = true;
                    }
                }
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
                    //std::cout << "Created Receiver\n";
                    DBG("Create Receiver");
//                    std::this_thread::sleep_for(std::chrono::milliseconds());
                    
                    receiverInit = true;

                }
        );
}

void Receiver2AudioProcessor::write_latency(long long val)
{
    std::string filename = "/Users/zack/Documents/turnup_audio_testing/latencies.txt";
    std::ofstream myFile;
    
    myFile.open(filename,  std::ios::out | std::ios::app);
            
//        const char* data_  = reinterpret_cast<char*>(data.data());
    
    myFile << val << std::endl;

//        int num[5] = {4, 3, 6, 7, 12};
//        for (int i=0; i<5; ++i)
//              myFile << num[i] << std::endl;
    myFile.close();
}

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
    
    tbbVector->clear();
    
    for (int i=0; i<150; i++)
    {
        tbbVector->push_back(std::vector<uint8_t>(samplesPerBlock*8));
    }
    
    counter_lst.clear();
    
    for (int i=0; i<150; i++)
    {
        counter_lst.push_back(0);
    }
    
    bufferSize_ = samplesPerBlock;
    
//    to_float.clear();
//    for (int i = 0; i < 4; i++)
//    {
//        to_float.push_back(0);
//    }

    read_pos = 0;
    write_pos = 0;
    
    loading = false;
    ready = false;
    
//    std::thread receiver_thread(&Receiver2AudioProcessor::client_request, this, std::ref(client), std::ref(info), std::ref(control_channel_id));
//    receiver_thread.detach();

    //std::this_thread::sleep_for(std::chrono::microseconds(1000));
//    myVector.resize (samplesPerBlock);
//    float* dataPtrs[1] = {myVector.data()};

   
    
}

void Receiver2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
//    DBG("start");
//    if (process_intervals.size() > arrival_intervals.size())
//    {
//        for (int i=0; i<arrival_intervals.size(); i++)
//        {`
//            DBG("val_process = " << process_intervals[i]);
//            DBG("val_packet = " << arrival_intervals[i]);
//        }
//    }else{
//        for (int i=0; i<arrival_intervals.size(); i++)
//        {
//            DBG("val_process = " << process_intervals[i]);
//            DBG("val_packet = " << arrival_intervals[i]);
//        }
//    }
//
//    DBG("end");
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
//    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    
//    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    int counter = 0;
    
//    DBG(c_write_counter());
    
//    if (!ready && receiverInit)
//    {
//        if (write_pos > get_buffering_size())
//        {
//            ready = true;
//        }
//    }
    
//    if (c_write_counter() > get_buffering_size() && receiverInit)
//    {
//        ready = true;
//    }
    
    if (write_pos > get_buffering_size() && stream_init)
    {
        ready = true;
    }

    
//    DBG("read_pos = " << read_pos);
//    DBG("write_pos = " << write_pos);
//    DBG("c_write_counter = " << c_write_counter());
    
    if (!loading && receiverInit)
    {
        
            if (ready)
            {
//                DBG("tbb size = " << tbbVector->size());
//                DBG("tbb[0] = " << tbbVector->at(0).size());
//                bufferMutex.lock();
//                if (((*tbbVector)[read_pos]) !=  )
//                {
                    //DBG("(*((*tbbVector)[read_pos])).size() = " << (*((*tbbVector)[read_pos])).size());
                    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
                    {
                        auto* channelData = buffer.getWritePointer (channel);
                        
                        
                        for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
                        {
                            //                    std::vector<uint8_t> to_float;
                            //                    for (int i = 0; i < 4; i++)
                            //                    {
                            //
                            //                        to_float.push_back((*((*tbbVector)[read_pos]))[counter + i]);
                            //
                            //                    }
                            
                            unsigned b1 = (unsigned)(((*tbbVector)[read_pos])[counter]);
                            unsigned b2 = (unsigned)(((*tbbVector)[read_pos])[counter + 1]);
                            unsigned b3 = (unsigned)(((*tbbVector)[read_pos])[counter + 2]);
                            unsigned b4 = (unsigned)(((*tbbVector)[read_pos])[counter + 3]);

//                            unsigned b1 = (unsigned)(((*tbbVector).at(read_pos))[counter]);
//                            unsigned b2 = (unsigned)(((*tbbVector).at(read_pos))[counter + 1]);
//                            unsigned b3 = (unsigned)(((*tbbVector).at(read_pos))[counter + 2]);
//                            unsigned b4 = (unsigned)(((*tbbVector).at(read_pos))[counter + 3]);
                            
//                             unsigned b1 = (unsigned)(((*tbbVector).at(c_read_counter()))[counter]);
//                             unsigned b2 = (unsigned)(((*tbbVector).at(c_read_counter()))[counter + 1]);
//                             unsigned b3 = (unsigned)(((*tbbVector).at(c_read_counter()))[counter + 2]);
//                             unsigned b4 = (unsigned)(((*tbbVector).at(c_read_counter()))[counter + 3]);

//                            unsigned b1 = (unsigned)(((*tbbVector)[c_read_counter()])[counter]);
//                            unsigned b2 = (unsigned)(((*tbbVector)[c_read_counter()])[counter + 1]);
//                            unsigned b3 = (unsigned)(((*tbbVector)[c_read_counter()])[counter + 2]);
//                            unsigned b4 = (unsigned)(((*tbbVector)[c_read_counter()])[counter + 3]);
//
                            b1 <<= 24;
                            b2 <<= 24;
                            b2 >>= 8;
                            b3 <<= 24;
                            b3 >>= 16;
                            
                            unsigned res = b1 + b2 + b3 + b4;
                            
                            float* p_ = (float*)(&res);
                            
                            //auto res = corelink::utils::system::from_bytes<float>(to_float);
//                            DBG("res = " << *p_);
                            channelData[sample] = *p_;
                            
//                            data_to_write.push_back(*p_);
                            
                            counter += 4;
                        }
                    }
//                    delete (*tbbVector)[read_pos];
//                if (dir_path != "")
//                {
//                    std::thread write_to_dat_thread(&Receiver2AudioProcessor::write_to_dat, this, std::move(data_to_write), counter_lst[c_read_counter()], std::ref(dir_path));
//                    write_to_dat_thread.detach();
//                }
////                }
                
//                    c_read_counter++;
                read_pos += 1;
                read_pos = read_pos % tbbVector->size();
                
//                std::async(std::launch::async, &Receiver2AudioProcessor::send_data, this);

            }
    }
    
}

void Receiver2AudioProcessor::write_to_dat(std::vector<float> data, int counter, std::string& dir_path)
{
    std::string filename = dir_path + "/" + std::to_string(counter) + ".dat";
    std::ofstream myFile(filename);
            
//        const char* data_  = reinterpret_cast<char*>(data.data());
    
    for (int i = 0; i < data.size(); i++)
    {
        myFile << data[i] << std::endl;
    }

//        int num[5] = {4, 3, 6, 7, 12};
//        for (int i=0; i<5; ++i)
//              myFile << num[i] << std::endl;
    myFile.close();
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
