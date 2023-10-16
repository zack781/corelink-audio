#include<iostream>
#include<alsa/asoundlib.h>

unsigned int sample_rate = 44100;                       /* Sample rate */
unsigned int channels = 2;                             /* Channels */
snd_pcm_uframes_t buffer_size = 1024;                   /* Buffer Size */

int main(int argc, char* argv[])
{
    
    const char *card_name = "surround51";
    snd_pcm_t *pcm_handle;

    int error = snd_pcm_open(&pcm_handle, card_name, SND_PCM_STREAM_PLAYBACK, 0);
    if (error < 0)
    {
      std::cerr << "Error with opening PCM device = " << snd_strerror(error) << "\n";
      return 1;
    } else {
      std::cout << "pcm opened" << "\n";
    }

    while (pcm_handle == NULL);
    
    if (snd_pcm_avail(pcm_handle)) {
      std::cout << "pcm is available" << "\n";
    }
    
    snd_pcm_drop(pcm_handle);
    
    snd_pcm_drain(pcm_handle);

    snd_pcm_hw_params_t *params;
    
    // Allocate PCM hardware
    error = snd_pcm_hw_params_malloc(&params);
    if (error < 0){
      std::cout << "there's something wrong with parameter setting\n";
    }

    // Init PCM hardware parameters
    error = snd_pcm_hw_params_any(pcm_handle, params);
    if (error < 0){
      std::cout << "there's something wrong with parameter setting\n";
    }

    // Set PCM access type to interleaved
    error = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (error < 0){
      std::cout << "there's something wrong with parameter setting\n";
    }

    // Set PCM format to signed 16-bit little-endian
    error = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    if (error < 0){
      std::cout << "there's something wrong with parameter setting\n";
    }

    // Set PCM samplerate = 44100;
    error = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, 0);
    if (error < 0){
      std::cout << "there's something wrong with parameter setting\n";
    } 

    // Set Stereo Channels
    error = snd_pcm_hw_params_set_channels_near(pcm_handle, params, &channels);
    if (error < 0){
      std::cout << "there's something wrong with parameter setting\n";
    }

    // Set buffersize
    error = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size);
    if (error < 0){
      std::cout << "there's something wrong with parameter setting\n";
    } 

    error = snd_pcm_prepare(pcm_handle);
    if (error < 0)
    {
      if (error == -EPIPE) {
        std::cout << "Buffer underrun occurred" << "\n";
      } else if (error == -ESTRPIPE) {
        std::cout << "Stream is suspended" << "\n";
      } else {
        std::cerr << "Error with preparing PCM device = " << snd_strerror(error) << "\n";
        return 1;
      }
    }

    error = snd_pcm_start(pcm_handle);
    if (error < 0)
    {
      std::cerr << "Error with starting PCM device = " << snd_strerror(error) << "\n";
      return 1;
    }

    // hi-res timestamp
    snd_pcm_uframes_t frames;
    snd_htimestamp_t get_ticks;
    
    error = snd_pcm_htimestamp(pcm_handle, &frames, &get_ticks);
    if (error < 0)
    {
      std::cerr << "Error with timestamping = " << snd_strerror(error) << "\n";
      return 1;
    }

    std::cout << "Timestamping value = " << get_ticks.tv_sec << "." << get_ticks.tv_nsec << "\n";

    // Close pcm handle
    snd_pcm_close(pcm_handle);

    return 0;
}
