#! /usr/bin/env python3

import zmq
import struct
import time

TEST_INIT = False
UI16_PACKET_SIZE = 4
UI32_PACKET_SIZE = 4 
F32_PACKET_SIZE = 4
REPETITIONS = 10

topic = "test_topic".encode('ascii')
print("Connection Info: Topic: {} | Envelope size: {} \n".format( topic.decode(encoding = 'ascii') , \
    struct.calcsize("H")*UI16_PACKET_SIZE + struct.calcsize("I")*UI32_PACKET_SIZE + struct.calcsize("f")*F32_PACKET_SIZE + len(topic) + 1 ) \
)

with zmq.Context() as context:
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://127.0.0.1:5555")

    i = 0

    try:
        for ii in range( 0, REPETITIONS):
            
            # print( "Writing {} Data Elements".format( UI16_PACKET_SIZE + UI32_PACKET_SIZE + F32_PACKET_SIZE ) )
            ui16buffer = []
            for j in range(0,UI16_PACKET_SIZE):
                ui16buffer.append( j + i )
            ui32buffer = []
            for j in range(0,UI32_PACKET_SIZE):
                ui32buffer.append( j + j + i )
            f32buffer = []
            for j in range(0,F32_PACKET_SIZE):
                f32buffer.append( j / (i+1) )

            struct_format = "<{:d}ss{:d}H{:d}I{:d}f".format( len(topic) , UI16_PACKET_SIZE, UI32_PACKET_SIZE, F32_PACKET_SIZE )
            data = struct.pack( struct_format, \
                                topic, " ".encode('ascii'), \
                                ui16buffer[0], ui16buffer[1], ui16buffer[2], ui16buffer[3], \
                                ui32buffer[0], ui32buffer[1], ui32buffer[2], ui32buffer[3], \
                                 f32buffer[0],  f32buffer[1],  f32buffer[2],  f32buffer[3] )
            socket.send(data)
            
            print( "Message {:d} Sent to Topic: {}\n".format( i, topic.decode(encoding = 'ascii') ) )
            time.sleep(1)
            i += 1
            
        socket.close()

    except KeyboardInterrupt:
        socket.close()
    except Exception as error:
        print("[ERROR]: {}".format(error))
        socket.close()