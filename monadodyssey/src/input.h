#pragma once

#include "common.h"
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <functional>
#include <unordered_map>

#pragma region Command
class OneCommand {
public:
	std::function<bool(void)> pass_condition, continuous_condition;
	std::function<void(void)> when_ok, when_miss;
	unsigned int under_limit_time, limit_time;

	OneCommand(std::function<bool(void)> pass_condition, unsigned int limit_time, std::function<void(void)> when_ok = []() {}, std::function<void(void)> when_timeout = []() {}, unsigned int under_limit_time = 0) {
		this->pass_condition = pass_condition; this->continuous_condition = nullptr; this->when_ok = when_ok; this->when_miss = when_timeout; this->under_limit_time = under_limit_time; this->limit_time = limit_time;
	}
	OneCommand(std::function<bool(void)> pass_condition, std::function<bool(void)> continuous_condition, std::function<void(void)> when_ok = []() {}, std::function<void(void)> when_miss = []() {}, unsigned int under_limit_time = 0) {
		this->pass_condition = pass_condition; this->continuous_condition = continuous_condition; this->when_ok = when_ok; this->when_miss = when_miss; this->under_limit_time = under_limit_time;
	}
};

//複数のキー入力の流れに対して判定を行う
class CommandFlow {
private:
	unsigned int checking_index;
	unsigned int time;
	std::function<bool(void)> start_trigger;
	std::function<void(void)> start_passed;
	std::vector<std::shared_ptr<OneCommand>> command_flow;

public:
	CommandFlow(std::function<bool(void)> start_trigger, std::function<void(void)> start_passed = []() {}) { this->initialize(); this->start_trigger = start_trigger; this->start_passed = start_passed; }
	void initialize() { this->checking_index = 0; this->time = 0; };
	void update();

	void addNextCommand(const std::shared_ptr<OneCommand> &command) { this->command_flow.push_back(command); }
	void addNextCommand(std::function<bool(void)> pass_condition, unsigned int limit_time, std::function<void(void)> when_ok = []() {}, std::function<void(void)> when_timeout = []() {}, unsigned int under_limit_time = 0) {
		this->command_flow.push_back(std::make_shared<OneCommand>(pass_condition, limit_time, when_ok, when_timeout, under_limit_time));
	}
	void addNextCommand(std::function<bool(void)> pass_condition, std::function<bool(void)> continuous_pass, std::function<void(void)> when_ok = []() {}, std::function<void(void)> when_miss = []() {}, unsigned int under_limit_time = 0) {
		this->command_flow.push_back(std::make_shared<OneCommand>(pass_condition, continuous_pass, when_ok, when_miss, under_limit_time));
	}
};

template <typename T_UserCommandid>
class CommandOperationInterface {
public:
	~CommandOperationInterface() {}
	virtual void makeNewCommandFlowListener(T_UserCommandid id, std::function<bool(void)> start_trigger, std::function<void(void)> start_passed = []() {}) = 0;
	virtual void removeCommandFlowListenerAll() = 0;
	virtual void removeCommandFlowListener(T_UserCommandid id) = 0;
	virtual void addNextCommandToFlow(T_UserCommandid command_id, const std::shared_ptr<OneCommand> &command) = 0;
	virtual void addNextCommandToFlow(T_UserCommandid command_id, std::function<bool(void)> pass_condition, unsigned int limit_time, std::function<void(void)> when_pass = []() {}, std::function<void(void)> when_timeout = []() {}, unsigned int under_limit_time = 0) = 0;
	virtual void addNextCommandToFlow(T_UserCommandid command_id, std::function<bool(void)> pass_condition, std::function<bool(void)> continuous_pass, std::function<void(void)> when_ok = []() {}, std::function<void(void)> when_miss = []() {}, unsigned int under_limit_time = 0) = 0;
};

template <typename T_UserCommandid>
class CommandManager : public CommandOperationInterface<T_UserCommandid> {
private:
	std::unordered_map<T_UserCommandid, std::shared_ptr<CommandFlow>> commandFlow_list;
	bool accept_update_input, return_all_false;

public:
	CommandManager() { this->initialize(); }
	void initialize() { this->accept_update_input = true; this->return_all_false = false; this->commandFlow_list.clear(); }
	void update() { if (!this->accept_update_input) return; for (auto it = this->commandFlow_list.begin(); it != this->commandFlow_list.end(); ++it) this->commandFlow_list[it->first]->update(); }

	void acceptUpdateInput(bool accept) { this->accept_update_input = accept; }
	void setReturnAllKeyFalse(bool all_false) { this->return_all_false = all_false; }

	void makeNewCommandFlowListener(T_UserCommandid id, std::function<bool(void)> start_trigger, std::function<void(void)> start_passed = []() {}) override { this->commandFlow_list[id] = std::make_shared<CommandFlow>(start_trigger, start_passed); }
	void removeCommandFlowListenerAll() override { this->commandFlow_list.clear(); }
	void removeCommandFlowListener(T_UserCommandid id) override { this->commandFlow_list.erase(id); }
	void addNextCommandToFlow(T_UserCommandid command_id, const std::shared_ptr<OneCommand> &command) override { if (this->commandFlow_list.find(command_id) != this->commandFlow_list.end()) this->commandFlow_list[command_id]->addNextCommand(command); }
	void addNextCommandToFlow(T_UserCommandid command_id, std::function<bool(void)> pass_condition, unsigned int limit_time, std::function<void(void)> when_pass = []() {}, std::function<void(void)> when_timeout = []() {}, unsigned int under_limit_time = 0) override {
		if (this->commandFlow_list.find(command_id) != this->commandFlow_list.end()) this->commandFlow_list[command_id]->addNextCommand(pass_condition, limit_time, when_pass, when_timeout, under_limit_time);
	}
	void addNextCommandToFlow(T_UserCommandid command_id, std::function<bool(void)> pass_condition, std::function<bool(void)> continuous_pass, std::function<void(void)> when_ok = []() {}, std::function<void(void)> when_miss = []() {}, unsigned int under_limit_time = 0) override {
		if (this->commandFlow_list.find(command_id) != this->commandFlow_list.end()) this->commandFlow_list[command_id]->addNextCommand(pass_condition, continuous_pass, when_ok, when_miss, under_limit_time);
	}
};
#pragma endregion


#pragma region Key
class Key {
private:
	unsigned int libID;
	unsigned int down_counter, re_down_time;
	unsigned int up_counter, re_up_time;

public:
	Key(unsigned int libID) { this->libID = libID; this->initialize(); }
	void initialize();
	void update_on(bool is_on);
	unsigned int getLibID() { return this->libID; }

	bool isDown() { return this->down_counter > 0; }
	bool isUp() { return this->up_counter > 0; }
	bool isDownOnce() { return this->down_counter == 1; }
	bool isUpOnce() { return this->up_counter == 1; }
	bool isKeepDown(unsigned int time_min) { return this->down_counter >= time_min; }
	bool isKeepUp(unsigned int time_min) { return this->up_counter >= time_min; }
	bool isKeepDownOnce(unsigned int time) { return this->down_counter == time; }
	bool isKeepUpOnce(unsigned int time) { return this->up_counter == time; }
	bool isReDownInTime(unsigned int allow_time) { return ((this->down_counter == 1) && (this->re_down_time <= allow_time)); }//キーを離してから時間内に再び押す
	bool isReUpInTime(unsigned int allow_time) { return ((this->up_counter == 1) && (this->re_up_time <= allow_time)); }//キーを押してから時間内に離す
	bool isReDownWithinTime(unsigned int min_time, unsigned int max_time) { return ((this->down_counter == 1) && (this->re_down_time >= min_time) && (this->re_down_time <= max_time)); }
	bool isReUpWithinTime(unsigned int min_time, unsigned int max_time) { return ((this->up_counter == 1) && (this->re_up_time >= min_time) && (this->re_up_time <= min_time)); }
	bool isDoubleDownInTime(unsigned int allow_time) { return ((this->down_counter == 1) && (this->re_down_time + this->re_up_time <= allow_time)); }//キーが上がった状態から、時間内に二度押す
	bool isDoubleUpInTime(unsigned int allow_time) { return ((this->up_counter == 1) && (this->re_down_time + this->re_up_time <= allow_time)); }//キーが下がった状態から、時間内に二度離す
	bool isDoubleDownWithinTime(unsigned int min_time, unsigned int max_time) { return ((this->down_counter == 1) && (this->re_down_time + this->re_up_time >= min_time) && (this->re_down_time + this->re_up_time <= max_time)); }
	bool isDoubleUpWithinTime(unsigned int min_time, unsigned int max_time) { return ((this->up_counter == 1) && (this->re_down_time + this->re_up_time >= min_time) && (this->re_down_time + this->re_up_time <= max_time)); }
};

template <typename T_UserKeyID>
class KeyManager {
private:
	std::unordered_map<T_UserKeyID, std::shared_ptr<Key>> check_keys;
	bool accept_update_input, return_all_false;

	std::vector<T_UserKeyID> downed_keys_pre, downed_keys;//キー同時押し判定用
	unsigned int downed_time, time_counter;

	std::function<bool(unsigned int)> keyCheck;

public:
	KeyManager(std::function<bool(unsigned int)> keyCheck) { this->keyCheck = keyCheck; this->initialize(); }
	virtual ~KeyManager() {}
	void initialize() { this->accept_update_input = true; this->return_all_false = false; this->downed_time = 10000; this->time_counter = 0; this->check_keys.clear(); this->downed_keys.clear(); this->downed_keys_pre.clear(); }
	void update();
	void updateFromLog(std::function<bool(T_UserKeyID)> keyCheck_log);
	void acceptUpdateInput(bool accept) { this->accept_update_input = accept; }
	void setReturnAllKeyFalse(bool all_false) { this->return_all_false = all_false; }

	auto getCheckingKeysListBegin() { return this->check_keys.begin(); }
	auto getCheckingKeysListEnd() { return this->check_keys.end(); }
	unsigned int getLibID(T_UserKeyID key_id) { return this->check_keys[key_id]->getLibID(); }

	void addKeyListener(T_UserKeyID code, unsigned int libID) { this->check_keys[code] = std::make_shared<Key>(libID); }
	void removeKeyListener() { this->check_keys.clear(); }
	void removeKeyListener(T_UserKeyID code) { this->check_keys.erase(code); }

