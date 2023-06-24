depend:
	sudo apt-get install libpam0g-dev libcurl4-openssl-dev wget
	wget "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp"

compile:
	mkdir -p build
	g++ -shared -fPIC -O1 pam_telegram.cpp -o ./build/pam_telegram.so -lpam -lcurl	