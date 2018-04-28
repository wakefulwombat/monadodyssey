#pragma once

#include "base_sceneBase.h"
#include "Eigen\Dense"

enum class SceneKindAll {
	None,
	Title,
	StageSelect,
	GameMain,
	Result,
	Ending
};

class SceneManager : public SceneCommonData, public std::enable_shared_from_this<SceneManager> {
private:
	SceneKindAll newSceneKind;
	std::shared_ptr<SceneBase> nowScene;

public:
	SceneManager();
	void initialize();
	void update();
};