	bool isDown(T_UserKeyID code) { if (this->return_all_false) return false; return this->check_keys[code]->isDown(); }
	bool isUp(T_UserKeyID code) { if (this->return_all_false) return false; else return this->check_keys[code]->isUp(); }
	bool isDownOnce(T_UserKeyID code) { if (this->return_all_false) return false; else return this->check_keys[code]->isDownOnce(); }
	bool isUpOnce(T_UserKeyID code) { if (this->return_all_false) return false; else return this->check_keys[code]->isUpOnce(); }
	bool isKeepDown(T_UserKeyID code, unsigned int time_min) { if (this->return_all_false) return false; else return this->check_keys[code]->isKeepDown(time_min); }
	bool isKeepUp(T_UserKeyID code, unsigned int time_min) { if (this->return_all_false) return false; else return this->check_keys[code]->isKeepUp(time_min); }
	bool isKeepDownOnce(T_UserKeyID code, unsigned int time) { if (this->return_all_false) return false; else return this->check_keys[code]->isKeepDownOnce(time); }
	bool isKeepUpOnce(T_UserKeyID code, unsigned int time) { if (this->return_all_false) return false; else return this->check_keys[code]->isKeepUpOnce(time); }
	bool isReDownInTime(T_UserKeyID code, unsigned int allow_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isReDownInTime(allow_time); }
	bool isReUpInTime(T_UserKeyID code, unsigned int allow_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isReUpInTime(allow_time); }
	bool isReDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isReDownWithinTime(min_time, max_time); }
	bool isReUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isReUpWithinTime(min_time, max_time); }
	bool isDoubleDownInTime(T_UserKeyID code, unsigned int allow_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isDoubleDownInTime(allow_time); }
	bool isDoubleUpInTime(T_UserKeyID code, unsigned int allow_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isDoubleUpInTime(allow_time); }
	bool isDoubleDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isDoubleDownWithinTime(min_time, max_time); }
	bool isDoubleUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) { if (this->return_all_false) return false; else return this->check_keys[code]->isDoubleUpWithinTime(min_time, max_time); }

	bool isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time);
	bool isDownOnceWhileDown(T_UserKeyID down_once, T_UserKeyID while_down) { if (down_once == while_down) return false; return (this->isDown(while_down)) && (this->isDownOnce(down_once)); }
	bool isUpOnceWhileDown(T_UserKeyID up_once, T_UserKeyID while_down) { if (up_once == while_down) return false; return (this->isDown(while_down)) && (this->isUpOnce(up_once)); }
	bool isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time);
	bool isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time);
};
#pragma endregion


#pragma region KeyBoard
template <typename T_UserKeyID>
class KeyBoardOperationInterface {
public:
	virtual ~KeyBoardOperationInterface() {}
	virtual void addKeyListener(T_UserKeyID keyID, unsigned int libID) = 0;
	virtual void removeKeyListener() = 0;
	virtual void removeKeyListener(T_UserKeyID keyID) = 0;
};

template <typename T_UserKeyID>
class KeyBoardEventInterface {
public:
	virtual ~KeyBoardEventInterface() {}

	virtual bool isDown(T_UserKeyID code) = 0;
	virtual bool isUp(T_UserKeyID code) = 0;
	virtual bool isDownOnce(T_UserKeyID code) = 0;
	virtual bool isUpOnce(T_UserKeyID code) = 0;
	virtual bool isKeepDown(T_UserKeyID code, unsigned int time_min) = 0;
	virtual bool isKeepUp(T_UserKeyID code, unsigned int time_min) = 0;
	virtual bool isKeepDownOnce(T_UserKeyID code, unsigned int time) = 0;
	virtual bool isKeepUpOnce(T_UserKeyID code, unsigned int time) = 0;
	virtual bool isReDownInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isReUpInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isReDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isReUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isDoubleDownInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isDoubleUpInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isDoubleDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isDoubleUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;

	virtual bool isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) = 0;//key1とkey2が時間内に同時押し(順不同)
	virtual bool isDownOnceWhileDown(T_UserKeyID down_once, T_UserKeyID while_down) = 0;//while_downが押されている間にdown_onceが押される
	virtual bool isUpOnceWhileDown(T_UserKeyID up_once, T_UserKeyID while_down) = 0;//while_downが押されている間にdown_onceが離される
	virtual bool isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) = 0;//key1が離されてから時間内にkey2が押される
	virtual bool isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time) = 0;
};

template <typename T_LogID, typename T_UserKeyID>
class KeyBoardLoggingInterface {
public:
	virtual ~KeyBoardLoggingInterface() {}

	virtual void startLogRecordNow(T_LogID log_id) = 0;
	virtual void endLogRecord(T_LogID log_id) = 0;
	virtual void removeLogRecord(T_LogID log_id) = 0;
	virtual bool isLogRecordingNow(T_LogID log_id) = 0;

	virtual unsigned int getLogTimeMax(T_LogID log_id) = 0;
	virtual void initReplay(T_LogID log_id) = 0;
	virtual void updateReplayTimeCount(T_LogID log_id) = 0;
	virtual std::shared_ptr<KeyBoardEventInterface<T_UserKeyID>> getLogEventInterface(T_LogID log_id) = 0;
};

template<typename T_UserKeyID, typename T_UserCommandID>
class KeyBoard : public KeyBoardOperationInterface<T_UserKeyID>, public KeyBoardEventInterface<T_UserKeyID>, public std::enable_shared_from_this<KeyBoard<T_UserKeyID, T_UserCommandID>> {
private:
	std::shared_ptr<KeyManager<T_UserKeyID>> key_mgr;
	std::shared_ptr<CommandManager<T_UserCommandID>> command_mgr;

public:
	KeyBoard(std::function<bool(unsigned int)> isDown) { this->key_mgr = std::make_shared<KeyManager<T_UserKeyID>>(isDown); this->command_mgr = std::make_shared<CommandManager<T_UserCommandID>>(); this->initialize(); }
	void initialize() { this->acceptUpdateInput(true); this->setReturnAllKeyFalse(false); this->key_mgr->initialize(); this->command_mgr->initialize(); }
	void update() { this->key_mgr->update(); this->command_mgr->update(); }
	void updateFromLog(std::function<bool(T_UserKeyID)> keyCheck_log) { this->key_mgr->updateFromLog(keyCheck_log); this->command_mgr->update(); }
	std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>> clone() { return std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>>(new KeyBoard<T_UserKeyID, T_UserCommandID>(*this)); }

	void acceptUpdateInput(bool accept) { this->key_mgr->acceptUpdateInput(accept); this->command_mgr->acceptUpdateInput(accept); }
	void setReturnAllKeyFalse(bool all_false) { this->key_mgr->setReturnAllKeyFalse(all_false); }
	unsigned int getLibID(T_UserKeyID key_id) { return this->key_mgr->getLibID(key_id); }

	std::shared_ptr<KeyBoardOperationInterface<T_UserKeyID>> getOperationInterface() { return this->shared_from_this(); }
	std::shared_ptr<KeyBoardEventInterface<T_UserKeyID>> getEventInterface() { return this->shared_from_this(); }
	std::shared_ptr<CommandOperationInterface<T_UserCommandID>> getCommandOperationInterface() { return this->command_mgr; }

	
	void addKeyListener(T_UserKeyID code, unsigned int libID) override { this->key_mgr->addKeyListener(code, libID); }
	void removeKeyListener() override { this->key_mgr->removeKeyListener(); }
	void removeKeyListener(T_UserKeyID code) override { this->key_mgr->removeKeyListener(code); }

	bool isDown(T_UserKeyID code) override { return this->key_mgr->isDown(code); }
	bool isUp(T_UserKeyID code) override { return this->key_mgr->isUp(code); }
	bool isDownOnce(T_UserKeyID code) override { return this->key_mgr->isDownOnce(code); }
	bool isUpOnce(T_UserKeyID code) override { return this->key_mgr->isUpOnce(code); }
	bool isKeepDown(T_UserKeyID code, unsigned int time_min) override { return this->key_mgr->isKeepDown(code, time_min); }
	bool isKeepUp(T_UserKeyID code, unsigned int time_min) override { return this->key_mgr->isKeepUp(code, time_min); }
	bool isKeepDownOnce(T_UserKeyID code, unsigned int time) override { return this->key_mgr->isKeepDownOnce(code, time); }
	bool isKeepUpOnce(T_UserKeyID code, unsigned int time) override { return this->key_mgr->isKeepUpOnce(code, time); }
	bool isReDownInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isReDownInTime(code, allow_time); }
	bool isReUpInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isReUpInTime(code, allow_time); }
	bool isReDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isReDownWithinTime(code, min_time, max_time); }
	bool isReUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isReUpWithinTime(code, min_time, max_time); }
	bool isDoubleDownInTime(T_UserKeyID code, unsigned int allow_time)override { return this->key_mgr->isDoubleDownInTime(code, allow_time); }
	bool isDoubleUpInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isDoubleUpInTime(code, allow_time); }
	bool isDoubleDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDoubleDownWithinTime(code, min_time, max_time); }
	bool isDoubleUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDoubleUpWithinTime(code, min_time, max_time); }

	bool isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) override { return this->key_mgr->isSimultanouesDownOnce(code1, code2, allow_time); }
	bool isDownOnceWhileDown(T_UserKeyID down_once, T_UserKeyID while_down) override { return this->key_mgr->isDownOnceWhileDown(down_once, while_down); }
	bool isUpOnceWhileDown(T_UserKeyID up_once, T_UserKeyID while_down) override { return this->key_mgr->isUpOnceWhileDown(up_once, while_down); }
	bool isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) override { return this->key_mgr->isDownOnceNextDownOnceInTime(code1, code2, allow_time); }
	bool isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDownOnceNextDownOnceWithinTime(code1, code2, min_time, max_time); }
};

template<typename T_UserKeyID, typename T_UserCommandID>
class KeyBoardLog {
private:
	bool is_logging;
	unsigned int log_counter;
	const std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>> origin;
	std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>> log;
	std::unordered_map<T_UserKeyID, std::vector<bool>> onoff_log;

