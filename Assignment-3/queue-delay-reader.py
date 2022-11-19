import matplotlib.pyplot as plt
import numpy as np

with open('tcp-example.tr') as f:
    lines = f.readlines()

    # with open('enqueue.txt', 'w') as file:
    #     for l in lines:
    #         file.write(l)
    #         file.write('\n')

    enqueue_time = {}
    dequeue_time = []
    delay_time = []

    for packet in lines:
        if packet.split()[0] == "+":
            enqueue_time[int(packet.split()[18])] = float(packet.split()[1])

        elif packet.split()[0] == "-":
            dequeue_time.append(float(packet.split()[1]))
            delay_time.append(
                float(packet.split()[1]) - enqueue_time[int(packet.split()[18])])

    plt.scatter(dequeue_time, delay_time, s=3, color='r')
    plt.xlabel('Dequeuing Time (in sec)')
    plt.ylabel('Queuing Delay = deque_time - enque_time (in sec)')
    plt.savefig('tcp-example.png')
