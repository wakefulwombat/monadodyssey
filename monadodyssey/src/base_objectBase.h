#pragma once

#include <memory>
#include "common.h"
#include "camera.h"
#include "DxLib.h"
#include <math.h>
#include <vector>
#include <functional>

class CameraDrawInterface;

//大きさを持つオブジェクトの基底
class ObjectBase {
private:
	bool validation;
	Vec2D world_pos;
	double expansion_rate;
	double rotate_angle;

	const Vec2D default_obj_size;
	const unsigned int z_sort;//zソート

public:
	ObjectBase(Vec2D world_pos, unsigned int z_sort, Vec2D default_obj_size) : z_sort(z_sort), default_obj_size(default_obj_size) { this->world_pos = world_pos; this->initialize(); }
	virtual ~ObjectBase(){}

	virtual void initialize() { this->validation = true; this->expansion_rate = 1.0; this->rotate_angle = 0.0; }
	virtual void update() = 0;
	virtual void draw(const std::shared_ptr<CameraDrawInterface> &camera) const = 0;
	virtual void finalize() {}

	bool isValid() const { return this->validation; }
	unsigned int getZSort() const { return this->z_sort; }
	double getRotateAngle() { return this->rotate_angle; }
	Vec2D getSize() const { return this->default_obj_size*this->expansion_rate; }
	//bool isVisibleInCamera(const std::shared_ptr<Camera> &camera) const { return camera->isVisibleForWindowPos(this->world_pos, this->default_obj_size*this->expansion_rate, this->rotate_angle); }
	Vec2D getWorldPosition() const { return this->world_pos; }

	void setInvalid() { this->validation = false; }
	void setWorldPosition(Vec2D world_pos) { this->world_pos = world_pos; }
	void setExpansionRate(double expansion_rate) { this->expansion_rate = expansion_rate; }
	void setRotateAngle(double rotate_angle) { this->rotate_angle = rotate_angle; }
};


//画像を使うオブジェクト
class ImageProperty {
public:
	ImageProperty(Vec2D img_original_size, Vec2D obj_size, double rot = 0.0, double opac = 1.0, bool turn = false) :img_size(img_original_size), img_expansion_x(obj_size.x / img_original_size.x), img_expansion_y(obj_size.y / img_original_size.y), img_rotation_offset(rot) {
		this->img_opacity = opac;
		this->img_turn = turn;
	}
	
	Vec2D ImgSize() const { return this->img_size; }
	double ImgExp_X() const { return this->img_expansion_x; }
	double ImgExp_Y() const { return this->img_expansion_y; }
	double ImgRotOffset() const { return this->img_rotation_offset; }
	double ImgOpacity() const { return this->img_opacity; }
	bool ImgTurn() const { return this->img_turn; }

private:
	const Vec2D img_size;//画像ファイル自体のサイズ
	const double img_expansion_x, img_expansion_y;//画像ファイルとオブジェクトのサイズ比
	const double img_rotation_offset;//画像ファイルをあらかじめ回転させておく量
	double img_opacity;//画像透明度(0.0~1.0)
	bool img_turn;//左右反転

protected:
	void ImgOpacity(double opacity) { this->img_opacity = opacity; }
	void ImgTurn(bool turn) { this->img_turn = turn; }
};

//移動するオブジェクト
class MoveProperty {
public:
	MoveProperty(Vec2D trans_vel = Vec2D(0.0, 0.0), Vec2D trans_acc = Vec2D(0.0, 0.0), double omega_z = 0.0) {
		this->trans_vel = trans_vel;
		this->trans_acc = trans_acc;
		this->omega_z = omega_z;
	}

private:
	//並進
	Vec2D trans_vel, trans_acc;
	
	//回転
	double omega_z, domega_z;

protected:
	Vec2D TransVel() const { return this->trans_vel; }
	Vec2D TransAcc() const { return this->trans_acc; }
	double OmegaZ() const { return this->omega_z; }
	double dOmegaZ() const { return this->domega_z; }

	void TransVel(Vec2D trans_vel) { this->trans_vel = trans_vel; }
	void TransAcc(Vec2D trans_acc) { this->trans_acc = trans_acc; }
	void TransVel(double r, double ang) { this->trans_vel = Vec2D(r*cos(ang), r*sin(ang)); }
	void TransAcc(double r, double ang) { this->trans_acc = Vec2D(r*cos(ang), r*sin(ang)); }
	void addTransVel(Vec2D trans_vel) { this->trans_vel += trans_vel; }
	void addTransAcc(Vec2D trans_acc) { this->trans_acc += trans_acc; }
	void OmegaZ(double omega_z) { this->omega_z = omega_z; }
	void dOmegaZ(double domega_z) { this->domega_z = domega_z; }
	void addOmegaZ(double omega_z) { this->omega_z += omega_z; }
	void adddOmegaZ(double domega_z) { this->domega_z += domega_z; }

	void rotateAngleTransVel(double ang) { this->trans_vel = Mat2D::rotation(ang)*this->trans_vel; }
	void rotateAngleTransAcc(double ang) { this->trans_acc = Mat2D::rotation(ang)*this->trans_acc; }
};