	std::function<bool(unsigned int)> isDown;

public:
	KeyBoardLog(std::function<bool(unsigned int)> isDown, std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>> origin) : origin(origin) { this->isDown = isDown; }
	void pushLog() { for (auto it = this->onoff_log.begin(); it != this->onoff_log.end(); ++it) { it->second.push_back(this->isDown(this->origin->getLibID(it->first))); } }
	void endLogging() { this->is_logging = false; }

	void initReplay() { if (this->is_logging) return; this->log = this->origin->clone(); this->log_counter = 0; }
	void updateReplayTimeCount() { if (this->is_logging) return; if (this->log_counter >= this->getLoggedTimeMax()) return; this->log->updateFromLog([this](T_UserKeyID id) {return onoff_log[id][log_counter]; }); this->log_counter++; }

	bool isLoggingNow() { return this->is_logging; }
	unsigned int getLoggedTimeMax() { return this->onoff_log.size(); }

	std::shared_ptr<KeyBoardEventInterface<T_UserKeyID>> getLogEventInterface() { return this->log; }
};

template<typename T_UserKeyID, typename T_UserCommandID, typename T_LogID>
class KeyBoardLogs : public KeyBoardLoggingInterface<T_LogID, T_UserKeyID> {
private:
	std::unordered_map<T_LogID, std::shared_ptr<KeyBoardLog<T_UserKeyID, T_UserCommandID>>> logs;
	std::function<bool(unsigned int)> isDown;
	std::function<std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>>(void)> getKeyBoardNowClone;

public:
	KeyBoardLogs(std::function<bool(unsigned int)> isDown, std::function<std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>>(void)> getKeyBoardNowClone) { this->isDown = isDown; this->getKeyBoardNowClone = getKeyBoardNowClone; }
	void initialize() { this->logs.clear(); }
	void update() { for (auto it = this->logs.begin(); it != this->logs.end(); ++it) { if (!it->second->isLoggingNow()) continue; it->second->pushLog(); } }

	void startLogRecordNow(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id] = std::make_shared<KeyBoardLog<T_UserKeyID, T_UserCommandID>>(this->isDown, this->getKeyBoardNowClone()); }
	void endLogRecord(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->endLogging(); }
	void removeLogRecord(T_LogID log_id) override { this->logs.erase(log_id); }
	bool isLogRecordingNow(T_LogID log_id)  override { if (this->logs.find(log_id) != this->logs.end()) return false; return this->logs[log_id]->isLoggingNow(); }
	
	unsigned int getLogTimeMax(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return 0; return this->logs[log_id]->getLoggedTimeMax(); }
	void initReplay(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->initReplay(); }
	void updateReplayTimeCount(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->updateReplayTimeCount(); }
	std::shared_ptr<KeyBoardEventInterface<T_UserKeyID>> getLogEventInterface(T_LogID log_id) override { return this->logs[log_id]->getLogEventInterface(); }
};

template<typename T_UserKeyID, typename T_UserCommandID, typename T_LogID>
class KeyBoardManager {
private:
	std::shared_ptr<KeyBoard<T_UserKeyID, T_UserCommandID>> now;
	std::shared_ptr<KeyBoardLogs<T_UserKeyID, T_UserCommandID, T_LogID>> logs;
	char key_now[256];

	bool isDown(unsigned int lib_key) { return this->key_now[lib_key] != 0; }

public:
	KeyBoardManager() { this->now = std::make_shared<KeyBoard<T_UserKeyID, T_UserCommandID>>([this](unsigned int lib_id) {return this->isDown(lib_id); }); this->logs = std::make_shared<KeyBoardLogs<T_UserKeyID, T_UserCommandID, T_LogID>>([this](unsigned int lib_id) {return this->isDown(lib_id); }, [this]() {return now->clone(); }); this->initialize(); }
	void initialize() { for (int i = 0; i < 256; ++i) this->key_now[i] = 0; this->now->initialize(); this->logs->initialize(); }
	void update() { GetHitKeyStateAll(this->key_now); this->now->update(); this->logs->update(); }

	std::shared_ptr<KeyBoardOperationInterface<T_UserKeyID>> getOperationInterface() { return this->now->getOperationInterface(); }
	std::shared_ptr<KeyBoardEventInterface<T_UserKeyID>> getEventInterface() { return this->now->getEventInterface(); }
	std::shared_ptr<CommandOperationInterface<T_UserKeyID>> getCommandOperationInterface() { return this->now->getCommandOperationInterface(); }
	std::shared_ptr<KeyBoardLoggingInterface<T_LogID, T_UserKeyID>> getLoggingInterface() { return this->logs; }

	void setReturnAllKeyFalse(bool readable) { this->now->setReturnAllKeyFalse(readable); }
	void acceptUpdateInput(bool accept) { this->now->acceptUpdateInput(accept); }
};
#pragma endregion



#pragma region Mouse
template<typename T_UserKeyID>
class MouseOperationInterface {
public:
	virtual ~MouseOperationInterface() {}

	virtual void addKeyListener(T_UserKeyID keyID, unsigned int libID) = 0;
	virtual void removeKeyListener() = 0;
	virtual void removeKeyListener(T_UserKeyID keyID) = 0;

	virtual Vec2D getPointerPosition() = 0;
	virtual void setPointerPosition(Vec2D pos) = 0;
	virtual void hidePointer() = 0;
	virtual void showPointer() = 0;
	virtual int getWheelRotationDelta() = 0;
};

template <typename T_UserKeyID>
class MouseEventInterface {
public:
	virtual ~MouseEventInterface() {}

	virtual bool isDown(T_UserKeyID code) = 0;
	virtual bool isUp(T_UserKeyID code) = 0;
	virtual bool isDownOnce(T_UserKeyID code) = 0;
	virtual bool isUpOnce(T_UserKeyID code) = 0;
	virtual bool isKeepDown(T_UserKeyID code, unsigned int time_min) = 0;
	virtual bool isKeepUp(T_UserKeyID code, unsigned int time_min) = 0;
	virtual bool isKeepDownOnce(T_UserKeyID code, unsigned int time) = 0;
	virtual bool isKeepUpOnce(T_UserKeyID code, unsigned int time) = 0;
	virtual bool isReDownInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isReUpInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isReDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isReUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isDoubleDownInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isDoubleUpInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isDoubleDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isDoubleUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;

	virtual bool isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) = 0;//key1とkey2が時間内に同時押し(順不同)
	virtual bool isDownOnceWhileDown(T_UserKeyID down_once, T_UserKeyID while_down) = 0;//while_downが押されている間にdown_onceが押される
	virtual bool isUpOnceWhileDown(T_UserKeyID up_once, T_UserKeyID while_down) = 0;//while_downが押されている間にdown_onceが離される
	virtual bool isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) = 0;//key1が離されてから時間内にkey2が押される
	virtual bool isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time) = 0;

	virtual bool isMoved() = 0;//ポインタが動いたか
	virtual bool isInSquare(Vec2D leftup_window, Vec2D rightdown_window) = 0;//ポインタが領域内にあるか
	virtual bool isInCircle(Vec2D center_window, double r) = 0;
	virtual bool isCrossIntoSquare(Vec2D leftup_window, Vec2D rightdown_window) = 0;//ポインタが領域内に入った瞬間か
	virtual bool isCrossIntoCircle(Vec2D center_window, double r) = 0;
	virtual bool isCrossOuttoSquare(Vec2D leftup_window, Vec2D rightdown_window) = 0;//ポインタが領域外に出た瞬間か
	virtual bool isCrossOutoCircle(Vec2D center_window, double r) = 0;

	virtual bool isDownInSquare(T_UserKeyID id, Vec2D leftup_window, Vec2D rightdown_window) = 0;//領域内でキーが下がっているか
	virtual bool isDownInCircle(T_UserKeyID id, Vec2D center_window, double r) = 0;
	virtual bool isDownOnceInSquare(T_UserKeyID id, Vec2D leftup_window, Vec2D rightdown_window) = 0;//領域内でキーが押されたか
	virtual bool isDownOnceInCircle(T_UserKeyID id, Vec2D center_window, double r) = 0;
	virtual bool isUpOnceInSquare(T_UserKeyID id, Vec2D leftup_window, Vec2D rightdown_window) = 0;//領域内でキーが離されたか
	virtual bool isUpOnceInCircle(T_UserKeyID id, Vec2D center_window, double r) = 0;
	virtual bool isDoubleDownInTimeInSquare(T_UserKeyID id, unsigned int allow_time, Vec2D leftup_window, Vec2D rightdown_window) = 0;//領域内でキーが二回押されたか
	virtual bool isDoubleDownInTimeInCircle(T_UserKeyID id, unsigned int allow_time, Vec2D center_window, double r) = 0;
};

template <typename T_LogID, typename T_UserKeyID>
class MouseLoggingInterface {
public:
	virtual ~MouseLoggingInterface() {}

	virtual void startLogRecordNow(T_LogID log_id) = 0;
	virtual void endLogRecord(T_LogID log_id) = 0;
	virtual void removeLogRecord(T_LogID log_id) = 0;
	virtual bool isLogRecordingNow(T_LogID log_id) = 0;

	virtual unsigned int getLogTimeMax(T_LogID log_id) = 0;
	virtual void initReplay(T_LogID log_id) = 0;
	virtual void updateReplayTimeCount(T_LogID log_id) = 0;
	virtual std::shared_ptr<MouseEventInterface<T_UserKeyID>> getLogEventInterface(T_LogID log_id) = 0;
};

template<typename T_UserKeyID>
class MousePointerManager {
private:
	bool showing_pointer;//os標準のポインタを表示するか
	Vec2D pointer_position_now, pointer_position_pre;
	Vec2D pointer_pos_storage;//ポインタ座標復元用保存先
	int wheel_rotation;

	bool accept_update_input, return_all_false;

