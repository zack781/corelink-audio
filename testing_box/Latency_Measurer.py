import sys
sys.path.append("/Users/zack/Documents/repos/audacity/scripts/piped-work")
import pipeclient
client = pipeclient.PipeClient()

import pyaudio
import wave
import threading

CHUNK = 1024
FORMAT = pyaudio.paInt16
CHANNELS = 2
RATE = 44100
RECORD_SECONDS = 180
##WAVE_OUTPUT_FILENAME = "/Users/zack/Documents/compSci/Corelink/voice.wav"

p = pyaudio.PyAudio()
for i in range(p.get_device_count()):
    print(p.get_device_info_by_index(i))

def record_vb_A():
    WAVE_OUTPUT_FILENAME = "/Users/zack/Documents/compSci/Corelink/VB_A.wav"

    audio = pyaudio.PyAudio()

    stream = audio.open(format=FORMAT, channels=CHANNELS,
                rate=RATE, input=True,input_device_index = 5,
                frames_per_buffer=CHUNK)

    Recordframes = []
     
    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        data = stream.read(CHUNK)
        Recordframes.append(data)
##    print ("recording stopped")
     
    stream.stop_stream()
    stream.close()
    audio.terminate()
     
    waveFile = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
    waveFile.setnchannels(CHANNELS)
    waveFile.setsampwidth(audio.get_sample_size(FORMAT))
    waveFile.setframerate(RATE)
    waveFile.writeframes(b''.join(Recordframes))
    waveFile.close()

def record_vb_B():
    WAVE_OUTPUT_FILENAME = "/Users/zack/Documents/compSci/Corelink/VB_B.wav"

    audio = pyaudio.PyAudio()
    
    stream = audio.open(format=FORMAT, channels=CHANNELS,
                rate=RATE, input=True,input_device_index = 6,
                frames_per_buffer=CHUNK)

    Recordframes = []
     
    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        data = stream.read(CHUNK)
        Recordframes.append(data)
##    print ("recording stopped")
     
    stream.stop_stream()
    stream.close()
    audio.terminate()
     
    waveFile = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
    waveFile.setnchannels(CHANNELS)
    waveFile.setsampwidth(audio.get_sample_size(FORMAT))
    waveFile.setframerate(RATE)
    waveFile.writeframes(b''.join(Recordframes))
    waveFile.close()


if __name__ == '__main__':
##    Thread(target=recorder, args=('VB-Cable A',)).start()
##    time.sleep(6)
##    Thread(target=recorder, args=('VB-Cable B',)).start()
    threading.Thread(target=record_vb_B).start()
    threading.Thread(target=record_vb_A).start()
    

    






