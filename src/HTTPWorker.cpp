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
#include <pthread.h>

using namespace std;

Json::Value getJSON(std::string jsonMessage){

	Json::Value parsedFromString;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(jsonMessage, parsedFromString);
	if (parsingSuccessful)
	{
		//Json::StyledWriter styledWriter;
		//std::cout << styledWriter.write(parsedFromString) << std::endl;
		//std::cout << "Parsing successful!" << std::endl;
		//std::cout << jsonMessage << std::endl;
		return(parsedFromString);
	}
	return(parsedFromString);
}

std::string get_file_contents(const char *filename)
{
	ifstream in(filename);
	std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	return(s);
}

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

	Json::FastWriter fastWriter;
	//cout << fastWriter.write(value) << endl;



	DeviceState device = DeviceState();
	Json::Value meteorId = value[scMeteorIdLabel];
	device.SetMeteorId(meteorId.asString());

	Json::Value id = value[scIdLabel];
	device.SetId(id.asInt());

	Json::Value name = value[scNameLabel];
	device.SetName(name.asString());

	activity_state activity;
	Json::Value currObj = value[scCurrentLabel];
	Json::Value present = currObj[scPresentTagsLabel];
	std::vector<std::string> tags;
	for (unsigned int i=0;i<present.size();i++){
		//cout << "Parsing array element " << i << ": " << present[i].asString() << endl;
		if(present[i].asString().length()>0){
			tags.push_back(present[i].asString());
		}
	}
	device.SetPresentTags(tags);


	Json::Value actObj = currObj[scActivityLabel];
	activity.id = actObj[scIdLabel].asInt();
	activity.name = actObj[scNameLabel].asString();
	Json::Value stateObj = actObj[scStateLabel];
	state st;
	st.denominator = stateObj[scDenominatorLabel].asInt();
	st.numerator = stateObj[scNumeratorLabel].asInt();
	st.value = stateObj[scValueLabel].asFloat();
	if(stateObj[scRepresentationLabel].asString().compare("circular")==0){
		st.representation = circular;
	} else if(stateObj[scRepresentationLabel].asString().compare("rectangular")==0){
		st.representation = rectangular;
	} else if(stateObj[scRepresentationLabel].asString().compare("tokens")==0){
		st.representation = tokens;
	} else if(stateObj[scRepresentationLabel].asString().compare("tangram")==0){
		st.representation = tangram;
	}
	activity.currentState = st;
	device.SetActivity(activity);

	device.SetHasChanged(false);

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

		//cout << "Extracted device state for " << device.GetMeteorId() << endl;
	}

	return(device);
}

bool putRemoteString(std::string pUrl, std::string data){

	CURLcode code(CURLE_FAILED_INIT);
	CURL* curl = curl_easy_init();
	bool success = false;
	if (curl) {
		struct curl_slist *headers=NULL; /* init to NULL is important */
		//headers = curl_slist_append(headers, client_id_header);
	    headers = curl_slist_append(headers, "Content-Type: application/json");

	    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	    curl_easy_setopt(curl, CURLOPT_URL, pUrl.c_str());
	    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */

	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str()); /* data goes here */

	    code = curl_easy_perform(curl);
	    if(code==CURLE_OK) success = true;

	    curl_slist_free_all(headers);
	    curl_easy_cleanup(curl);
	}
	return(success);
}

std::string getJSONFromDevice(DeviceState state){

	std::string data;

	Json::Value json;
	json[scMeteorIdLabel] = state.GetMeteorId();

	//current state
	Json::Value current;

	//activity state
	Json::Value activity;
	Json::Value act_state;
	act_state[scNumeratorLabel] = state.GetActivity().currentState.numerator;
	act_state[scDenominatorLabel] = state.GetActivity().currentState.denominator;
	act_state[scValueLabel] = state.GetActivity().currentState.value;
	if(state.GetActivity().currentState.representation == circular)	act_state[scRepresentationLabel] = "circular";
	else if(state.GetActivity().currentState.representation == rectangular)	act_state[scRepresentationLabel] = "rectangular";
	else if(state.GetActivity().currentState.representation == tokens)	act_state[scRepresentationLabel] = "tokens";
	else if(state.GetActivity().currentState.representation == tangram)	act_state[scRepresentationLabel] = "tangram";

	activity[scStateLabel] = act_state;
	activity[scIdLabel] = state.GetActivity().id;
	activity[scNameLabel] = state.GetActivity().name;
	current[scActivityLabel] = activity;

	//present tags
	Json::Value presentTags;
	if(state.GetPresentTags().size()==0){
		current[scPresentTagsLabel] = Json::Value(Json::arrayValue);
	}else{
		for(int i=0;i<state.GetPresentTags().size();i++){
			presentTags.append(state.GetPresentTags().at(i));
		}
		current[scPresentTagsLabel] = presentTags;
	}

	json[scCurrentLabel] = current;

	json[scIdLabel] = state.GetId();
	json[scNameLabel] = state.GetName();

	Json::FastWriter fastWriter;
	data = fastWriter.write(json);

	//cout << "Built JSON object: " << data << endl;

	return data;
}


