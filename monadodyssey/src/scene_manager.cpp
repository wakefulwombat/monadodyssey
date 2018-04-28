#include "scene_manager.h"
#include "base_sceneBase.h"
#include "input.h"

SceneManager::SceneManager() : SceneCommonData() {
	this->initialize();

	Input_T::getOperationInterface_mouse()->addKeyListener("left", MOUSE_INPUT_LEFT);
	Input_T::getOperationInterface_mouse()->addKeyListener("right", MOUSE_INPUT_RIGHT);
	Input_T::getOperationInterface_mouse()->addKeyListener("middle", MOUSE_INPUT_MIDDLE);
}

void SceneManager::initialize() {
	this->newSceneKind = SceneKindAll::GameMain;
}

void SceneManager::update() {
	if (this->newSceneKind != SceneKindAll::None) {
		if (this->nowScene != nullptr) {
			this->nowScene->finalize();
		}

		switch (this->newSceneKind) {
		case SceneKindAll::Ending:
			this->nowScene = nullptr;
			break;

		case SceneKindAll::GameMain:
			this->nowScene = nullptr;
			break;

		case SceneKindAll::StageSelect:
			this->nowScene = nullptr;
			break;

		case SceneKindAll::Result:
			this->nowScene = nullptr;
			break;

		case SceneKindAll::Title:
			this->nowScene = nullptr;
			break;
		}

		this->newSceneKind = SceneKindAll::None;
		this->nowScene->initialize();
	}
	else {
		this->nowScene->update();
	}
}