	std::function<Vec2D(void)> get_pointer_position;
	std::function<int(void)> get_wheel_rotation;

public:
	MousePointerManager(std::function<Vec2D(void)> getPointerPosition, std::function<int(void)> getWheelRotation) { this->get_pointer_position = getPointerPosition; this->get_wheel_rotation = getWheelRotation; this->initialize(); }
	void initialize() { this->accept_update_input = true; this->return_all_false = false; this->showing_pointer = false; this->pointer_position_now = Vec2D(); this->pointer_position_pre = Vec2D(); this->wheel_rotation = 0; }
	void update() { if (!this->accept_update_input) return; this->pointer_position_pre = this->pointer_position_now; this->pointer_position_now = this->get_pointer_position(); this->wheel_rotation += get_wheel_rotation(); }
	void updateFromLog(std::function<Vec2D(void)> pointerPos_log, std::function<int(void)> wheelRotate_log){ if (!this->accept_update_input) return; this->pointer_position_pre = this->pointer_position_now; this->pointer_position_now = pointerPos_log(); this->wheel_rotation += wheelRotate_log(); }
	void acceptUpdateInput(bool accept) { this->accept_update_input = accept; }
	void setReturnAllKeyFalse(bool all_false) { this->return_all_false = all_false; }

	Vec2D getPointerPosition() { if (this->return_all_false) return Vec2D(); return this->pointer_position_now; }
	void setPointerPosition(Vec2D pos) { SetMousePoint((int)(pos.x), (int)(pos.y)); }
	void hidePointer() { this->showing_pointer = false; this->pointer_pos_storage = this->pointer_position_now; SetMouseDispFlag(false); }
	void showPointer() { this->showing_pointer = true; this->pointer_position_now = this->pointer_pos_storage; SetMouseDispFlag(true); SetMousePoint((int)this->pointer_position_now.x, (int)this->pointer_position_now.y); }
	int getWheelRotationDelta() { if (this->return_all_false) return 0; int val = this->wheel_rotation; this->wheel_rotation = 0; return val; }

	bool isMoved() { if (this->return_all_false) return false; return this->pointer_position_now != this->pointer_position_pre; }
	bool isInSquare(Vec2D leftup_window, Vec2D rightdown_window) { if (this->return_all_false) return false; return Vec2D::isInSquare(leftup_window, rightdown_window, this->pointer_position_now); }
	bool isInCircle(Vec2D center_window, double r) { if (this->return_all_false) return false; return (this->pointer_position_now - center_window).toNorm() < r; }
	bool isCrossIntoSquare(Vec2D leftup_window, Vec2D rightdown_window) { if (this->return_all_false) return false; return ((Vec2D::isInSquare(leftup_window, rightdown_window, this->pointer_position_now)) && (!Vec2D::isInSquare(leftup_window, rightdown_window, this->pointer_position_pre))); }
	bool isCrossIntoCircle(Vec2D center_window, double r) { if (this->return_all_false) return false; return (((this->pointer_position_now - center_window).toNorm() <= r) && ((this->pointer_position_pre - center_window).toNorm() > r)); }
	bool isCrossOuttoSquare(Vec2D leftup_window, Vec2D rightdown_window) { if (this->return_all_false) return false; return ((!Vec2D::isInSquare(leftup_window, rightdown_window, this->pointer_position_now)) && (Vec2D::isInSquare(leftup_window, rightdown_window, this->pointer_position_pre))); }
	bool isCrossOutoCircle(Vec2D center_window, double r) { if (this->return_all_false) return false; return (((this->pointer_position_now - center_window).toNorm() > r) && ((this->pointer_position_pre - center_window).toNorm() <= r)); }
};

template<typename T_UserKeyID, typename T_UserCommandID>
class Mouse : public MouseOperationInterface<T_UserKeyID>, public MouseEventInterface<T_UserKeyID>, public std::enable_shared_from_this<Mouse<T_UserKeyID, T_UserCommandID>> {
private:
	std::shared_ptr<KeyManager<T_UserKeyID>> key_mgr;
	std::shared_ptr<CommandManager<T_UserCommandID>> command_mgr;
	std::shared_ptr<MousePointerManager<T_UserKeyID>> pointer_mgr;

public:
	Mouse(std::function<bool(unsigned int)> isDown, std::function<Vec2D(void)> getPointerPosition, std::function<int(void)> getWheelRotation) {
		this->key_mgr = std::make_shared<KeyManager<T_UserKeyID>>(isDown); this->command_mgr = std::make_shared<CommandManager<T_UserCommandID>>(); this->pointer_mgr = std::make_shared<MousePointerManager<T_UserKeyID>>(getPointerPosition, getWheelRotation); this->initialize();
	}
	void initialize() { this->acceptUpdateInput(true); this->setReturnAllKeyFalse(false); this->key_mgr->initialize(); this->command_mgr->initialize(); this->pointer_mgr->initialize(); }
	void update() { this->key_mgr->update(); this->pointer_mgr->update(); this->command_mgr->update(); }
	void updateFromLog(std::function<bool(T_UserKeyID)> keyCheck_log, std::function<Vec2D(void)> pointerPos_log, std::function<int(void)> wheelRotate_log) { this->key_mgr->updateFromLog(keyCheck_log); this->pointer_mgr->updateFromLog(pointerPos_log, wheelRotate_log); this->command_mgr->update(); }
	std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>> clone() { return std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>>(new Mouse<T_UserKeyID, T_UserCommandID>(*this)); }

	void acceptUpdateInput(bool accept) { this->key_mgr->acceptUpdateInput(accept); this->command_mgr->acceptUpdateInput(accept); }
	void setReturnAllKeyFalse(bool all_false) { this->key_mgr->setReturnAllKeyFalse(all_false); }
	unsigned int getLibID(T_UserKeyID key_id) { return this->key_mgr->getLibID(key_id); }

	std::shared_ptr<MouseOperationInterface<T_UserKeyID>> getOperationInterface() { return this->shared_from_this(); }
	std::shared_ptr<MouseEventInterface<T_UserKeyID>> getEventInterface() { return this->shared_from_this(); }
	std::shared_ptr<CommandOperationInterface<T_UserCommandID>> getCommandOperationInterface() { return this->command_mgr; }


	void addKeyListener(T_UserKeyID code, unsigned int libID) override { this->key_mgr->addKeyListener(code, libID); }
	void removeKeyListener() override { this->key_mgr->removeKeyListener(); }
	void removeKeyListener(T_UserKeyID code) override { this->key_mgr->removeKeyListener(code); }

	Vec2D getPointerPosition() override { return this->pointer_mgr->getPointerPosition(); }
	void setPointerPosition(Vec2D pos) override { this->pointer_mgr->setPointerPosition(pos); }
	void hidePointer() override { this->pointer_mgr->hidePointer(); }
	void showPointer() override { this->pointer_mgr->showPointer(); }
	int getWheelRotationDelta() override { return this->pointer_mgr->getWheelRotationDelta(); }

	bool isDown(T_UserKeyID code) override { return this->key_mgr->isDown(code); }
	bool isUp(T_UserKeyID code) override { return this->key_mgr->isUp(code); }
	bool isDownOnce(T_UserKeyID code) override { return this->key_mgr->isDownOnce(code); }
	bool isUpOnce(T_UserKeyID code) override { return this->key_mgr->isUpOnce(code); }
	bool isKeepDown(T_UserKeyID code, unsigned int time_min) override { return this->key_mgr->isKeepDown(code, time_min); }
	bool isKeepUp(T_UserKeyID code, unsigned int time_min) override { return this->key_mgr->isKeepUp(code, time_min); }
	bool isKeepDownOnce(T_UserKeyID code, unsigned int time) override { return this->key_mgr->isKeepDownOnce(code, time); }
	bool isKeepUpOnce(T_UserKeyID code, unsigned int time) override { return this->key_mgr->isKeepUpOnce(code, time); }
	bool isReDownInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isReDownInTime(code, allow_time); }
	bool isReUpInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isReUpInTime(code, allow_time); }
	bool isReDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isReDownWithinTime(code, min_time, max_time); }
	bool isReUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isReUpWithinTime(code, min_time, max_time); }
	bool isDoubleDownInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isDoubleDownInTime(code, allow_time); }
	bool isDoubleUpInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isDoubleUpInTime(code, allow_time); }
	bool isDoubleDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDoubleDownWithinTime(code, min_time, max_time); }
	bool isDoubleUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDoubleUpWithinTime(code, min_time, max_time); }

	bool isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) override { return this->key_mgr->isSimultanouesDownOnce(code1, code2, allow_time); }
	bool isDownOnceWhileDown(T_UserKeyID down_once, T_UserKeyID while_down) override { return this->key_mgr->isDownOnceWhileDown(down_once, while_down); }
	bool isUpOnceWhileDown(T_UserKeyID up_once, T_UserKeyID while_down) override { return this->key_mgr->isUpOnceWhileDown(up_once, while_down); }
	bool isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) override { return this->key_mgr->isDownOnceNextDownOnceInTime(code1, code2, allow_time); }
	bool isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDownOnceNextDownOnceWithinTime(code1, code2, min_time, max_time); }

	bool isMoved() override { return this->pointer_mgr->isMoved(); }
	bool isInSquare(Vec2D leftup_window, Vec2D rightdown_window) override { return this->pointer_mgr->isInSquare(leftup_window, rightdown_window); }
	bool isInCircle(Vec2D center_window, double r) override { return this->pointer_mgr->isInCircle(center_window, r); }
	bool isCrossIntoSquare(Vec2D leftup_window, Vec2D rightdown_window) override { return this->pointer_mgr->isCrossIntoSquare(leftup_window, rightdown_window); }
	bool isCrossIntoCircle(Vec2D center_window, double r) override { return this->pointer_mgr->isCrossIntoCircle(center_window, r); }
	bool isCrossOuttoSquare(Vec2D leftup_window, Vec2D rightdown_window) override { return this->pointer_mgr->isCrossOuttoSquare(leftup_window, rightdown_window); }
	bool isCrossOutoCircle(Vec2D center_window, double r) override { return this->pointer_mgr->isCrossOutoCircle(center_window, r); }

	bool isDownInSquare(T_UserKeyID id, Vec2D leftup_window, Vec2D rightdown_window) override { return (this->key_mgr->isDown(id)) && (this->pointer_mgr->isInSquare(leftup_window, rightdown_window)); }
	bool isDownInCircle(T_UserKeyID id, Vec2D center_window, double r) override { return (this->key_mgr->isDown(id)) && (this->pointer_mgr->isInCircle(center_window, r)); }
	bool isDownOnceInSquare(T_UserKeyID id, Vec2D leftup_window, Vec2D rightdown_window) override { return (this->key_mgr->isDownOnce(id)) && (this->pointer_mgr->isInSquare(leftup_window, rightdown_window)); }
	bool isDownOnceInCircle(T_UserKeyID id, Vec2D center_window, double r) override { return (this->key_mgr->isDownOnce(id)) && (this->pointer_mgr->isInCircle(center_window, r)); }
	bool isUpOnceInSquare(T_UserKeyID id, Vec2D leftup_window, Vec2D rightdown_window) override { return (this->key_mgr->isUpOnce(id)) && (this->pointer_mgr->isInSquare(leftup_window, rightdown_window)); }
	bool isUpOnceInCircle(T_UserKeyID id, Vec2D center_window, double r) override { return (this->key_mgr->isUpOnce(id)) && (this->pointer_mgr->isInCircle(center_window, r)); }
	bool isDoubleDownInTimeInSquare(T_UserKeyID id, unsigned int allow_time, Vec2D leftup_window, Vec2D rightdown_window) override { return (this->key_mgr->isDoubleDownInTime(id, allow_time)) && (this->pointer_mgr->isInSquare(leftup_window, rightdown_window)); }
	bool isDoubleDownInTimeInCircle(T_UserKeyID id, unsigned int allow_time, Vec2D center_window, double r) override { return (this->key_mgr->isDoubleDownInTime(id, allow_time)) && (this->pointer_mgr->isInCircle(center_window, r)); }
};