//Local variable to hold the current lamp state with respect to the classroom
pthread_mutex_t devstate_mutex;
DeviceState localDevState;

//Make the setting of the global variable thread-safe
void SetLocalDeviceState(DeviceState state){
	pthread_mutex_lock(&devstate_mutex);
	localDevState = state;
	pthread_mutex_unlock(&devstate_mutex);
}


int main() {
	cout << "Initializing worker...." << endl;
	curl_global_init(CURL_GLOBAL_ALL);

	int deviceCheckInterval = 500; //periodicity of checking if a request is needed, in ms

	int classCheckInterval = 2000; // pediodicity of sending requests, independently of whether the file has changed

	cout << "Getting device initial state...";
	std::string remoteDeviceUrl = "http://localhost:3000/api/devices/LziCQ4oJQ7bpQv7sA";
	DeviceState tmpState = getRemoteDeviceState(remoteDeviceUrl);
	if(tmpState.GetMeteorId().length()!=0) SetLocalDeviceState(tmpState);
	cout << "Success!" << endl;

	long initialTimestampClass = Time::MillisTimestamp();
	long initialTimestampDevice = Time::MillisTimestamp();

	while(1){

		long timestamp = Time::MillisTimestamp();
		//cout << timestamp << " - Loop beginning" << endl;

		if(timestamp-initialTimestampClass>classCheckInterval){
			//if the class checking interval has passed, we get the new classroom state from the server
			cout << timestamp << " - We get the classroom state from the server" << endl;

			//we set the local classroom state variable in memory (with a mutex)

			//we set the initial timestamp to the new one
			initialTimestampClass = timestamp;
		}

		//for testing, here we can load a JSON from a manually editable file
		std::string jsonFile = get_file_contents("/home/lprisan/workspace/HTTPWorker/device1.json");
		Json::Value json = getJSON(jsonFile);
		DeviceState tempState = getDeviceFromJSON(json);
		if(!tempState.equals(localDevState)){
			tempState.SetHasChanged(true);
			SetLocalDeviceState(tempState);
		}

		if(localDevState.hasChanged()){//if some other thread changed the device state
			//copy the local state to a temp variable
			DeviceState state = localDevState;
			cout << "Detected state change: " << getJSONFromDevice(state) << endl;

			//convert the object to JSON
			std::string jsonState = getJSONFromDevice(state);

			//do PUT of the JSON object to the server
			bool success = false;
			cout << "PUTting remote JSON...";
			success = putRemoteString(remoteDeviceUrl,jsonState);
			if(success) cout << "Success!" << endl;
			else cout << "Failure!" << endl;

			//if PUT was successful, we update the local state to our temp variable with hasChanged=false (with mutex)
			//cout << "temp state: " << getJSONFromDevice(state) << endl;
			//cout << "in memory state: " << getJSONFromDevice(localDevState) << endl;

			//we also check if it has changed meanwhile, if so, we do not overwrite it and we go to the next iteration
			if(success && localDevState.equals(state)){
				cout << "Updating local device state in memory..." << endl;
				state.SetHasChanged(false);
				SetLocalDeviceState(state);
			}
			//if not successful, we do nothing and try again the next iteration
		}

		timestamp = Time::MillisTimestamp();
		//if(timestamp-initialTimestamp<classCheckInterval && !localDevState.hasChanged()){//we check if there is a need for a new class state check or device update
		if(timestamp-initialTimestampDevice<deviceCheckInterval){//we check if there is a need for a new device state check, if not we sleep
			//if not, we wait for a short period of time
			cout << "Going to sleep for (ms)" << deviceCheckInterval << endl;

			Time::Sleep(deviceCheckInterval);
		}else initialTimestampDevice = timestamp;

		//cout << timestamp << " - Loop end" << endl;

	}

	curl_global_cleanup();

	return 0;
}



