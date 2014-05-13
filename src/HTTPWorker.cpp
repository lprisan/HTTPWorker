//============================================================================
// Name        : HTTPWorker.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>      /* printf */
#include "Time.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <json/json.h>
#include <curl/curl.h>
#include "DeviceState.hpp"

using namespace std;

Json::Value getJSON(std::string jsonMessage){

	Json::Value parsedFromString;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(jsonMessage, parsedFromString);
	if (parsingSuccessful)
	{
		//Json::StyledWriter styledWriter;
		//std::cout << styledWriter.write(parsedFromString) << std::endl;
		std::cout << "Parsing successful!" << std::endl;
		//std::cout << jsonMessage << std::endl;
	}
	return(parsedFromString);
}

//std::string get_file_contents(const char *filename)
//{
//	ifstream in(filename);
//	std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
//	return(s);
//}

// callback function writes data to a std::ostream
static size_t data_write(void* buf, size_t size, size_t nmemb, void* userp)
{
	if(userp)
	{
		std::ostream& os = *static_cast<std::ostream*>(userp);
		std::streamsize len = size * nmemb;
		if(os.write(static_cast<char*>(buf), len))
			return len;
	}

	return 0;
}

/**
 * timeout is in seconds
 **/
CURLcode curl_read(const std::string& url, std::ostream& os, long timeout = 30)
{
	CURLcode code(CURLE_FAILED_INIT);
	CURL* curl = curl_easy_init();

	if(curl)
	{
		if(CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &data_write))
		&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
		&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
		&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
		&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
		&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str())))
		{
			code = curl_easy_perform(curl);
		}
		curl_easy_cleanup(curl);
	}
	return code;
}


DeviceState getDeviceFromJSON(Json::Value value){

	DeviceState device = DeviceState();
	Json::Value meteorId = value[scMeteorIdLabel];
	device.SetMeteorId(meteorId.asString());

	//TODO: complete building the whole object

	return(device);
}


DeviceState getRemoteDeviceState(std::string pUrl){

	DeviceState device = DeviceState();

	std::ostringstream oss;
	std::string jsonstring;
	if(CURLE_OK == curl_read(pUrl, oss))
	{
		// Web page successfully written to string
		jsonstring = oss.str();
	}

	if(jsonstring.length()>0){
		//cout << "Received json: " << jsonstring << endl;
		//Parse the json
		Json::Value json = getJSON(jsonstring);
		device = getDeviceFromJSON(json);

		cout << "Extracted device state for " << device.GetMeteorId() << endl;
	}

	return(device);
}



int main() {
	cout << "Initializing worker...." << endl;
	curl_global_init(CURL_GLOBAL_ALL);

	int changeInterval = 100; //periodicity of checking if a request is needed, in ms

	int classCheckInterval = 2000; // pediodicity of sending requests, independently of whether the file has changed

	cout << "Getting device initial state...";
	std::string remoteDeviceUrl = "http://localhost:3000/api/devices/LziCQ4oJQ7bpQv7sA";
	DeviceState localDevState = getRemoteDeviceState(remoteDeviceUrl);
	cout << "Success!" << endl;

	long initialTimestamp = Time::MillisTimestamp();

	while(1){

		long timestamp = Time::MillisTimestamp();

		if(timestamp-initialTimestamp>classCheckInterval){
			//if the class checking interval has passed, we get the new classroom state from the server

			//we set the local classroom state variable in memory (with a mutex)

			//we set the initial timestamp to the new one
		}

		//TODO: for testing, here we can load a JSON from a manually editable file

		if(localDevState.hasChanged()){//if some other thread changed the device state
			//copy the local state to a temp variable

			//convert the object to JSON

			//do PUT of the JSON object to the server

			//if PUT was successful, we update the local state to our temp variable with hasChanged=false (with mutex)
			//TODO: we can also check if it has changed meanwhile, if so, we do not overwrite it and we go to the next iteration

			//if not, we do nothing and try again the next iteration
		}

		timestamp = Time::MillisTimestamp();
		if(timestamp-initialTimestamp<classCheckInterval && !localDevState.hasChanged()){//we check if there is a need for a new class state check or device update
			//if not, we wait for a short period of time
			Time::Sleep(changeInterval);
		}

	}

	curl_global_cleanup();

	return 0;
}



