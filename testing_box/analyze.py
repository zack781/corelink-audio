import matplotlib.pyplot as plt

buffer_size = []
read_pos_vals = []
write_pos_vals = []

buffer_stats = open("buffer_stats.dat", "r")

data = buffer_stats.readlines()

for line in data:
    print(line)
    
    write_pos_lst = []
    read_pos_lst = []

    n = 0
    while line[n] != " " and n < len(line):
        read_pos_lst.append(line[n])
        n+=1
    read_pos = int("".join(read_pos_lst))
    read_pos_vals.append(read_pos)

    while line[n] == " " or line[n] == "-" and n < len(line):
        n+=1

    while line[n] != "\n" and  n < len(line):
        write_pos_lst.append(line[n])
        n+=1
    write_pos = int("".join(write_pos_lst))
    write_pos_vals.append(write_pos)

    if write_pos > read_pos:
        buffer_size.append(write_pos - read_pos)
    else:
        buffer_size.append(150 - (read_pos - write_pos))

n = []
for i in range(len(buffer_size)):
    n.append(i)

plt.plot(n, buffer_size, color='green', linewidth = 0, marker='o', markerfacecolor='blue', markersize=5)

# plt.plot(read_pos_vals, write_pos_vals, color='green', linewidth = 0,
#         marker='o', markerfacecolor='blue', markersize=5)
plt.title('T450(Tx) Mac(Rx) 44100 512')
plt.xlabel('Packet Number #')
plt.ylabel('Buffer Size')
plt.show()
