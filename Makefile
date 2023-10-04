compile_test_server:
	g++ -std=c++11 cpp_publisher.cpp -o cpp_publisher -lzmq

compile_test_subscriber:
	g++ -std=c++11 cpp_subscriber.cpp -o cpp_subscriber -lzmq

install_dependents:
	sudo apt-get install clang libzmq5 libzmq3-dev python3 python3-zmq