template<typename T_UserKeyID, typename T_UserCommandID>
class MouseLog {
private:
	bool is_logging;
	unsigned int log_counter;
	const std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>> origin;
	std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>> log;
	std::unordered_map<T_UserKeyID, std::vector<bool>> onoff_log;
	std::vector<Vec2D> mouse_pointer_log;
	std::vector<int> mouse_wheel_rotation_log;

	std::function<bool(unsigned int)> isDown;
	std::function<Vec2D(void)> getPointerPosition;
	std::function<int(void)> getWheelRotation;

public:
	MouseLog(std::function<bool(unsigned int)> isDown, std::function<Vec2D(void)> getPointerPosition, std::function<int(void)> getWheelRotation, std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>> origin) : origin(origin) { this->isDown = isDown; this->getPointerPosition = getPointerPosition; this->getWheelRotation = getWheelRotation; }
	void pushLog() { for (auto it = this->onoff_log.begin(); it != this->onoff_log.end(); ++it) { it->second.push_back(this->isDown(this->origin->getLibID(it->first))); } this->mouse_pointer_log.push_back(this->getPointerPosition()); this->mouse_wheel_rotation_log.push_back(this->getWheelRotation()); }
	void endLogging() { this->is_logging = false; }

	void initReplay() { if (this->is_logging) return; this->log = this->origin->clone(); this->log_counter = 0; }
	void updateReplayTimeCount() { if (this->is_logging) return; if (this->log_counter >= this->getLoggedTimeMax()) return; this->log->updateFromLog([this](T_UserKeyID id) {return onoff_log[id][log_counter]; }, [this]() {return mouse_pointer_log[log_counter]; }, [this]() {return mouse_wheel_rotation_log[log_counter]; }); this->log_counter++; }

	bool isLoggingNow() { return this->is_logging; }
	unsigned int getLoggedTimeMax() { return this->onoff_log.size(); }

	std::shared_ptr<MouseEventInterface<T_UserKeyID>> getLogEventInterface() { return this->log; }
};

template<typename T_UserKeyID, typename T_UserCommandID, typename T_LogID>
class MouseLogs : public MouseLoggingInterface<T_LogID, T_UserKeyID> {
private:
	std::unordered_map<T_LogID, std::shared_ptr<MouseLog<T_UserKeyID, T_UserCommandID>>> logs;
	std::function<bool(unsigned int)> isDown;
	std::function<Vec2D(void)> getPointerPosition;
	std::function<int(void)> getWheelRotation;
	std::function<std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>>(void)> getMouseNowClone;

public:
	MouseLogs(std::function<bool(unsigned int)> isDown, std::function<Vec2D(void)> getPointerPosition, std::function<int(void)> getWheelRotation, std::function<std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>>(void)> getMouseNowClone) { this->isDown = isDown; this->getPointerPosition = getPointerPosition; this->getWheelRotation = getWheelRotation; this->getMouseNowClone = getMouseNowClone; }
	void initialize() { this->logs.clear(); }
	void update() { for (auto it = this->logs.begin(); it != this->logs.end(); ++it) { if (!it->second->isLoggingNow()) continue; it->second->pushLog(); } }

	void startLogRecordNow(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id] = std::make_shared<MouseLog<T_UserKeyID, T_UserCommandID>>(this->isDown, this->getPointerPosition, this->getWheelRotation, this->getMouseNowClone()); }
	void endLogRecord(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->endLogging(); }
	void removeLogRecord(T_LogID log_id) override { this->logs.erase(log_id); }
	bool isLogRecordingNow(T_LogID log_id)  override { if (this->logs.find(log_id) != this->logs.end()) return false; return this->logs[log_id]->isLoggingNow(); }
	
	unsigned int getLogTimeMax(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return 0; return this->logs[log_id]->getLoggedTimeMax(); }
	void initReplay(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->initReplay(); }
	void updateReplayTimeCount(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->updateReplayTimeCount(); }
	std::shared_ptr<MouseEventInterface<T_UserKeyID>> getLogEventInterface(T_LogID log_id) override { return this->logs[log_id]->getLogEventInterface(); }
};

template<typename T_UserKeyID, typename T_UserCommandID, typename T_LogID>
class MouseManager {
private:
	std::shared_ptr<Mouse<T_UserKeyID, T_UserCommandID>> now;
	std::shared_ptr<MouseLogs<T_UserKeyID, T_UserCommandID, T_LogID>> logs;
	
	int mouse_input;
	Vec2D mouse_pos;
	int wheel_rotation;

	bool isDown(unsigned int lib_key) { return (this->mouse_input&lib_key) != 0; }
	Vec2D getMousePointerPosition() { return this->mouse_pos; }
	int getMouseWheelRotation() { return this->wheel_rotation; }

public:
	MouseManager() { this->now = std::make_shared<Mouse<T_UserKeyID, T_UserCommandID>>([this](unsigned int lib_id) {return this->isDown(lib_id); }, [this]() {return this->getMousePointerPosition(); }, [this]() { return this->getMouseWheelRotation(); }); this->logs = std::make_shared<MouseLogs<T_UserKeyID, T_UserCommandID, T_LogID>>([this](unsigned int lib_id) {return this->isDown(lib_id); }, [this]() {return this->getMousePointerPosition(); }, [this]() {return this->getMouseWheelRotation(); }, [this]() {return now->clone(); }); this->initialize(); }
	void initialize() { this->mouse_input = 0; this->mouse_pos = Vec2D(); this->wheel_rotation = 0; this->now->initialize(); this->logs->initialize(); }
	void update() { this->mouse_input = GetMouseInput(); this->wheel_rotation = GetMouseWheelRotVol(); int x, y; GetMousePoint(&x, &y); this->mouse_pos = Vec2D(x, y); this->now->update(); this->logs->update(); }

	std::shared_ptr<MouseOperationInterface<T_UserKeyID>> getOperationInterface() { return this->now->getOperationInterface(); }
	std::shared_ptr<MouseEventInterface<T_UserKeyID>> getEventInterface() { return this->now->getEventInterface(); }
	std::shared_ptr<CommandOperationInterface<T_UserKeyID>> getCommandOperationInterface() { return this->now->getCommandOperationInterface(); }
	std::shared_ptr<MouseLoggingInterface<T_LogID, T_UserKeyID>> getLoggingInterface() { return this->logs; }

	void setReturnAllKeyFalse(bool readable) { this->now->setReturnAllKeyFalse(readable); }
	void acceptUpdateInput(bool accept) { this->now->acceptUpdateInput(accept); }
};
#pragma endregion



#pragma region GamePad
enum class AnalogStickLeftRight {
	Left,
	Right
};

template<typename T_UserKeyID>
class GamePadOperationInterface {
public:
	virtual ~GamePadOperationInterface() {}

	virtual void addKeyListener(T_UserKeyID keyID, unsigned int libID) = 0;
	virtual void removeKeyListener() = 0;
	virtual void removeKeyListener(T_UserKeyID keyID) = 0;

	virtual Vec2D getSlope(AnalogStickLeftRight lr, double per_min = 0.0, double per_max = 1.0) = 0;//0.0~per_minの傾きを0,per_max~1.0の傾きを1としたうえで、レバーの傾きを0.0~1.0で返す
	virtual Vec2D getSlopeHighPass(AnalogStickLeftRight lr, double per_min = 0.0) = 0;//per_min以下の傾きは0,それ以外はそのまま
	virtual Direction4 getDirection4(AnalogStickLeftRight lr, double per_min = 0.0) = 0;
	virtual Direction8 getDirection8(AnalogStickLeftRight lr, double per_min = 0.0) = 0;
};

template <typename T_UserKeyID>
class GamePadEventInterface {
public:
	virtual ~GamePadEventInterface() {}

