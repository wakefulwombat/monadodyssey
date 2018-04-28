#include "input.h"
#include "DxLib.h"
#include <algorithm>

//Key///////////////////////////////////////////////////////////////////////////
void Key::initialize() {
	this->down_counter = 0;
	this->up_counter = 10000;
	this->re_down_time = 10000;
	this->re_up_time = 10000;
}

void Key::update_on(bool is_on) {
	if (is_on) {
		this->down_counter++;
		if (this->down_counter == 1) {
			this->re_down_time = this->up_counter;
		}
		this->up_counter = 0;
	}

	else {
		this->up_counter++;
		if (this->up_counter == 1) {
			this->re_up_time = this->down_counter;
		}
		this->down_counter = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////



//KeyCheck//////////////////////////////////////////////////////////////////////
template <typename T_UserKeyID>
void KeyManager<T_UserKeyID>::update() {
	if (!this->accept_update_input) return;
	this->time_counter++;
	std::vector<T_UserKeyID> k;
	for (std::unordered_map<T_UserKeyID, std::shared_ptr<Key>>::iterator it = this->check_keys.begin(); it != this->check_keys.end(); ++it) {
		it->second->update_on(this->keyCheck(it->second->getLibID()));
		if (it->second->isDownOnce()) k.push_back(it->first);
	}

	if (k.size() > 0) {
		this->downed_time = this->time_counter;
		this->time_counter = 0;
		this->downed_keys_pre = this->downed_keys;
		this->downed_keys = k;
	}
}

template <typename T_UserKeyID>
void KeyManager<T_UserKeyID>::updateFromLog(std::function<bool(T_UserKeyID)> keyCheck_log){
	if (!this->accept_update_input) return;
	this->time_counter++;
	std::vector<T_UserKeyID> k;
	for (std::unordered_map<T_UserKeyID, std::shared_ptr<Key>>::iterator it = this->check_keys.begin(); it != this->check_keys.end(); ++it) {
		it->second->update_on(keyCheck_log(it->first));
		if (it->second->isDownOnce()) k.push_back(it->first);
	}

	if (k.size() > 0) {
		this->downed_time = this->time_counter;
		this->time_counter = 0;
		this->downed_keys_pre = this->downed_keys;
		this->downed_keys = k;
	}
}

template <typename T_UserKeyID>
bool KeyManager<T_UserKeyID>::isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) {
	if (code1 == code2) return false;
	if (std::find(this->downed_keys_pre.begin(), this->downed_keys_pre.end(), code1) != this->downed_keys_pre.end()) {
		if (std::find(this->downed_keys.begin(), this->downed_keys.end(), code2) != this->downed_keys.end()) {
			if ((this->downed_time <= allow_time) && (this->time_counter == 0)) return true;
		}
	}
	if (std::find(this->downed_keys_pre.begin(), this->downed_keys_pre.end(), code2) != this->downed_keys_pre.end()) {
		if (std::find(this->downed_keys.begin(), this->downed_keys.end(), code1) != this->downed_keys.end()) {
			if ((this->downed_time <= allow_time) && (this->time_counter == 0)) return true;
		}
	}
	return false;
}

template <typename T_UserKeyID>
bool KeyManager<T_UserKeyID>::isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) {
	if (code1 == code2) return false;
	if (std::find(this->downed_keys_pre.begin(), this->downed_keys_pre.end(), code1) != this->downed_keys_pre.end()) {
		if (std::find(this->downed_keys.begin(), this->downed_keys.end(), code2) != this->downed_keys.end()) {
			if ((this->downed_time <= allow_time) && (this->time_counter == 0)) return true;
		}
	}
	return false;
}

