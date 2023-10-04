#! /usr/bin/env python3

import zmq
import struct
import time

UI16_PACKET_SIZE = 4
UI32_PACKET_SIZE = 4 
F32_PACKET_SIZE = 4

topic = "test_topic".encode('ascii')
print("Connection Info: Topic: {} | Envelope size: {} \n".format( topic.decode(encoding = 'ascii') , \
    struct.calcsize("H")*UI16_PACKET_SIZE + struct.calcsize("I")*UI32_PACKET_SIZE + struct.calcsize("f")*F32_PACKET_SIZE + len(topic) + 1 ) \
)

with zmq.Context() as context:
    socket = context.socket(zmq.SUB)
    socket.setsockopt(zmq.SUBSCRIBE, topic)

    # GET ONLY MOST RECENT MESSAGE
    socket.setsockopt(zmq.CONFLATE, 1)
    socket.connect("tcp://127.0.0.1:5555")

    i = 0

    try:
        while True:
            binary_topic, data_buffer = socket.recv().split(b' ', 1)

            decoded_btopic = binary_topic.decode(encoding = 'ascii')

            # print( "Reading {} Data Elements".format( UI16_PACKET_SIZE + UI32_PACKET_SIZE + F32_PACKET_SIZE ) )
            struct_format = "{:d}H{:d}I{:d}f".format( UI16_PACKET_SIZE, UI32_PACKET_SIZE, F32_PACKET_SIZE )

            data = struct.unpack(struct_format, data_buffer)
            print( "Message {:d} Recieved on Topic: {}".format( i, topic.decode(encoding = 'ascii') ) )
            print("\tdata: {}\n".format(data))

            i += 1
        socket.close()

    except KeyboardInterrupt:
        socket.close()
    except Exception as error:
        print("[ERROR]: {}".format(error))
        socket.close()