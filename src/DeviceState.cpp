/*
 * DeviceState.cpp
 *
 *  Created on: May 13, 2014
 *      Author: lprisan
 */

#include "DeviceState.hpp"

DeviceState::DeviceState() {
	mChanged = false;

}

DeviceState::~DeviceState() {
	// TODO Auto-generated destructor stub
}

bool DeviceState::equals(DeviceState other){

	if(this->mId != other.GetId()) return false;
	if(this->mMeteorId != other.GetMeteorId()) return false;
	if(this->mName.compare(other.GetName())!=0) return false;
	if(this->mPresentTags.size() != other.GetPresentTags().size()) return false;
	else{//we check all vector elements for present tags
		for(int i=0;i<this->mPresentTags.size();i++){
			if(this->mPresentTags.at(i).compare(other.GetPresentTags().at(i))!=0){
				return false;
			}
		}
	}
	//check the activity state
	if(this->mActivity.id!=other.GetActivity().id || this->mActivity.name.compare(other.GetActivity().name)!=0) return false;
	else{//we check the activity state
		if(this->mActivity.currentState.denominator != other.GetActivity().currentState.denominator ||
				this->mActivity.currentState.numerator != other.GetActivity().currentState.numerator ||
				this->mActivity.currentState.value != other.GetActivity().currentState.value ||
				this->mActivity.currentState.representation != other.GetActivity().currentState.representation) return false;
		//TODO: Maybe add a tolerance margin in value, since it is a float?
	}

	//If all else failed, they must be equal!
	return true;
}
