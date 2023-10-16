from scipy.io.wavfile import write
import numpy as np

from scipy.io import wavfile
import matplotlib.pyplot as plt
import scipy.io

samplerate = 48000
buffersize = 512

right = []
left = []
count = 0
tot = []

f = open("audio_file.dat", "r")

channel = 0

for val in f:
    if channel == 0:
        left.append(val)
        if count % buffersize == 0:
            if channel == 1: channel = 0
            else: channel = 1
    else:
        right.append(0)
        if count % buffersize == 0:
            if channel == 1: channel = 0
            else: channel = 1
    count += 1
print('length left = ', len(left))
print('length right = ', len(right))

if len(left) < len(right):
    for i in range(len(left)):
        # print('i = ', i)
        tot.append([left[i], right[i]])
else:
    for i in range(len(right)):
        # print('i = ', i)
        tot.append([left[i], right[i]])

a = np.array(tot)
print(a.shape)

write("receiver2.wav", samplerate, a.astype(np.float32))

wav_fname = 'receiver2.wav'
samplerate, data = wavfile.read(wav_fname)
length = data.shape[0] / samplerate

time = np.linspace(0., length, data.shape[0])
fig, axs = plt.subplots(2)
fig.suptitle('Audio Data')
axs[0].plot(time, data[:, 0],'tab:blue')
axs[1].plot(time, data[:, 1], 'tab:blue')
plt.show()
