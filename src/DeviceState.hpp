/*
 * DeviceState.hpp
 *
 *  Created on: May 13, 2014
 *      Author: lprisan
 */

#ifndef DEVICESTATE_HPP_
#define DEVICESTATE_HPP_

#include <string>
#include <vector>


	//Labels are for the JSON serialization
	  static const std::string scNumeratorLabel = "numerator";
	  static const std::string scDenominatorLabel = "denominator";
	  static const std::string scValueLabel = "value";
	  static const std::string scRepresentationLabel = "representation";
	  static const std::string scIdLabel = "id";
	  static const std::string scNameLabel = "name";
	  static const std::string scStateLabel = "state";
		static const std::string scMeteorIdLabel="_id";
		static const std::string scCurrentLabel="current";
		static const std::string scActivityLabel="activity";
		static const std::string scPresentTagsLabel="presentTags";



enum Representations {circular, rectangular, tokens, tangram};

struct state{


	  int numerator;
	  int denominator;
	  float value;
	  Representations representation;

  };

struct activity_state {
  int id;
  std::string name;
  state currentState;
};


class DeviceState {
public:
	DeviceState();
	virtual ~DeviceState();

//	const std::string *GetMeteorId() const {return &mMeteorId;}
//    const std::string *GetId() const {return &mId;};
//	const std::string *GetName() const {return &mName;};
//	const activity_state *GetActivity() const {return &mActivity;};
//	const std::vector<std::string> *GetPresentTags() const {return &mPresentTags;};

	std::string GetMeteorId() {return mMeteorId;}
    int GetId() {return mId;};
	std::string GetName() {return mName;};
	activity_state GetActivity() {return mActivity;};
	std::vector<std::string> GetPresentTags() {return mPresentTags;};
	bool hasChanged() {return mChanged;}

	void SetMeteorId(std::string meteorId) {mMeteorId = meteorId;}
	void SetId(int id) {mId = id;}
	void SetName(std::string name) {mName = name;}
	void SetActivity(activity_state activity) {mActivity = activity;}
	void SetPresentTags(std::vector<std::string> presentTags) {mPresentTags = presentTags;}
	void SetHasChanged(bool changed) {mChanged = changed;}

	bool equals(DeviceState other);

private:
	std::string mMeteorId;

	int mId;

	std::string mName;

	activity_state mActivity;

	std::vector<std::string> mPresentTags;

	bool mChanged;

};

#endif /* DEVICESTATE_HPP_ */
