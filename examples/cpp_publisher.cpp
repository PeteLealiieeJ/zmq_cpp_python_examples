#include <iostream>
#include <zmq.hpp>
#include <string>
#include <stdio.h>
#include <stdlib.h> 
// SLEEP FUNCTION
#ifndef _WIN32
    #include <unistd.h>
#else
    #include <windows.h>
    #define sleep(n)	Sleep(n)
#endif

namespace utlift_zmq_cnnct{
    class publisher_zmq_cpp{
        private:
            // ZMQ CONTEXT AND SOCKET
            zmq::context_t ctx;
            zmq::socket_t skt;
            const char* tcp_address_string;

            // MESSAGING FORMATS
            const char* TOPIC; 
            const size_t topic_size;
            const size_t data_size;
            const size_t envelope_size;

        public:
            publisher_zmq_cpp( const char* addressi, const char* TOPICi, size_t datasi ) : 
                ctx(1),
                skt( ctx, ZMQ_PUB  ) ,
                tcp_address_string(addressi), 
                TOPIC(TOPICi), 
                topic_size(std::strlen(TOPIC)), 
                data_size(data_size), 
                envelope_size( std::strlen(TOPIC) + 1 + datasi)
            {
                printf("Connection Info: Topic: %s | Envelope size: %zu\n\n", this -> TOPIC, this -> envelope_size );
                this -> skt.bind( this -> tcp_address_string);
            };

            ~publisher_zmq_cpp(){
                this -> skt.close();
                this -> ctx.shutdown();
            }

            bool publish_sectioned_message( uint16_t ui16_buffer[], const unsigned int UI16_PACKET_SIZE, 
                                            uint32_t ui32_buffer[], const unsigned int UI32_PACKET_SIZE,
                                            _Float32 f32_buffer[], const unsigned int F32_PACKET_SIZE ){
                
                // printf("Writing %u Data Elements\n\n", UI16_PACKET_SIZE + UI32_PACKET_SIZE + F32_PACKET_SIZE );
                zmq::message_t envelope( this -> envelope_size );

                // TOPIC
                memcpy( envelope.data(), this -> TOPIC, this -> topic_size );
                // PADDING
                memcpy((void*)((char*)envelope.data() + this -> topic_size), " ", 1);
                // DATA 
                memcpy((void*)((char*)envelope.data() + this -> topic_size + 1 ), 
                    ui16_buffer, UI16_PACKET_SIZE * sizeof(uint16_t));
                memcpy((void*)((char*)envelope.data() + this -> topic_size + 1 + (UI16_PACKET_SIZE * sizeof(uint16_t)) ), 
                    ui32_buffer, UI32_PACKET_SIZE * sizeof(uint32_t));
                memcpy((void*)((char*)envelope.data() + this -> topic_size + 1 + (UI16_PACKET_SIZE * sizeof(uint16_t)) + (UI32_PACKET_SIZE * sizeof(uint32_t)) ), 
                    f32_buffer, F32_PACKET_SIZE * sizeof(_Float32));

                bool send_pass = this -> skt.send(envelope);
                return send_pass;
            }

    };
}



int main(){

    const char* tcp_address = "tcp://*:5555";
    const char* tcp_topic = "test_topic";
    const unsigned int UI16_PACKET_SIZE = 4;
    const unsigned int UI32_PACKET_SIZE = 4;
    const unsigned int F32_PACKET_SIZE = 4;

    size_t data_size = ( UI16_PACKET_SIZE * sizeof(uint16_t) ) + ( UI32_PACKET_SIZE * sizeof(uint32_t) ) + ( F32_PACKET_SIZE * sizeof(_Float32) );

    const unsigned int REPETITIONS = 10;

    utlift_zmq_cnnct::publisher_zmq_cpp test_zmq_publisher( tcp_address, tcp_topic, data_size );

    for (unsigned int i = 0; i < REPETITIONS; i++){

        // SETUP RESPECTIVE BUFFERS 
        uint16_t u16buffer[UI16_PACKET_SIZE];
        for (unsigned int j = 0; j < UI16_PACKET_SIZE; j++){
            u16buffer[j] = (uint16_t) j + i; 
        }
        uint32_t u32buffer[UI32_PACKET_SIZE];
        for (unsigned int j = 0; j < UI32_PACKET_SIZE; j++){
            u32buffer[j] = (uint32_t) j + j + i; 
        }
        _Float32 f32buffer[F32_PACKET_SIZE];
        for (unsigned int j = 0; j < F32_PACKET_SIZE; j++){
            f32buffer[j] = (_Float32) j / ( i + 1 ); 
        }


        // SEND MESSAGE TO TOPIC
        bool send_pass = test_zmq_publisher.publish_sectioned_message(  u16buffer, UI16_PACKET_SIZE,
                                                                        u32buffer, UI32_PACKET_SIZE, 
                                                                        f32buffer,  F32_PACKET_SIZE );
        if( !send_pass ){
            printf("[WARNING]: Message %u Was NOT Sent to Topic.\n\n", i);
            break;
        }

        printf("Message %u Sent to Topic: %s\n\n", i, tcp_topic);
        sleep(1);
    }

    return EXIT_SUCCESS;
}