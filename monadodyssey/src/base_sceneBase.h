#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <future>
#include "base_objectBase.h"
#include "Camera.h"


class LayerBase {
protected:
	const std::shared_ptr<Camera> camera;

	const Color_RGB background_color;

	std::vector<std::shared_ptr<ObjectBase>> objects;
	const double under_layer_shield_ratio;//下層レイヤーの遮蔽率(描画前にカメラに設定する)(0.0で映さない)
	const bool allow_update_under_layer;//下層レイヤーを更新するか
	bool validation;

	virtual void checkEvent_backGroundClicked(Vec2D mouse_window_pos) {};

public:
	LayerBase(double under_layer_shield_ratio, bool allow_update_under_layer, const std::shared_ptr<Camera> &camera);
	virtual ~LayerBase() {}

	std::shared_ptr<Camera> getCamera() { return this->camera; }
	void setCameraShieldRatio(double ratio) { return this->camera->setShieldRatio(ratio); }
	bool isValid() { return this->validation; }
	void addObject(const std::shared_ptr<ObjectBase> &obj) { this->objects.push_back(obj); std::sort(this->objects.begin(), this->objects.end(), [](const std::shared_ptr<ObjectBase> &left, const std::shared_ptr<ObjectBase> &right) {return left->getZSort() < right->getZSort(); }); }
	
	double getShieldRatioOfUnderLayer() { return this->under_layer_shield_ratio; }
	bool doesAllowedUpdateUnderLayer() { return this->allow_update_under_layer; }

	void setInvalid() { this->validation = false; }

	void initialize() { this->validation = true; this->objects.clear(); this->camera->initialize(); }
	void update();
	virtual void draw() const { for each(std::shared_ptr<ObjectBase> obj in this->objects) { obj->draw(this->camera); } }
};

class WindowBase {
protected:
	const std::shared_ptr<Camera> camera;
	std::vector<std::shared_ptr<LayerBase>> layers;
	bool validation;

	Vec2D center_pos_onWindow;
	Vec2D window_size;

public:
	WindowBase(CameraRange drawArea) : camera(std::make_shared<Camera>(drawArea)) { this->initialise(); }
	WindowBase(CameraRange drawArea, CameraRange moveArea) : camera(std::make_shared<Camera>(drawArea, moveArea)) { this->initialise(); }
	WindowBase(CameraRange drawArea, CameraRange moveArea, Vec2D pos_anchor_window, Vec2D pos_anchor_world) : camera(std::make_shared<Camera>(drawArea, moveArea, pos_anchor_window, pos_anchor_world)) { this->initialise(); }

	void initialise() { this->layers.clear(); this->validation = true;}
	void update();
	virtual void draw() const;

	bool isValid() { return this->validation; }

	void setInvalid() { this->validation = false; }
	void setCenter(Vec2D center_pos) { this->center_pos_onWindow = center_pos; }
	void setLeftUp(Vec2D leftup) { this->center_pos_onWindow = leftup + this->window_size / 2; }
	void setSize(Vec2D window_size) { this->window_size = window_size; }
};

class SceneCommonData {
public:
	int stage_num;

	SceneCommonData() { this->stage_num = 0; }
};

class SceneBase {
protected:
	std::shared_ptr<SceneCommonData> commonData;
	std::vector<std::shared_ptr<WindowBase>> windows;

public:
	SceneBase(const std::shared_ptr<SceneCommonData> &common) { this->commonData = common; this->initialize(); }
	virtual ~SceneBase() {}
	void pushLayer(const std::shared_ptr<WindowBase> &layer) { this->windows.push_back(layer); }

	virtual void initialize() { this->windows.clear(); }
	void update() { for (auto it = this->windows.begin(); it != this->windows.end();) { if (!(*it)->isValid()) { it = this->windows.erase(it); continue; } (*it)->update(); ++it; } }
	virtual void finalize() {}
};
