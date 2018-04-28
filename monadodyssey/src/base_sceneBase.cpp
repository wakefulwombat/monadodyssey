#include "base_sceneBase.h"
#include "input.h"
#include <deque>

LayerBase::LayerBase(double under_layer_shield_ratio, bool allow_update_under_layer, const std::shared_ptr<Camera> &cam)
	: under_layer_shield_ratio(under_layer_shield_ratio), allow_update_under_layer(allow_update_under_layer), camera(cam) {
	this->initialize();
}

void LayerBase::update() {
	this->camera->update();
	
	for (auto it = this->objects.begin(); it != this->objects.end();) {
		if (!(*it)->isValid()) {
			it = this->objects.erase(it);
		}
		else {
			(*it)->update();
			++it;
		}
	}
}

void WindowBase::update()
{
	if (!this->layers[this->layers.size() - 1]->isValid()) this->layers.pop_back();
	unsigned int l = this->layers.size();

	std::deque<double> shields;
	std::deque<bool> update_allowed;

	shields.push_front(1.0);
	update_allowed.push_front(true);
	for (int i = l - 2; i >= 0; --i) {
		shields.push_front(this->layers[i + 1]->getShieldRatioOfUnderLayer()*shields.front());
	}
	for (int i = l - 2; i >= 0; --i) {
		if (!this->layers[i + 1]->doesAllowedUpdateUnderLayer()) {
			for (int j = i; j >= 0; --j) update_allowed.push_front(false);
			break;
		}
		else update_allowed.push_front(true);
	}


	for (unsigned int i = 0; i < l; ++i) {
		if (i == l - 1) Input_T::setReturnAllKeyFalse(false);
		else Input_T::setReturnAllKeyFalse(true);

		if (update_allowed[i]) this->layers[i]->update();
	}
	for (unsigned int i = 0; i < l; ++i) {
		this->layers[i]->setCameraShieldRatio(shields[i]);
		this->layers[i]->draw();
	}
}

void WindowBase::draw() const
{
}