template <typename T_UserKeyID>
bool KeyManager<T_UserKeyID>::isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time) {
	if (code1 == code2) return false;
	if (std::find(this->downed_keys_pre.begin(), this->downed_keys_pre.end(), code1) != this->downed_keys_pre.end()) {
		if (std::find(this->downed_keys.begin(), this->downed_keys.end(), code2) != this->downed_keys.end()) {
			if ((this->downed_time >= min_time) && (this->downed_time <= max_time) && (this->time_counter == 0)) return true;
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////


//Command///////////////////////////////////////////////////////////////////////
void CommandFlow::update() {
	if (this->checking_index == 0){
		if (this->start_trigger()) {
			this->start_passed();
			if (this->checking_index != this->command_flow.size()) this->checking_index++;
			this->time = 0;
		}
	}
	else{
		this->time++;
		if (this->time < this->command_flow[this->checking_index - 1]->under_limit_time) return;
		//ŽžŠÔ§ŒÀðŒ
		if (this->command_flow[this->checking_index - 1]->continuous_condition == nullptr) {
			if (this->time > this->command_flow[this->checking_index - 1]->limit_time) {
				this->command_flow[this->checking_index - 1]->when_miss();
				this->checking_index = 0;
			}
			else {
				if (this->command_flow[this->checking_index - 1]->pass_condition()) {
					this->command_flow[this->checking_index - 1]->when_ok();
					if (this->checking_index == this->command_flow.size()) {
						this->checking_index = 0;
					}
					else {
						this->checking_index++;
						this->time = 0;
					}
				}
			}
		}
		//Œp‘±ðŒ
		else {
			if (this->command_flow[this->checking_index - 1]->continuous_condition()) {
				if (this->command_flow[this->checking_index - 1]->pass_condition()) {
					this->command_flow[this->checking_index - 1]->when_ok();
					if (this->checking_index == this->command_flow.size()) {
						this->checking_index = 0;
					}
					else {
						this->checking_index++;
						this->time = 0;
					}
				}
			}
			else {
				this->command_flow[this->checking_index - 1]->when_miss();
				this->checking_index = 0;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////




//AnalogStickManager////////////////////////////////////////////////////////////
template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isInRangeOfAngle(double check_ang, double ang1, double ang2)
{
	double range = min(abs(ang1 - ang2), abs(M_PI*2 + ang2 - ang1));
	double delta1 = min(abs(ang1 - check_ang), abs(M_PI*2 + check_ang - ang1));
	double delta2 = min(abs(ang2 - check_ang), abs(M_PI*2 + check_ang - ang2));
	if (delta1 > range) return false;
	if (delta2 > range) return false;
	return true;
}

template <typename T_UserKeyID>
Vec2D AnalogStickManager<T_UserKeyID>::getSlope(AnalogStickLeftRight lr, double r_min, double r_max)
{
	if (this->return_all_false) return Vec2D();

	double norm = this->get_analog_stick_slope(lr).toNorm();
	double angle = atan2(this->get_analog_stick_slope(lr).y, this->get_analog_stick_slope(lr).x);
	if (norm < r_min) return Vec2D();
	if (norm > r_max) return Vec2D::fromPowAng(1.0, angle);
	return Vec2D::fromPowAng(norm / (r_max - r_min), angle);
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isSlopeOverRangeUpToTime(AnalogStickLeftRight lr, double per_min, unsigned int time_min)
{
	if (this->return_all_false) return false;

	for (unsigned int i = 0; i <= time_min; ++i) {
		if (this->analog_stick_slope_raw[(int)lr][i].toNorm() < per_min) return false;
	}
	return true;
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isSlopeWithinRangeUpToTime(AnalogStickLeftRight lr, double per_min, double per_max, unsigned time_min)
{
	if (this->return_all_false) return false;

	double norm;
	for (unsigned int i = 0; i <= time_min; ++i) {
		norm = this->analog_stick_slope_raw[(int)lr][i].toNorm();
		if (norm < per_min) return false;
		if (norm > per_max) return false;
	}
	return true;
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isInDirection(AnalogStickLeftRight lr, double ang1, double ang2, double per_min, double per_max)
{
	if (this->return_all_false) return false;

	double norm = this->get_analog_stick_slope(lr).toNorm();
	double angle = atan2(this->analog_stick_slope_raw[(int)lr][0].y, this->analog_stick_slope_raw[(int)lr][0].x);
	if (norm < per_min) return false;
	if (norm > per_max) return false;
	if (!this->isInRangeOfAngle(angle, ang1, ang2)) return false;
	return true;
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isRoundClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time)
{
	if (this->return_all_false) return false;

	double total_ang = 0.0;
	double cross;

	if (this->analog_stick_slope_raw[(int)lr][0].toNorm() < per_min) return false;
	for (unsigned int i = 1; i <= per_min; ++i) {
		if (this->analog_stick_slope_raw[(int)lr][i].toNorm() < per_min) return false;

		Vec2D aft = this->analog_stick_slope_raw[(int)lr][i - 1];
		Vec2D bef = this->analog_stick_slope_raw[(int)lr][i];
		double aft_ang = atan2(aft.y, aft.x);
		double bef_ang = atan2(bef.y, bef.x);
		cross = Vec2D::cross(bef, aft);
		if (cross >= 0.0){
			total_ang += min(abs(aft_ang - bef_ang), abs(M_PI * 2 - aft_ang + bef_ang));
		}

		if (total_ang > M_PI * 2) return true;
	}
	return false;
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isRoundCounterClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time)
{
	if (this->return_all_false) return false;

	double total_ang = 0.0;
	double cross;

	if (this->analog_stick_slope_raw[(int)lr][0].toNorm() < per_min) return false;
	for (unsigned int i = 1; i <= per_min; ++i) {
		if (this->analog_stick_slope_raw[(int)lr][i].toNorm() < per_min) return false;

		Vec2D aft = this->analog_stick_slope_raw[(int)lr][i - 1];
		Vec2D bef = this->analog_stick_slope_raw[(int)lr][i];
		double aft_ang = atan2(aft.y, aft.x);
		double bef_ang = atan2(bef.y, bef.x);
		cross = Vec2D::cross(bef, aft);
		if (cross <= 0.0){
			total_ang += min(abs(aft_ang - bef_ang), abs(M_PI * 2 - aft_ang + bef_ang));
		}

		if (total_ang > M_PI * 2) return true;
	}
	return false;
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction4 dir, unsigned int time)
{
	if (this->return_all_false) return false;

	bool isCrossOut = false;
	for (unsigned int i = time - 1; i >= 0; --i){
		if (!isCrossOut){
			if ((this->analog_stick_slope_raw[(int)lr][i + 1].toNorm() < slope_threshold_crossOut) && (this->analog_stick_slope_raw[(int)lr][i].toNorm() >= slope_threshold_crossOut)){
				isCrossOut = true;
			}
		}
		else{
			if (this->analog_stick_slope_raw[(int)lr][i].toNorm() > slope_threshold_crossOut){
				if (Vec2D::toDirection4(this->analog_stick_slope_raw[(int)lr][i]) != dir) return false;
			}
			if (this->analog_stick_slope_raw[(int)lr][i].toNorm() <= slope_threshold_returnInto) return true;
		}
	}
	return false;
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction8 dir, unsigned int time)
{
	if (this->return_all_false) return false;

	bool isCrossOut = false;
	for (unsigned int i = time - 1; i >= 0; --i){
		if (!isCrossOut){
			if ((this->analog_stick_slope_raw[(int)lr][i + 1].toNorm() < slope_threshold_crossOut) && (this->analog_stick_slope_raw[(int)lr][i].toNorm() >= slope_threshold_crossOut)){
				isCrossOut = true;
			}
		}
		else{
			if (this->analog_stick_slope_raw[(int)lr][i].toNorm() > slope_threshold_crossOut){
				if (Vec2D::toDirection8(this->analog_stick_slope_raw[(int)lr][i]) != dir) return false;
			}
			if (this->analog_stick_slope_raw[(int)lr][i].toNorm() <= slope_threshold_returnInto) return true;
		}
	}
	return false;
}

template <typename T_UserKeyID>
bool AnalogStickManager<T_UserKeyID>::isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, double ang1, double ang2, unsigned int time)
{
	if (this->return_all_false) return false;

	bool isCrossOut = false;
	for (unsigned int i = time - 1; i >= 0; --i){
		if (!isCrossOut){
			if ((this->analog_stick_slope_raw[(int)lr][i + 1].toNorm() < slope_threshold_crossOut) && (this->analog_stick_slope_raw[(int)lr][i].toNorm() >= slope_threshold_crossOut)){
				isCrossOut = true;
			}
		}
		else{
			double angle = atan2(this->analog_stick_slope_raw[(int)lr][i].y, this->analog_stick_slope_raw[(int)lr][i].x);
			if (this->analog_stick_slope_raw[(int)lr][i].toNorm() > slope_threshold_crossOut){
				if (!this->isInRangeOfAngle(angle, ang1, ang2)) return false;
			}
			if (this->analog_stick_slope_raw[(int)lr][i].toNorm() <= slope_threshold_returnInto) return true;
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////


//Input/////////////////////////////////////////////////////////////////////////
std::shared_ptr<KeyBoardManager<std::string, std::string, unsigned int>> Input_T::keyBoard_mgr = std::make_shared<KeyBoardManager<std::string, std::string, unsigned int>>();
std::shared_ptr<MouseManager<std::string, std::string, unsigned int>> Input_T::mouse_mgr = std::make_shared<MouseManager<std::string, std::string, unsigned int>>();
std::shared_ptr<GamePadManager<std::string, std::string, unsigned int>> Input_T::gamePad_mgr = std::make_shared<GamePadManager<std::string, std::string, unsigned int>>();

void Input_T::setReturnAllKeyFalse(bool readable) {
	Input_T::keyBoard_mgr->setReturnAllKeyFalse(readable);
	Input_T::mouse_mgr->setReturnAllKeyFalse(readable);
	Input_T::gamePad_mgr->setReturnAllKeyFalse(readable);
}

void Input_T::acceptUpdateInput(bool accept) {
	Input_T::keyBoard_mgr->acceptUpdateInput(accept);
	Input_T::mouse_mgr->acceptUpdateInput(accept);
	Input_T::gamePad_mgr->acceptUpdateInput(accept);
}
////////////////////////////////////////////////////////////////////////////////