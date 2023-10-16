from scipy.io.wavfile import write
import numpy as np

from scipy.io import wavfile
import matplotlib.pyplot as plt
import scipy.io

samplerate = 48000
buffersize = 1024

right = []
left = []
count = 0
tot = []

f = open("/Users/zack/Documents/turnup_audio_testing/sender/8000.dat", "r")

for val in f:
    if count < buffersize:
        right.append(val.replace("\n", ""))
        count+=1
    else:
        left.append(val.replace("\n", ""))


for i in range(len(left)):
    tot.append([left[i], right[i]])

a = np.array(tot)
print(a.shape)

write("sender.wav", samplerate, a.astype(np.float32))

right = []
left = []
count = 0
tot = []

f = open("/Users/zack/Documents/turnup_audio_testing/receiver2/8000.dat", "r")

for val in f:
    if count < buffersize:
        right.append(val.replace("\n", ""))
        count+=1
    else:
        left.append(val.replace("\n", ""))


for i in range(len(left)):
    tot.append([left[i], right[i]])

a = np.array(tot)
print(a.shape)

write("receiver2.wav", samplerate, a.astype(np.float32))

wav_fname = 'sender.wav'
samplerate, data = wavfile.read(wav_fname)
length = data.shape[0] / samplerate

wav_fname_ = 'receiver2.wav'
samplerate_, data_ = wavfile.read(wav_fname)
length_ = data.shape[0] / samplerate

fig, axs = plt.subplots(2, 2)
time = np.linspace(0., length, data.shape[0])
time_ = np.linspace(0., length_, data_.shape[0])
axs[0, 0].set_title('Left channel')
axs[0, 0].plot(time, data[:, 0],'tab:blue')
axs[0, 1].set_title('Right channel')
axs[0, 1].plot(time, data[:, 1], 'tab:blue')
axs[1, 0].set_title('Left channel')
axs[1, 0].plot(time_, data_[:, 0],'tab:orange')
axs[1, 1].set_title('Right channel')
axs[1, 1].plot(time_, data_[:, 1], 'tab:orange')
plt.legend()
fig.text(0.5, 0.04, 'Time [s]', ha='center', va='center')
fig.text(0.06, 0.5, 'Amplitude', ha='center', va='center', rotation='vertical')
plt.show()


