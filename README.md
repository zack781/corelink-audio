# corelink_audio

https://corelink.hsrn.nyu.edu/documentation/technical-overview

This project aims to develope a high fidelity low latency audio transport plugin (VST,AU) using the Corelink network framework and JUCE audio framework.

**Dependencies for Development:**
- Corelink c++ repo (https://cpp-docs.hsrn.nyu.edu/)
- oneTBB repo (https://github.com/oneapi-src/oneTBB)
- JUCE

For Mac:
- Xcode Command Line Tools
- Xcode IDE

Header:
  - ./corelink-client/cpp/include
  - ./external-dependencies/cpp/asio-cpp/v1.24.0
  - ./external-dependencies/cpp/rapidjson
  - ./external-dependencies/cpp
  - ./oneTBB/build/my_installed_onetbb/include/oneapi
  - ./oneTBB/build/my_installed_onetbb/include
  - ./oneTBB/build/my_installed_onetbb/include/tbb
    
 Library:
  - ./oneTBB/build//my_installed_onetbb/lib

Current source code can be found in ./testing_box under ./sender and ./receiver2

Existing support only for MAC. Window and Linux versions will be available in the near future.

**Setup Instructions:**
1. Install the necessary dependencies
2. Clone the above repositories
3. Open the following files: corelink-audio/Mac/sender/sender.jucer, corelink-audio/Mac/receiver2/receiver2.jucer, and JUCE/extras/AudioPluginHost/AudioPluginHost.jucer in Xcode using the JUCE Projucer
4. Build the sender.component and receiver.component plugin files using the AU scheme and Debug/Release configuration
5. Build the audiopluginhost standalone plugin using default configurations in Xcode
6. The sender.component and receiver2.component files should populate ~/Library/Audio/Plug-Ins/Components once the builds are successfull
7. The audiopluginhost executable should automatically be opened once the build is finished
8. Open the Available Plugins window under the option tab. Click options -> Click Scan for new or updated AudioUnit plugins

More docs to come.