	virtual bool isDown(T_UserKeyID code) = 0;
	virtual bool isUp(T_UserKeyID code) = 0;
	virtual bool isDownOnce(T_UserKeyID code) = 0;
	virtual bool isUpOnce(T_UserKeyID code) = 0;
	virtual bool isKeepDown(T_UserKeyID code, unsigned int time_min) = 0;
	virtual bool isKeepUp(T_UserKeyID code, unsigned int time_min) = 0;
	virtual bool isKeepDownOnce(T_UserKeyID code, unsigned int time) = 0;
	virtual bool isKeepUpOnce(T_UserKeyID code, unsigned int time) = 0;
	virtual bool isReDownInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isReUpInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isReDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isReUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isDoubleDownInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isDoubleUpInTime(T_UserKeyID code, unsigned int allow_time) = 0;
	virtual bool isDoubleDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;
	virtual bool isDoubleUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) = 0;

	virtual bool isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) = 0;//key1とkey2が時間内に同時押し(順不同)
	virtual bool isDownOnceWhileDown(T_UserKeyID down_once, T_UserKeyID while_down) = 0;//while_downが押されている間にdown_onceが押される
	virtual bool isUpOnceWhileDown(T_UserKeyID up_once, T_UserKeyID while_down) = 0;//while_downが押されている間にdown_onceが離される
	virtual bool isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) = 0;//key1が離されてから時間内にkey2が押される
	virtual bool isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time) = 0;

	virtual bool isSlopeOverRange(AnalogStickLeftRight lr, double per_min) = 0;//一定の傾き以上か
	virtual bool isSlopeWithinRange(AnalogStickLeftRight lr, double per_min, double per_max) = 0;//一定の傾き範囲内か
	virtual bool isSlopeOverRangeUpToTime(AnalogStickLeftRight lr, double per_min, unsigned int time_min) = 0;//一定時間以上一定の角度以上倒し続けているか
	virtual bool isSlopeWithinRangeUpToTime(AnalogStickLeftRight lr, double per_min, double per_max, unsigned time_min) = 0;//一定時間以上傾斜角度をある範囲にとどめ続けているか
	virtual bool isSlopeCrossOuttoThresholdOnce(AnalogStickLeftRight lr, double threshold) = 0;//傾斜角が閾値を超えた瞬間か(外に向かって)
	virtual bool isSlopeCrossIntoThresholdOnce(AnalogStickLeftRight lr, double threshold) = 0;//傾斜角が閾値を超えた瞬間か(中心に向かって)
	virtual bool isInDirection(AnalogStickLeftRight lr, Direction4 dir, double per_min = 0.01, double per_max = 1.0) = 0;
	virtual bool isInDirection(AnalogStickLeftRight lr, Direction8 dir, double per_min = 0.01, double per_max = 1.0) = 0;
	virtual bool isInDirection(AnalogStickLeftRight lr, double ang1, double ang2, double per_min = 0.01, double per_max = 1.0) = 0;
	virtual bool isRoundClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time) = 0;//時計回りに一回転した瞬間か
	virtual bool isRoundCounterClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time) = 0;//反時計回りに一回転した瞬間か
	virtual bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction4 dir, unsigned int time) = 0;//一定時間内に特定の方向に倒してから戻ってきた瞬間か
	virtual bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction8 dir, unsigned int time) = 0;
	virtual bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, double ang1, double ang2, unsigned int time) = 0;

	virtual bool isKeyDownOnceWhileInDirection(T_UserKeyID key, AnalogStickLeftRight lr, Direction4 dir, double per_min = 0.01, double per_max = 1.0) = 0;//特定方向に倒している間にキーが押されたか
	virtual bool isKeyDownOnceWhileInDirection(T_UserKeyID key, AnalogStickLeftRight lr, Direction8 dir, double per_min = 0.01, double per_max = 1.0) = 0;
	virtual bool isKeyDownOnceWhileInDirection(T_UserKeyID key, AnalogStickLeftRight lr, double ang1, double ang2, double per_min = 0.01, double per_max = 1.0) = 0;
	virtual bool isKeyDownOnceWhileDownAndInDirection(T_UserKeyID down_once, T_UserKeyID while_down, AnalogStickLeftRight lr, Direction4 dir, double per_min = 0.01, double per_max = 1.0) = 0;//特定方向に倒している間にキーが押されたか
	virtual bool isKeyDownOnceWhileDownAndInDirection(T_UserKeyID down_once, T_UserKeyID while_down, AnalogStickLeftRight lr, Direction8 dir, double per_min = 0.01, double per_max = 1.0) = 0;
	virtual bool isKeyDownOnceWhileDownAndInDirection(T_UserKeyID down_once, T_UserKeyID while_down, AnalogStickLeftRight lr, double ang1, double ang2, double per_min = 0.01, double per_max = 1.0) = 0;
};

template <typename T_LogID, typename T_UserKeyID>
class GamePadLoggingInterface {
public:
	virtual ~GamePadLoggingInterface() {}

	virtual void startLogRecordNow(T_LogID log_id) = 0;
	virtual void endLogRecord(T_LogID log_id) = 0;
	virtual void removeLogRecord(T_LogID log_id) = 0;
	virtual bool isLogRecordingNow(T_LogID log_id) = 0;

	virtual unsigned int getLogTimeMax(T_LogID log_id) = 0;
	virtual void initReplay(T_LogID log_id) = 0;
	virtual void updateReplayTimeCount(T_LogID log_id) = 0;
	virtual std::shared_ptr<GamePadEventInterface<T_UserKeyID>> getLogEventInterface(T_LogID log_id) = 0;
};

template <typename T_UserKeyID>
class AnalogStickManager {
private:
	unsigned int const log_bef_count;
	std::deque<Vec2D> analog_stick_slope_raw[2];

	bool accept_update_input, return_all_false;

	std::function<Vec2D(AnalogStickLeftRight)> get_analog_stick_slope;

	bool isInRangeOfAngle(double check_ang, double ang1, double ang2);//0~2PI

public:
	AnalogStickManager(std::function<Vec2D(AnalogStickLeftRight)> getAnalogStickSlope) : log_bef_count(300) { this->get_analog_stick_slope = getAnalogStickSlope; this->initialize(); }
	void initialize() { for (unsigned int i = 0; i < this->log_bef_count; ++i) { this->analog_stick_slope_raw[0].push_back(Vec2D()); this->analog_stick_slope_raw[1].push_back(Vec2D()); } }
	void update() {
		this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Left].pop_back(); this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Right].pop_back(); this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Left].push_front(this->get_analog_stick_slope(AnalogStickLeftRight::Left)); this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Right].push_front(this->get_analog_stick_slope(AnalogStickLeftRight::Right));
	}
	void updateFromLog(std::function<Vec2D(AnalogStickLeftRight lr)> getAnalogStickSlope) {
		this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Left].pop_back(); this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Right].pop_back(); this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Left].push_front(getAnalogStickSlope(AnalogStickLeftRight::Left)); this->analog_stick_slope_raw[(int)AnalogStickLeftRight::Right].push_front(getAnalogStickSlope(AnalogStickLeftRight::Right));
	}
	void acceptUpdateInput(bool accept) { this->accept_update_input = accept; }
	void setReturnAllKeyFalse(bool all_false) { this->return_all_false = all_false; }

	Vec2D getSlope(AnalogStickLeftRight lr, double per_min = 0.0, double per_max = 1.0);
	Vec2D getSlopeHighPass(AnalogStickLeftRight lr, double per_min = 0.0) { if (this->return_all_false) return Vec2D(); if (this->get_analog_stick_slope(lr).toNorm() < per_min) return Vec2D(); else return this->get_analog_stick_slope(lr); }
	Direction4 getDirection4(AnalogStickLeftRight lr, double per_min = 0.01) { if (this->return_all_false) return Direction4::NEUTRAL; return Vec2D::toDirection4(this->analog_stick_slope_raw[(int)lr][0],per_min); }
	Direction8 getDirection8(AnalogStickLeftRight lr, double per_min = 0.01) { if (this->return_all_false) return Direction8::NEUTRAL; return Vec2D::toDirection8(this->analog_stick_slope_raw[(int)lr][0],per_min); }

	bool isSlopeOverRange(AnalogStickLeftRight lr, double per_min) { if (this->return_all_false) return false; return this->analog_stick_slope_raw[(int)lr][0].toNorm() >= per_min; }
	bool isSlopeWithinRange(AnalogStickLeftRight lr, double per_min, double per_max) { if (this->return_all_false) return false; if ((this->analog_stick_slope_raw[(int)lr][0].toNorm() < per_min) || (this->analog_stick_slope_raw[(int)lr][0].toNorm() > per_max)) return false; return true; }
	bool isSlopeOverRangeUpToTime(AnalogStickLeftRight lr, double per_min, unsigned int time_min);
	bool isSlopeWithinRangeUpToTime(AnalogStickLeftRight lr, double per_min, double per_max, unsigned time_min);
	bool isSlopeCrossOuttoThresholdOnce(AnalogStickLeftRight lr, double threshold) { if (this->return_all_false) return false; return ((this->analog_stick_slope_raw[(int)lr][0].toNorm() < threshold) && (this->analog_stick_slope_raw[(int)lr][1].toNorm() >= threshold)); }
	bool isSlopeCrossIntoThresholdOnce(AnalogStickLeftRight lr, double threshold) { if (this->return_all_false) return false; return ((this->analog_stick_slope_raw[(int)lr][0].toNorm() > threshold) && (this->analog_stick_slope_raw[(int)lr][1].toNorm() <= threshold)); }
	bool isInDirection(AnalogStickLeftRight lr, Direction4 dir, double per_min = 0.01, double per_max = 1.0) { if (this->return_all_false) return false; if ((this->analog_stick_slope_raw[(int)lr][0].toNorm() < per_min) || (this->analog_stick_slope_raw[(int)lr][0].toNorm() > per_max)) return false; return Vec2D::toDirection4(this->analog_stick_slope_raw[(int)lr][0],per_min) == dir; }
	bool isInDirection(AnalogStickLeftRight lr, Direction8 dir, double per_min = 0.01, double per_max = 1.0) { if (this->return_all_false) return false; if ((this->analog_stick_slope_raw[(int)lr][0].toNorm() < per_min) || (this->analog_stick_slope_raw[(int)lr][0].toNorm() > per_max)) return false; return Vec2D::toDirection8(this->analog_stick_slope_raw[(int)lr][0],per_min) == dir; }
	bool isInDirection(AnalogStickLeftRight lr, double ang1, double ang2, double per_min = 0.01, double per_max = 1.0);
	bool isRoundClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time);
	bool isRoundCounterClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time);
	bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction4 dir, unsigned int time);
	bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction8 dir, unsigned int time);
	bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, double ang1, double ang2, unsigned int time);
};

