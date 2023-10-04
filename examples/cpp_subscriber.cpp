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

    struct recvd_data{
        std::vector<int16_t> ui16_data;
        std::vector<int32_t> ui32_data;
        std::vector<_Float32> f32_data;
        recvd_data( const unsigned int UI16_PACKET_SIZE, 
                    const unsigned int UI32_PACKET_SIZE,
                    const unsigned int F32_PACKET_SIZE ): 
                    ui16_data( std::vector<int16_t>( UI16_PACKET_SIZE, 0 ) ),
                    ui32_data( std::vector<int32_t>( UI32_PACKET_SIZE, 0 ) ),
                    f32_data( std::vector<_Float32>( F32_PACKET_SIZE, 0.) ){};
    };

    class subscriber_zmq_cpp{
        private:
            // ZMQ CONTEXT AND SOCKET
            zmq::context_t ctx;
            zmq::socket_t skt;
            const char* tcp_address_string;

            // MESSAGING FORMATS
            const char* TOPIC; 
            const int conflate = 1;
            const size_t topic_size;
            const size_t data_size;
            const size_t envelope_size;

        public:
            subscriber_zmq_cpp( const char* addressi, const char* TOPICi, size_t datasi ) : 
                ctx(1),
                skt( ctx, ZMQ_SUB  ) ,
                tcp_address_string(addressi), 
                TOPIC(TOPICi), 
                topic_size(std::strlen(TOPIC)), 
                data_size(data_size), 
                envelope_size( std::strlen(TOPIC) + 1 + datasi)
            {
                printf("Connection Info: Topic: %s | Envelope size: %zu\n\n", this -> TOPIC, this -> envelope_size );
                this -> skt.setsockopt( ZMQ_SUBSCRIBE, "", 0);
                this -> skt.setsockopt( ZMQ_CONFLATE, &(this->conflate), sizeof((this->conflate)));
                this -> skt.connect( this -> tcp_address_string);
            };

            ~subscriber_zmq_cpp(){
                this -> skt.close();
                this -> ctx.shutdown();
            }

            bool recieve_sectioned_message( const unsigned int UI16_PACKET_SIZE, 
                                            const unsigned int UI32_PACKET_SIZE,
                                            const unsigned int F32_PACKET_SIZE,
                                            recvd_data& data_recieved ){
                
                // printf("Reading %u Data Elements ... \n", UI16_PACKET_SIZE + UI32_PACKET_SIZE + F32_PACKET_SIZE);
                zmq::message_t envelope( this -> envelope_size );
                bool recv_pass = skt.recv( &envelope, 0 );
                
                // TOPIC
                const char* topic_out = (char*) envelope.data(); 
                uint16_t ui16_buffer[UI16_PACKET_SIZE];
                uint32_t ui32_buffer[UI32_PACKET_SIZE];
                _Float32  f32_buffer[ F32_PACKET_SIZE];

                if(!recv_pass ){ return false; }

                // SKIP PADDING
                // DATA 
                memcpy( ui16_buffer, ((char*)envelope.data() + this -> topic_size + 1 ), 
                        UI16_PACKET_SIZE * sizeof(uint16_t) );
                for( size_t ii = 0; ii < UI16_PACKET_SIZE;  ii++ ){
                    data_recieved.ui16_data[ii] = ui16_buffer[ii];
                }

                memcpy( ui32_buffer, ((char*)envelope.data() + this -> topic_size + 1 + (UI16_PACKET_SIZE * sizeof(uint16_t)) ), 
                        UI32_PACKET_SIZE * sizeof(uint32_t) );
                for( size_t ii = 0; ii < UI32_PACKET_SIZE;  ii++ ){
                    data_recieved.ui32_data[ii] = ui32_buffer[ii];
                }

                memcpy( f32_buffer, ((char*)envelope.data() + this -> topic_size + 1 + (UI16_PACKET_SIZE * sizeof(uint16_t)) + (UI32_PACKET_SIZE * sizeof(uint32_t)) ), 
                        F32_PACKET_SIZE * sizeof(_Float32));
                for( size_t ii = 0; ii < F32_PACKET_SIZE;  ii++ ){
                    data_recieved.f32_data[ii] = f32_buffer[ii];
                }

                return true;
            }

    };
}



int main(){

    const char* tcp_address = "tcp://localhost:5555";
    const char* tcp_topic = "test_topic";
    const unsigned int UI16_PACKET_SIZE = 4;
    const unsigned int UI32_PACKET_SIZE = 4;
    const unsigned int F32_PACKET_SIZE = 4;

    size_t data_size = ( UI16_PACKET_SIZE * sizeof(uint16_t) ) + ( UI32_PACKET_SIZE * sizeof(uint32_t) ) + ( F32_PACKET_SIZE * sizeof(_Float32) );

    utlift_zmq_cnnct::subscriber_zmq_cpp test_zmq_publisher( tcp_address, tcp_topic, data_size );
    utlift_zmq_cnnct::recvd_data data_recieved(UI16_PACKET_SIZE, UI32_PACKET_SIZE, F32_PACKET_SIZE);
    
    unsigned int i = 0;
    while( true ){
        bool recv_pass = test_zmq_publisher.recieve_sectioned_message( UI16_PACKET_SIZE, UI32_PACKET_SIZE, F32_PACKET_SIZE, data_recieved );
        if( !recv_pass ){
            continue;
        }else{
           printf("Message %u Recieved on Topic: %s\n\n", i, tcp_topic);
           i = i + 1; 
           for( int ii = 0; ii<UI16_PACKET_SIZE; ii++ ){
               std::cout << data_recieved.ui16_data[ii] << std::endl;
           }
           for( int ii = 0; ii<UI32_PACKET_SIZE; ii++ ){
               std::cout << data_recieved.ui32_data[ii] << std::endl;
           }
           for( int ii = 0; ii<F32_PACKET_SIZE; ii++ ){
               std::cout << data_recieved.f32_data[ii] << std::endl;
           }
        }
        printf("\n");
    }
    return EXIT_SUCCESS;
}