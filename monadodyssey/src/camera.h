#pragma once

#include "common.h"
#include "base_objectBase.h"
#include <vector>
#include <string>
#include <memory>

class ImageProperty;
class ObjectBase;

class CameraDrawInterface {
public:
	virtual ~CameraDrawInterface() {}

	virtual void drawImageInWorld(Vec2D center_pos_world, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const = 0;
	virtual void drawImageOnWindowFixed(Vec2D center_pos_window, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const = 0;
	virtual void drawImageOnWindowSwayed(Vec2D center_pos_window, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const = 0;

	//図形描画用
	virtual void drawSquareOnWindowFixed(Vec2D center_pos_window, Vec2D size, Color_RGB color, bool fill = true) const = 0;
	virtual void drawSquareOnWindowSwayed(Vec2D center_pos_window, Vec2D size, Color_RGB color, bool fill = true) const = 0;
	virtual void drawRotateSquareInWorld(Vec2D center_pos_world, Vec2D original_size, double rot_angle, Color_RGB color, bool fill = true) const = 0;
	virtual void drawRotateSquareOnWindowFixed(Vec2D center_pos_window, Vec2D size, double rot_angle, Color_RGB color, bool fill = true) const = 0;
	virtual void drawRotateSquareOnWindowSwayed(Vec2D center_pos_window, Vec2D size, double rot_angle, Color_RGB color, bool fill = true) const = 0;

	virtual void drawLineInWorld(Vec2D pos_start_world, Vec2D pos_end_world, Color_RGB color, int width = 1) const = 0;
	virtual void drawLineOnWindowFixed(Vec2D pos_start_window, Vec2D pos_end_window, Color_RGB color, int width = 1) const = 0;
	virtual void drawLineOnWindowSwayed(Vec2D pos_start_window, Vec2D pos_end_window, Color_RGB color, int width = 1) const = 0;

	virtual void drawCircleInWorld(Vec2D center_pos_world, int r_original, Color_RGB color, bool fill = true) const = 0;
	virtual void drawCircleOnWindowFixed(Vec2D center_pos_window, int r, Color_RGB color, bool fill = true) const = 0;
	virtual void drawCircleOnWindowSwayed(Vec2D center_pos_window, int r, Color_RGB color, bool fill = true) const = 0;

	virtual void drawStringInWorld(Vec2D center_pos_world, std::string text, Color_RGB color, double expansion = 1.0) const = 0;
	virtual void drawStringOnWindowFixed(Vec2D center_pos_window, std::string text, Color_RGB color, double expansion = 1.0) const = 0;
	virtual void drawStringOnWindowSwayed(Vec2D center_pos_window, std::string text, Color_RGB color, double expansion = 1.0) const = 0;
};

class CameraRange {
private:
	const Vec2D _leftup, _rightdown;
public:
	CameraRange(Vec2D leftup, Vec2D rightdown) : _leftup(leftup), _rightdown(rightdown) { }
	const Vec2D leftup() const { return this->_leftup; }
	const Vec2D rightdown() const { return this->_rightdown; }
	const Vec2D size() const { return this->_rightdown - this->_leftup; }
	const Vec2D center() const { return this->_leftup + this->size() / 2; }
};

class Camera : public CameraDrawInterface{
private:
	const int buffer_id;
	const CameraRange drawArea_window, moveArea_world;
	double shield_ratio;//描画遮蔽率(0.0:黒 ～ 1.0:全描画)
	
	Vec2D pos_anchor_window;//カメラ基準点のスクリーン座標

	int move_count, move_count_max;
	Vec2D pos_anchor_world;//カメラ基準点のワールド座標
	Vec2D anchor_target_world_pos_start, anchor_target_world_pos_goal;

	int zoom_count, zoom_count_max;
	double zoom_magnification;
	double zoom_magnification_start, zoom_magnification_end;

	int rotate_count, rotate_count_max;
	double rotate_angle;
	double rotate_angle_start, rotate_angle_end;

	int sway_count, sway_count_max, sway_update_time;
	Vec2D sway;
	int sway_power_max;
	double sway_vib_trans_rad;
	int sway_vib_rotate_freq;
	
public:
	Camera(CameraRange drawArea) : drawArea_window(drawArea), moveArea_world(drawArea), buffer_id(MakeScreen((int)drawArea.size().x, (int)drawArea.size().y)) { this->initialize(); }
	Camera(CameraRange drawArea, CameraRange moveArea) : drawArea_window(drawArea), moveArea_world(moveArea), buffer_id(MakeScreen((int)drawArea.size().x, (int)drawArea.size().y)) {
		this->initialize(); this->pos_anchor_world = WINDOW_SIZE / 2; this->pos_anchor_window = WINDOW_SIZE / 2; this->anchor_target_world_pos_start = WINDOW_SIZE / 2; this->anchor_target_world_pos_goal = WINDOW_SIZE / 2;
	}
	Camera(CameraRange drawArea, CameraRange moveArea, Vec2D pos_anchor_window, Vec2D pos_anchor_world) : drawArea_window(drawArea), moveArea_world(moveArea), buffer_id(MakeScreen((int)drawArea.size().x, (int)drawArea.size().y)) {
		this->initialize(); this->pos_anchor_world = pos_anchor_world; this->pos_anchor_window = pos_anchor_window; this->anchor_target_world_pos_start = pos_anchor_world; this->anchor_target_world_pos_goal = pos_anchor_world;
	}
	
	void initialize();
	void update();

	void setShieldRatio(double ratio) { this->shield_ratio = ratio; }

	void setAnchorWorldPosition(Vec2D world_pos);
	void setAnchorWorldPosition(Vec2D world_pos, int count);
	void setAnchorWindowPosition(Vec2D window_pos);
	void setAnchorWindowPosition(Vec2D window_pos, int count);

	void setZoom(double zoom);
	void setZoom(double zoom, int count);
	void setZoomAdd(double zoom_add, int count);
	void setZoomReset(int count);
	double getZoom() const { return this->zoom_magnification; }

	void setRotation(double angle_rad);
	void setRotation(double angle_rad, int count);
	void setRotationAdd(double angle_rad_add, int count);
	double getRotation() const { return this->rotate_angle; }

	void setSwayRandomInSquare(int pow, int count_max, int update_time);
	void setSwayVibrationDecaying(int pow, int count_max, double trans_rad, int rot_freq);


	//画像描画用
	void drawImageInWorld(Vec2D center_pos_world, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const override;
	void drawImageOnWindowFixed(Vec2D center_pos_window, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const override;
	void drawImageOnWindowSwayed(Vec2D center_pos_window, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const override;

	//図形描画用
	void drawSquareOnWindowFixed(Vec2D center_pos_window, Vec2D size, Color_RGB color, bool fill = true) const override;//not sway, not rotate
	void drawSquareOnWindowSwayed(Vec2D center_pos_window, Vec2D size, Color_RGB color, bool fill = true) const override;//sway, not rotate
	void drawRotateSquareInWorld(Vec2D center_pos_world, Vec2D original_size, double rot_angle, Color_RGB color, bool fill = true) const override;
	void drawRotateSquareOnWindowFixed(Vec2D center_pos_window, Vec2D size, double rot_angle, Color_RGB color, bool fill = true) const override;//not sway, rotate (カメラによる回転は考えない。図形の回転のみ)
	void drawRotateSquareOnWindowSwayed(Vec2D center_pos_window, Vec2D size, double rot_angle, Color_RGB color, bool fill = true) const override;//sway, rotate (カメラによる回転は考えない.図形の回転のみ)

	void drawLineInWorld(Vec2D pos_start_world, Vec2D pos_end_world, Color_RGB color, int width = 1) const override;
	void drawLineOnWindowFixed(Vec2D pos_start_window, Vec2D pos_end_window, Color_RGB color, int width = 1) const override;
	void drawLineOnWindowSwayed(Vec2D pos_start_window, Vec2D pos_end_window, Color_RGB color, int width = 1) const override;

	void drawCircleInWorld(Vec2D center_pos_world, int r_original, Color_RGB color, bool fill = true) const override;
	void drawCircleOnWindowFixed(Vec2D center_pos_window, int r, Color_RGB color, bool fill = true) const override;
	void drawCircleOnWindowSwayed(Vec2D center_pos_window, int r, Color_RGB color, bool fill = true) const override;

	void drawStringInWorld(Vec2D center_pos_world, std::string text, Color_RGB color, double expansion = 1.0) const override;
	void drawStringOnWindowFixed(Vec2D center_pos_window, std::string text, Color_RGB color, double expansion = 1.0) const override;
	void drawStringOnWindowSwayed(Vec2D center_pos_window, std::string text, Color_RGB color, double expansion = 1.0) const override;

	bool isVisibleForWindowPos(Vec2D pos_window) const;//ウィンドウ座標系[px]の点がカメラ領域内に存在するか
	bool isVisibleForWindowPos(Vec2D pos_window, Vec2D size_window) const;//ウィンドウ座標系[px]の矩形がカメラ領域内に(一部でも)存在するか(回転を考慮しない)
	bool isVisibleForWindowPos(Vec2D pos_window, Vec2D size_window, double rot_angle_in_window) const;//ウィンドウ座標系[px]の矩形がカメラ領域内に(一部でも)存在するか(ウィンドウ座標系での回転角度を考慮する)
	bool isVisibleForWorldPos(Vec2D pos_world) const;//ワールド座標系[px]の点がカメラ領域内に存在するか
	bool isVisibleForWorldPos(const std::shared_ptr<ObjectBase> &obj) const;//オブジェクトがカメラ領域内に存在するか
	bool isVisibleForWorldPos(Vec2D pos_world, Vec2D size_original) const;//ワールド座標系[px]の矩形がカメラ領域内に(一部でも)存在するか(回転を考慮しない)
	bool isVisibleForWorldPos(Vec2D pos_world, Vec2D size_original, double rot_angle_in_window) const;//ワールド座標系[px]の矩形がカメラ領域内に(一部でも)存在するか(ウィンドウ座標系での回転角度を考慮する)

	Vec2D toWindowPosPxFromWorldPos(Vec2D world_pos) const { return (this->pos_anchor_window + Mat2D::rotation(-this->rotate_angle)*(world_pos - this->pos_anchor_world)*this->zoom_magnification); }
	Vec2D toWorldPosFromWindowPosPx(Vec2D window_pos) const { return (this->pos_anchor_world + Mat2D::rotation(-this->rotate_angle)*(window_pos - this->pos_anchor_window) / this->zoom_magnification); }
};