template<typename T_UserKeyID, typename T_UserCommandID>
class GamePad : public GamePadOperationInterface<T_UserKeyID>, public GamePadEventInterface<T_UserKeyID>, public std::enable_shared_from_this<GamePad<T_UserKeyID, T_UserCommandID>> {
private:
	std::shared_ptr<KeyManager<T_UserKeyID>> key_mgr;
	std::shared_ptr<CommandManager<T_UserCommandID>> command_mgr;
	std::shared_ptr<AnalogStickManager<T_UserKeyID>> analog_mgr;

public:
	GamePad(std::function<bool(unsigned int)> isDown, std::function<Vec2D(AnalogStickLeftRight)> getAnalogStickSlope) {
		this->key_mgr = std::make_shared<KeyManager<T_UserKeyID>>(isDown); this->command_mgr = std::make_shared<CommandManager<T_UserCommandID>>(); this->analog_mgr = std::make_shared<AnalogStickManager<T_UserKeyID>>(getAnalogStickSlope); this->initialize();
	}
	void initialize() { this->acceptUpdateInput(true); this->setReturnAllKeyFalse(false); this->key_mgr->initialize(); this->command_mgr->initialize(); this->analog_mgr->initialize(); }
	void update() { this->key_mgr->update(); this->command_mgr->update(); this->analog_mgr->update(); }
	void updateFromLog(std::function<bool(T_UserKeyID)> keyCheck_log, std::function<Vec2D(AnalogStickLeftRight lr)> getAnalogStickSlope) { this->key_mgr->updateFromLog(keyCheck_log); this->analog_mgr->updateFromLog(getAnalogStickSlope); this->command_mgr->update(); }
	std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>> clone() { return std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>>(new GamePad<T_UserKeyID, T_UserCommandID>(*this)); }

	void acceptUpdateInput(bool accept) { this->key_mgr->acceptUpdateInput(accept); this->command_mgr->acceptUpdateInput(accept); }
	void setReturnAllKeyFalse(bool all_false) { this->key_mgr->setReturnAllKeyFalse(all_false); }
	unsigned int getLibID(T_UserKeyID key_id) { return this->key_mgr->getLibID(key_id); }

	std::shared_ptr<GamePadOperationInterface<T_UserKeyID>> getOperationInterface() { return this->shared_from_this(); }
	std::shared_ptr<GamePadEventInterface<T_UserKeyID>> getEventInterface() { return this->shared_from_this(); }
	std::shared_ptr<CommandOperationInterface<T_UserCommandID>> getCommandOperationInterface() { return this->command_mgr; }


	void addKeyListener(T_UserKeyID code, unsigned int libID) override { this->key_mgr->addKeyListener(code, libID); }
	void removeKeyListener() override { this->key_mgr->removeKeyListener(); }
	void removeKeyListener(T_UserKeyID code) override { this->key_mgr->removeKeyListener(code); }

	Vec2D getSlope(AnalogStickLeftRight lr, double per_min = 0.0, double per_max = 1.0) override { return this->analog_mgr->getSlope(lr, per_min, per_max); }
	Vec2D getSlopeHighPass(AnalogStickLeftRight lr, double per_min = 0.0) override { return this->analog_mgr->getSlopeHighPass(lr, per_min); }
	Direction4 getDirection4(AnalogStickLeftRight lr, double per_min = 0.0) override { return this->analog_mgr->getDirection4(lr, per_min); }
	Direction8 getDirection8(AnalogStickLeftRight lr, double per_min = 0.0) override { return this->analog_mgr->getDirection8(lr, per_min); }

	bool isDown(T_UserKeyID code) override { return this->key_mgr->isDown(code); }
	bool isUp(T_UserKeyID code) override { return this->key_mgr->isUp(code); }
	bool isDownOnce(T_UserKeyID code) override { return this->key_mgr->isDownOnce(code); }
	bool isUpOnce(T_UserKeyID code) override { return this->key_mgr->isUpOnce(code); }
	bool isKeepDown(T_UserKeyID code, unsigned int time_min) override { return this->key_mgr->isKeepDown(code, time_min); }
	bool isKeepUp(T_UserKeyID code, unsigned int time_min) override { return this->key_mgr->isKeepUp(code, time_min); }
	bool isKeepDownOnce(T_UserKeyID code, unsigned int time) override { return this->key_mgr->isKeepDownOnce(code, time); }
	bool isKeepUpOnce(T_UserKeyID code, unsigned int time) override { return this->key_mgr->isKeepUpOnce(code, time); }
	bool isReDownInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isReDownInTime(code, allow_time); }
	bool isReUpInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isReUpInTime(code, allow_time); }
	bool isReDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isReDownWithinTime(code, min_time, max_time); }
	bool isReUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isReUpWithinTime(code, min_time, max_time); }
	bool isDoubleDownInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isDoubleDownInTime(code, allow_time); }
	bool isDoubleUpInTime(T_UserKeyID code, unsigned int allow_time) override { return this->key_mgr->isDoubleUpInTime(code, allow_time); }
	bool isDoubleDownWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDoubleDownWithinTime(code, min_time, max_time); }
	bool isDoubleUpWithinTime(T_UserKeyID code, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDoubleUpWithinTime(code, min_time, max_time); }

	bool isSimultanouesDownOnce(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) override { return this->key_mgr->isSimultanouesDownOnce(code1, code2, allow_time); }
	bool isDownOnceWhileDown(T_UserKeyID down_once, T_UserKeyID while_down) override { return this->key_mgr->isDownOnceWhileDown(down_once, while_down); }
	bool isUpOnceWhileDown(T_UserKeyID up_once, T_UserKeyID while_down) override { return this->key_mgr->isUpOnceWhileDown(up_once, while_down); }
	bool isDownOnceNextDownOnceInTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int allow_time) override { return this->key_mgr->isDownOnceNextDownOnceInTime(code1, code2, allow_time); }
	bool isDownOnceNextDownOnceWithinTime(T_UserKeyID code1, T_UserKeyID code2, unsigned int min_time, unsigned int max_time) override { return this->key_mgr->isDownOnceNextDownOnceWithinTime(code1, code2, min_time, max_time); }

	bool isSlopeOverRange(AnalogStickLeftRight lr, double per_min) override { return this->analog_mgr->isSlopeOverRange(lr, per_min); }
	bool isSlopeWithinRange(AnalogStickLeftRight lr, double per_min, double per_max) override { return this->analog_mgr->isSlopeWithinRange(lr, per_min, per_max); }
	bool isSlopeOverRangeUpToTime(AnalogStickLeftRight lr, double per_min, unsigned int time_min) override { return this->analog_mgr->isSlopeOverRangeUpToTime(lr, per_min, time_min); }
	bool isSlopeWithinRangeUpToTime(AnalogStickLeftRight lr, double per_min, double per_max, unsigned time_min) override { return this->analog_mgr->isSlopeWithinRangeUpToTime(lr, per_min, per_max, time_min); }
	bool isSlopeCrossOuttoThresholdOnce(AnalogStickLeftRight lr, double threshold) override { return this->analog_mgr->isSlopeCrossOuttoThresholdOnce(lr, threshold); }
	bool isSlopeCrossIntoThresholdOnce(AnalogStickLeftRight lr, double threshold) override { return this->analog_mgr->isSlopeCrossIntoThresholdOnce(lr, threshold); }
	bool isInDirection(AnalogStickLeftRight lr, Direction4 dir, double per_min = 0.01, double per_max = 1.0) override { return this->analog_mgr->isInDirection(lr, dir, per_min); }
	bool isInDirection(AnalogStickLeftRight lr, Direction8 dir, double per_min = 0.01, double per_max = 1.0) override { return this->analog_mgr->isInDirection(lr, dir, per_min); }
	bool isInDirection(AnalogStickLeftRight lr, double ang1, double ang2, double per_min = 0.01, double per_max = 1.0) override { return this->analog_mgr->isInDirection(lr, ang1, ang2, per_min); }
	bool isRoundClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time) override { return this->analog_mgr->isRoundClockwise(lr, per_min, time); }
	bool isRoundCounterClockwise(AnalogStickLeftRight lr, double per_min, unsigned int time) override { return this->analog_mgr->isRoundCounterClockwise(lr, per_min, time); }
	bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction4 dir, unsigned int time) override { return this->analog_mgr->isSlopeReturnInDirection(lr, slope_threshold_crossOut, slope_threshold_returnInto, dir, time); }
	bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, Direction8 dir, unsigned int time) override { return this->analog_mgr->isSlopeReturnInDirection(lr, slope_threshold_crossOut, slope_threshold_returnInto, dir, time); }
	bool isSlopeReturnInDirection(AnalogStickLeftRight lr, double slope_threshold_crossOut, double slope_threshold_returnInto, double ang1, double ang2, unsigned int time) override { return this->analog_mgr->isSlopeReturnInDirection(lr, slope_threshold_crossOut, slope_threshold_returnInto, ang1, ang2, time); }

	bool isKeyDownOnceWhileInDirection(T_UserKeyID key, AnalogStickLeftRight lr, Direction4 dir, double per_min = 0.01, double per_max = 1.0) override { return (this->key_mgr->isDownOnce(key)) && (this->analog_mgr->isInDirection(lr, dir, per_min, per_max)); }
	bool isKeyDownOnceWhileInDirection(T_UserKeyID key, AnalogStickLeftRight lr, Direction8 dir, double per_min = 0.01, double per_max = 1.0) override { return (this->key_mgr->isDownOnce(key)) && (this->analog_mgr->isInDirection(lr, dir, per_min, per_max)); }
	bool isKeyDownOnceWhileInDirection(T_UserKeyID key, AnalogStickLeftRight lr, double ang1, double ang2, double per_min = 0.01, double per_max = 1.0) override { return (this->key_mgr->isDownOnce(key)) && (this->analog_mgr->isInDirection(lr, ang1, ang2, per_min, per_max)); }
	bool isKeyDownOnceWhileDownAndInDirection(T_UserKeyID down_once, T_UserKeyID while_down, AnalogStickLeftRight lr, Direction4 dir, double per_min = 0.01, double per_max = 1.0) override { return (this->key_mgr->isDownOnceWhileDown(down_once, while_down)) && (this->analog_mgr->isInDirection(lr, dir, per_min, per_max)); }
	bool isKeyDownOnceWhileDownAndInDirection(T_UserKeyID down_once, T_UserKeyID while_down, AnalogStickLeftRight lr, Direction8 dir, double per_min = 0.01, double per_max = 1.0) override { return (this->key_mgr->isDownOnceWhileDown(down_once, while_down)) && (this->analog_mgr->isInDirection(lr, dir, per_min, per_max)); }
	bool isKeyDownOnceWhileDownAndInDirection(T_UserKeyID down_once, T_UserKeyID while_down, AnalogStickLeftRight lr, double ang1, double ang2, double per_min = 0.01, double per_max = 1.0) override { return (this->key_mgr->isDownOnceWhileDown(down_once, while_down)) && (this->analog_mgr->isInDirection(lr, ang1, ang2, per_min, per_max)); }
};

template<typename T_UserKeyID, typename T_UserCommandID>
class GamePadLog {
private:
	bool is_logging;
	unsigned int log_counter;
	const std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>> origin;
	std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>> log;
	std::unordered_map<T_UserKeyID, std::vector<bool>> onoff_log;
	std::vector<Vec2D> analog_stick_slope_log[2];

	std::function<bool(unsigned int)> isDown;
	std::function<Vec2D(AnalogStickLeftRight)> get_analog_stick_slope;

public:
	GamePadLog(std::function<bool(unsigned int)> isDown, std::function<Vec2D(AnalogStickLeftRight)> getAnalogStickSlope, std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>> origin) : origin(origin) { this->isDown = isDown; this->get_analog_stick_slope = getAnalogStickSlope; }
	void pushLog() { 
		for (auto it = this->onoff_log.begin(); it != this->onoff_log.end(); ++it) { it->second.push_back(this->isDown(this->origin->getLibID(it->first))); } this->analog_stick_slope_log[(int)AnalogStickLeftRight::Left].push_back(this->get_analog_stick_slope(AnalogStickLeftRight::Left)); this->analog_stick_slope_log[(int)AnalogStickLeftRight::Right].push_back(this->get_analog_stick_slope(AnalogStickLeftRight::Right));
	}
	void endLogging() { this->is_logging = false; }

	void initReplay() { if (this->is_logging) return; this->log = this->origin->clone(); this->log_counter = 0; }
	void updateReplayTimeCount() { if (this->is_logging) return; if (this->log_counter >= this->getLoggedTimeMax()) return; this->log->updateFromLog([this](T_UserKeyID id) {return onoff_log[id][log_counter]; }, [this](AnalogStickLeftRight lr) {return analog_stick_slope_log[(int)lr][log_counter]; }); this->log_counter++; }

	bool isLoggingNow() { return this->is_logging; }
	unsigned int getLoggedTimeMax() { return this->onoff_log.size(); }

	std::shared_ptr<GamePadEventInterface<T_UserKeyID>> getLogEventInterface() { return this->log; }
};

template<typename T_UserKeyID, typename T_UserCommandID, typename T_LogID>
class GamePadLogs : public GamePadLoggingInterface<T_LogID, T_UserKeyID> {
private:
	std::unordered_map<T_LogID, std::shared_ptr<GamePadLog<T_UserKeyID, T_UserCommandID>>> logs;
	std::function<bool(unsigned int)> isDown;
	std::function<Vec2D(AnalogStickLeftRight)> getAnalogStickSlope;
	std::function<std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>>(void)> getGamePadNowClone;

public:
	GamePadLogs(std::function<bool(unsigned int)> isDown, std::function<Vec2D(AnalogStickLeftRight)> getAnalogStickSlope, std::function<std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>>(void)> getGamePadNowClone) { this->isDown = isDown; this->getAnalogStickSlope = getAnalogStickSlope; this->getGamePadNowClone = getGamePadNowClone; }
	void initialize() { this->logs.clear(); }
	void update() { for (auto it = this->logs.begin(); it != this->logs.end(); ++it) { if (!it->second->isLoggingNow()) continue; it->second->pushLog(); } }

	void startLogRecordNow(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id] = std::make_shared<GamePadLog<T_UserKeyID, T_UserCommandID>>(this->isDown, this->getAnalogStickSlope, this->getGamePadNowClone()); }
	void endLogRecord(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->endLogging(); }
	void removeLogRecord(T_LogID log_id) override { this->logs.erase(log_id); }
	bool isLogRecordingNow(T_LogID log_id)  override { if (this->logs.find(log_id) != this->logs.end()) return false; return this->logs[log_id]->isLoggingNow(); }
	
	unsigned int getLogTimeMax(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return 0; return this->logs[log_id]->getLoggedTimeMax(); }
	void initReplay(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->initReplay(); }
	void updateReplayTimeCount(T_LogID log_id) override { if (this->logs.find(log_id) != this->logs.end()) return; this->logs[log_id]->updateReplayTimeCount(); }
	std::shared_ptr<GamePadEventInterface<T_UserKeyID>> getLogEventInterface(T_LogID log_id) override { return this->logs[log_id]->getLogEventInterface(); }
};

template<typename T_UserKeyID, typename T_UserCommandID, typename T_LogID>
class GamePadManager {
private:
	std::shared_ptr<GamePad<T_UserKeyID, T_UserCommandID>> now;
	std::shared_ptr<GamePadLogs<T_UserKeyID, T_UserCommandID, T_LogID>> logs;

	int gamepad_input;
	Vec2D stick_slope[2];

	bool isDown(unsigned int lib_key) { return (this->gamepad_input&lib_key) != 0; }
	Vec2D getAnalogStickSlope(AnalogStickLeftRight lr) { return this->stick_slope[(int)lr]; }

public:
	GamePadManager() { this->now = std::make_shared<GamePad<T_UserKeyID, T_UserCommandID>>([this](unsigned int lib_id) {return this->isDown(lib_id); }, [this](AnalogStickLeftRight lr) { return this->getAnalogStickSlope(lr); }); this->logs = std::make_shared<GamePadLogs<T_UserKeyID, T_UserCommandID, T_LogID>>([this](unsigned int lib_id) {return this->isDown(lib_id); }, [this](AnalogStickLeftRight lr) { return this->getAnalogStickSlope(lr); }, [this]() {return now->clone(); }); this->initialize(); }
	void initialize() { this->gamepad_input = 0; this->stick_slope[0] = Vec2D(); this->stick_slope[1] = Vec2D(); this->now->initialize(); this->logs->initialize(); }
	void update() { this->gamepad_input = GetJoypadInputState(DX_INPUT_PAD1); DINPUT_JOYSTATE input; GetJoypadDirectInputState(DX_INPUT_PAD1, &input); this->stick_slope[0] = Vec2D(0.001*input.X, 0.001*input.Y); this->stick_slope[1] = Vec2D(0.001*input.Z, 0.001*input.Rz);  this->now->update(); this->logs->update(); }

	std::shared_ptr<GamePadOperationInterface<T_UserKeyID>> getOperationInterface() { return this->now->getOperationInterface(); }
	std::shared_ptr<GamePadEventInterface<T_UserKeyID>> getEventInterface() { return this->now->getEventInterface(); }
	std::shared_ptr<CommandOperationInterface<T_UserKeyID>> getCommandOperationInterface() { return this->now->getCommandOperationInterface(); }
	std::shared_ptr<GamePadLoggingInterface<T_LogID, T_UserKeyID>> getLoggingInterface() { return this->logs; }

	void setReturnAllKeyFalse(bool readable) { this->now->setReturnAllKeyFalse(readable); }
	void acceptUpdateInput(bool accept) { this->now->acceptUpdateInput(accept); }
};
#pragma endregion


template <typename T_UserKeyID, typename T_UserCommandID, typename T_LogID>
class Input {
private:
	static std::shared_ptr<KeyBoardManager<T_UserKeyID, T_UserCommandID, T_LogID>> keyBoard_mgr;
	static std::shared_ptr<MouseManager<T_UserKeyID, T_UserCommandID, T_LogID>> mouse_mgr;
	static std::shared_ptr<GamePadManager<T_UserKeyID, T_UserCommandID, T_LogID>> gamePad_mgr;

public:
	static void initialize() { Input<T_UserKeyID, T_UserCommandID, T_LogID>::setReturnAllKeyFalse(true); Input<T_UserKeyID, T_UserCommandID, T_LogID>::acceptUpdateInput(true); Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->initialize(); Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->initialize(); Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->initialize(); }
	static void update() { Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->update(); Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->update(); Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->update(); }

	//key board interfaces
	static std::shared_ptr<KeyBoardOperationInterface<T_UserKeyID>> getOperationInterface_keyBoard() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->getOperationInterface(); }
	static std::shared_ptr<KeyBoardEventInterface<T_UserKeyID>> getEventInterface_keyBoard() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->getEventInterface(); }
	static std::shared_ptr<CommandOperationInterface<T_UserCommandID>> getCommandOperationInterface_keyBoard() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->getCommandOperationInterface(); }
	static std::shared_ptr<KeyBoardLoggingInterface<T_LogID, T_UserKeyID>> getLoggingInterface_keyBoard() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->getLoggingInterface(); }
	
	//mouse interfaces
	static std::shared_ptr<MouseOperationInterface<T_UserKeyID>> getOperationInterface_mouse() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->getOperationInterface(); }
	static std::shared_ptr<MouseEventInterface<T_UserKeyID>> getEventInterface_mouse() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->getEventInterface(); }
	static std::shared_ptr<CommandOperationInterface<T_UserCommandID>> getCommandOperationInterface_mouse() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->getCommandOperationInterface(); }
	static std::shared_ptr<MouseLoggingInterface<T_LogID, T_UserKeyID>> getLoggingInterface_mouse() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->getLoggingInterface(); }
	
	//game pad interfaces
	static std::shared_ptr<GamePadOperationInterface<T_UserKeyID>> getOperationInterface_gamePad() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->getOperationInterface(); }
	static std::shared_ptr<GamePadEventInterface<T_UserKeyID>> getEventInterface_gamepad() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->getEventInterface(); }
	static std::shared_ptr<CommandOperationInterface<T_UserCommandID>> getCommandOperationInterface_gamePad() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->getCommandOperationInterface(); }
	static std::shared_ptr<GamePadLoggingInterface<T_LogID, T_UserKeyID>> getLoggingInterface_gamePad() { return Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->getLoggingInterface(); }


	static void setReturnAllKeyFalse(bool readable) { Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->setReturnAllKeyFalse(readable); Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->setReturnAllKeyFalse(readable); Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->setReturnAllKeyFalse(readable); }
	static void acceptUpdateInput(bool accept) { Input<T_UserKeyID, T_UserCommandID, T_LogID>::keyBoard_mgr->acceptUpdateInput(accept); Input<T_UserKeyID, T_UserCommandID, T_LogID>::mouse_mgr->acceptUpdateInput(accept); Input<T_UserKeyID, T_UserCommandID, T_LogID>::gamePad_mgr->acceptUpdateInput(accept); }
};

using Input_T = Input<std::string, std::string, unsigned int>;