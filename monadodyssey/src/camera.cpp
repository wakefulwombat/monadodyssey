#include "Camera.h"
#include "resource.h"
#include "DxLib.h"
#include <math.h>
#include <algorithm>

void Camera::initialize() {
	this->pos_anchor_world = this->drawArea_window.center();
	this->pos_anchor_window = this->drawArea_window.center();
	this->shield_ratio = 1.0;

	this->anchor_target_world_pos_goal = this->pos_anchor_world;
	this->anchor_target_world_pos_start = this->pos_anchor_world;
	this->move_count = 0;
	this->move_count_max = 0;

	this->zoom_magnification = 1.0;
	this->zoom_count = 0;
	this->zoom_count_max = 0;
	this->zoom_magnification_end = 1.0;
	this->zoom_magnification_start = 1.0;

	this->rotate_angle = 0.0;
	this->rotate_count = 0;
	this->rotate_count_max = 0;
	this->rotate_angle_start = 0.0;
	this->rotate_angle_end = 0.0;

	this->sway = Vec2D(0.0, 0.0);
	this->sway_count = 0;
	this->sway_count_max = 0;
}

void Camera::setAnchorWorldPosition(Vec2D world_pos) {
	this->move_count = 0;
	this->move_count_max = 0;

	this->pos_anchor_world = world_pos;
	this->anchor_target_world_pos_goal = world_pos;
	this->anchor_target_world_pos_start = world_pos;
}

void Camera::setAnchorWorldPosition(Vec2D world_pos, int count) {
	this->move_count = 0;
	this->move_count_max = count;

	this->anchor_target_world_pos_goal = world_pos;
	this->anchor_target_world_pos_start = this->pos_anchor_world;
}

void Camera::setAnchorWindowPosition(Vec2D window_pos) {
	this->move_count = 0;
	this->move_count_max = 0;

	this->anchor_target_world_pos_goal = this->pos_anchor_world;
	this->anchor_target_world_pos_start = this->pos_anchor_world;
	this->pos_anchor_window = window_pos;
}

void Camera::setAnchorWindowPosition(Vec2D window_pos, int count) {
	this->move_count = 0;
	this->move_count_max = count;

	this->anchor_target_world_pos_goal = this->pos_anchor_world;
	this->anchor_target_world_pos_start = this->toWorldPosFromWindowPosPx(window_pos);
	this->pos_anchor_world = this->toWorldPosFromWindowPosPx(window_pos);
	this->pos_anchor_window = window_pos;
}

void Camera::setZoom(double zoom) {
	this->zoom_count = 0;
	this->zoom_count_max = 0;

	this->zoom_magnification = zoom;
	this->zoom_magnification_start = zoom;
	this->zoom_magnification_end = zoom;
}

void Camera::setZoom(double zoom, int count) {
	this->zoom_count = 0;
	this->zoom_count_max = count;

	this->zoom_magnification_start = this->zoom_magnification;
	this->zoom_magnification_end = zoom;
}

void Camera::setZoomAdd(double zoom_add, int count) {
	this->zoom_count = 0;
	this->zoom_count_max = count;

	this->zoom_magnification_start = this->zoom_magnification;
	this->zoom_magnification_end = this->zoom_magnification + zoom_add;
}

void Camera::setZoomReset(int count) {
	this->zoom_count = 0;
	this->zoom_count_max = count;

	this->zoom_magnification_start = this->zoom_magnification;
	this->zoom_magnification_end = 1.0;
}

void Camera::setRotation(double angle) {
	this->rotate_count = 0;
	this->rotate_count_max = 0;

	this->rotate_angle = angle;
	this->rotate_angle_start = angle;
	this->rotate_angle_end = angle;
}

void Camera::setRotation(double angle, int count) {
	this->rotate_count = 0;
	this->rotate_count_max = count;

	this->rotate_angle_start = this->rotate_angle;
	this->rotate_angle_end = angle;
}

void Camera::setRotationAdd(double angle_add, int count) {
	this->rotate_count = 0;
	this->rotate_count_max = count;

	this->rotate_angle_start = this->rotate_angle;
	this->rotate_angle_end = this->rotate_angle + angle_add;
}

void Camera::setSwayRandomInSquare(int pow, int count_max, int update_time) {
	this->sway_count = 0;
	this->sway_count_max = count_max;
	this->sway_power_max = pow;
	this->sway_update_time = update_time;

	this->sway = Vec2D(0.0, 0.0);
	this->sway_vib_rotate_freq = 0;
	this->sway_vib_trans_rad = 0.0;
}

void Camera::setSwayVibrationDecaying(int pow, int count_max, double trans_rad, int rot_freq) {
	this->sway_count = 0;
	this->sway_count_max = count_max;
	this->sway_power_max = pow;
	this->sway_vib_rotate_freq = rot_freq;
	this->sway_vib_trans_rad = trans_rad;

	this->sway = Vec2D(0.0, 0.0);
	this->sway_update_time = 0;
}


void Camera::drawImageInWorld(Vec2D center_pos, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const {
	Vec2D p = this->toWindowPosPxFromWorldPos(center_pos) + this->sway;
	double d = this->zoom_magnification*expansion;
	double a = img_prop->ImgRotOffset() + rotate - this->rotate_angle;

	if (!this->isVisibleForWindowPos(p, obj_size*d, a)) return;

	SetDrawBright((int)(this->shield_ratio*img_prop->ImgOpacity() * 255), (int)(this->shield_ratio*img_prop->ImgOpacity() * 255), (int)(this->shield_ratio*img_prop->ImgOpacity() * 255));
	DrawRotaGraph3((int)(p.x), (int)(p.y), (int)(img_prop->ImgSize().x) / 2, (int)(img_prop->ImgSize().y) / 2, d*img_prop->ImgExp_X(), d*img_prop->ImgExp_Y(), a, graphHandle, true, img_prop->ImgTurn());
}

void Camera::drawImageOnWindowFixed(Vec2D center_pos_window, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const {
	Vec2D p = center_pos_window;
	double d = this->zoom_magnification*expansion;
	double a = img_prop->ImgRotOffset() + rotate - this->rotate_angle;

	if (!this->isVisibleForWindowPos(p, obj_size*d, a)) return;

	SetDrawBright((int)(this->shield_ratio*img_prop->ImgOpacity() * 255), (int)(this->shield_ratio*img_prop->ImgOpacity() * 255), (int)(this->shield_ratio*img_prop->ImgOpacity() * 255));
	DrawRotaGraph3((int)(p.x), (int)(p.y), (int)(img_prop->ImgSize().x) / 2, (int)(img_prop->ImgSize().y) / 2, d*img_prop->ImgExp_X(), d*img_prop->ImgExp_Y(), a, graphHandle, true, img_prop->ImgTurn());
}

void Camera::drawImageOnWindowSwayed(Vec2D center_pos_window, Vec2D obj_size, double expansion, double rotate, int graphHandle, const std::shared_ptr<ImageProperty> &img_prop) const {
	Vec2D p = center_pos_window + this->sway;
	double d = this->zoom_magnification*expansion;
	double a = img_prop->ImgRotOffset() + rotate - this->rotate_angle;

	if (!this->isVisibleForWindowPos(p, obj_size*d, a)) return;

	SetDrawBright((int)(this->shield_ratio*img_prop->ImgOpacity() * 255), (int)(this->shield_ratio*img_prop->ImgOpacity() * 255), (int)(this->shield_ratio*img_prop->ImgOpacity() * 255));
	DrawRotaGraph3((int)(p.x), (int)(p.y), (int)(img_prop->ImgSize().x) / 2, (int)(img_prop->ImgSize().y) / 2, d*img_prop->ImgExp_X(), d*img_prop->ImgExp_Y(), a, graphHandle, true, img_prop->ImgTurn());
}


void Camera::drawSquareOnWindowFixed(Vec2D pos_center, Vec2D size, Color_RGB color, bool fill) const {
	if (!this->isVisibleForWindowPos(pos_center, size)) return;

	Vec2D lu = pos_center - (size / 2);
	Vec2D rd = pos_center + (size / 2);
	Color_RGB c = color*this->shield_ratio;

	DrawBox((int)lu.x, (int)lu.y, (int)rd.x, (int)rd.y, c.toColor(), fill);
}

void Camera::drawSquareOnWindowSwayed(Vec2D pos_center, Vec2D size, Color_RGB color, bool fill) const {
	if (!this->isVisibleForWindowPos(pos_center + this->sway, size)) return;

	Vec2D lu = pos_center - (size / 2) + this->sway;
	Vec2D rd = pos_center + (size / 2) + this->sway;
	Color_RGB c = color*this->shield_ratio;

	DrawBox((int)lu.x, (int)lu.y, (int)rd.x, (int)rd.y, c.toColor(), fill);
}

void Camera::drawRotateSquareOnWindowFixed(Vec2D pos_center, Vec2D size, double rot_angle, Color_RGB color, bool fill) const {
	if (!this->isVisibleForWindowPos(pos_center, size, rot_angle)) return;

	Vec2D p1 = pos_center + Mat2D::rotation(rot_angle)*size / 2;
	Vec2D p2 = pos_center + Mat2D::rotation(rot_angle)*Mat2D::scaling(-1.0, 1.0) * size / 2;
	Vec2D p3 = pos_center + Mat2D::rotation(rot_angle)*Mat2D::scaling(1.0, -1.0) * size / 2;
	Vec2D p4 = pos_center + Mat2D::rotation(rot_angle)*(-size) / 2;
	Color_RGB c = color*this->shield_ratio;

	DrawQuadrangle((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, (int)p3.x, (int)p3.y, (int)p4.x, (int)p4.y, c.toColor(), fill);
}

void Camera::drawRotateSquareOnWindowSwayed(Vec2D pos_center, Vec2D size, double rot_angle, Color_RGB color, bool fill) const {
	if (!this->isVisibleForWindowPos(pos_center + this->sway, size, rot_angle)) return;

	Vec2D p1 = pos_center + Mat2D::rotation(rot_angle)*size / 2 + this->sway;
	Vec2D p2 = pos_center + Mat2D::rotation(rot_angle)*Mat2D::scaling(-1.0, 1.0) * size / 2 + this->sway;
	Vec2D p3 = pos_center + Mat2D::rotation(rot_angle)*Mat2D::scaling(1.0, -1.0) * size / 2 + this->sway;
	Vec2D p4 = pos_center + Mat2D::rotation(rot_angle)*(-size) / 2 + this->sway;
	Color_RGB c = color*this->shield_ratio;

	DrawQuadrangle((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, (int)p3.x, (int)p3.y, (int)p4.x, (int)p4.y, c.toColor(), fill);
}

void Camera::drawRotateSquareInWorld(Vec2D pos_center, Vec2D original_size, double rot_angle, Color_RGB color, bool fill) const {
	Vec2D p = this->toWindowPosPxFromWorldPos(pos_center) + this->sway;
	Vec2D s = original_size * this->zoom_magnification;
	double a = rot_angle - this->rotate_angle;

	if (!this->isVisibleForWindowPos(p, s, a)) return;

	Vec2D p1 = p + Mat2D::rotation(a)*s / 2;
	Vec2D p2 = p + Mat2D::rotation(a)*Mat2D::scaling(-1.0, 1.0) * s / 2;
	Vec2D p3 = p + Mat2D::rotation(a)*Mat2D::scaling(1.0, -1.0) * s / 2;
	Vec2D p4 = p + Mat2D::rotation(a)*(-s) / 2;
	Color_RGB c = color*this->shield_ratio;

	DrawQuadrangle((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, (int)p3.x, (int)p3.y, (int)p4.x, (int)p4.y, c.toColor(), fill);
}

void Camera::drawLineInWorld(Vec2D pos_start, Vec2D pos_end, Color_RGB color, int x) const {
	Vec2D s = this->toWindowPosPxFromWorldPos(pos_start) + this->sway;
	Vec2D e = this->toWindowPosPxFromWorldPos(pos_end) + this->sway;

	if ((!this->isVisibleForWindowPos(s)) && (!this->isVisibleForWindowPos(e))) return;

	Color_RGB c = color*this->shield_ratio;
	DrawLine((int)s.x, (int)s.y, (int)e.x, (int)e.y, c.toColor(), x);
}

void Camera::drawLineOnWindowFixed(Vec2D pos_start, Vec2D pos_end, Color_RGB color, int x) const {
	if ((!this->isVisibleForWindowPos(pos_start)) && (!this->isVisibleForWindowPos(pos_end))) return;

	Color_RGB c = color*this->shield_ratio;
	DrawLine((int)pos_start.x, (int)pos_start.y, (int)pos_end.x, (int)pos_end.y, c.toColor(), x);
}

void Camera::drawLineOnWindowSwayed(Vec2D pos_start, Vec2D pos_end, Color_RGB color, int x) const {
	Vec2D s = pos_start + this->sway;
	Vec2D e = pos_end + this->sway;

	if ((!this->isVisibleForWindowPos(s)) && (!this->isVisibleForWindowPos(e))) return;

	Color_RGB c = color*this->shield_ratio;
	DrawLine((int)s.x, (int)s.y, (int)e.x, (int)e.y, c.toColor(), x);
}

void Camera::drawCircleInWorld(Vec2D pos_center, int r_original, Color_RGB color, bool fill) const {
	Vec2D c = this->toWindowPosPxFromWorldPos(pos_center) + this->sway;

	if (!this->isVisibleForWindowPos(c, Vec2D(2 * r_original, 2 * r_original)*this->zoom_magnification)) return;

	Color_RGB col = color*this->shield_ratio;
	DrawCircle((int)c.x, (int)c.y, (int)(r_original*this->zoom_magnification), col.toColor(), fill);
}

void Camera::drawCircleOnWindowFixed(Vec2D pos_center, int r, Color_RGB color, bool fill) const {
	if (!this->isVisibleForWindowPos(pos_center, Vec2D(2 * r, 2 * r))) return;

	Color_RGB col = color*this->shield_ratio;
	DrawCircle((int)pos_center.x, (int)pos_center.y, r, col.toColor(), fill);
}

void Camera::drawCircleOnWindowSwayed(Vec2D pos_center, int r, Color_RGB color, bool fill) const {
	Vec2D c = pos_center + this->sway;

	if (!this->isVisibleForWindowPos(c, Vec2D(2 * r, 2 * r))) return;

	Color_RGB col = color*this->shield_ratio;
	DrawCircle((int)c.x, (int)c.y, r, col.toColor(), fill);
}

void Camera::drawStringInWorld(Vec2D pos_center, std::string text, Color_RGB color, double expansion) const {
	Vec2D p = this->toWindowPosPxFromWorldPos(pos_center) + this->sway;
	double d = expansion*this->zoom_magnification;
	Vec2D s = Vec2D(GetDrawStringWidth(text.c_str(), text.length()), Resources::getFont()->font_size)*d;
	double a = -this->rotate_angle;

	if (!this->isVisibleForWindowPos(p, s, a)) return;
	
	Color_RGB c = color*this->shield_ratio;
	DrawRotaString((int)p.x, (int)p.y, d, d, s.x / 2, s.y / 2, a, c.toColor(), 0, 0, text.c_str());
}

void Camera::drawStringOnWindowFixed(Vec2D pos_center, std::string text, Color_RGB color, double expansion) const {
	Vec2D s = Vec2D(GetDrawStringWidth(text.c_str(), text.length()), Resources::getFont()->font_size)*expansion;

	if (!this->isVisibleForWindowPos(pos_center, s)) return;

	Color_RGB c = color*this->shield_ratio;
	DrawRotaString((int)pos_center.x, (int)pos_center.y, expansion, expansion, s.x / 2, s.y / 2, 0.0, c.toColor(), 0, 0, text.c_str());
}

void Camera::drawStringOnWindowSwayed(Vec2D pos_center, std::string text, Color_RGB color, double expansion) const {
	Vec2D p = pos_center + this->sway;
	Vec2D s = Vec2D(GetDrawStringWidth(text.c_str(), text.length()), Resources::getFont()->font_size)*expansion;

	if (!this->isVisibleForWindowPos(p, s)) return;

	Color_RGB c = color*this->shield_ratio;
	DrawRotaString((int)p.x, (int)p.y, expansion, expansion, s.x / 2, s.y / 2, 0.0, c.toColor(), 0, 0, text.c_str());
}



void Camera::update(){
	if (this->move_count < this->move_count_max) {
		this->move_count++;
		this->pos_anchor_world = this->anchor_target_world_pos_start + (this->anchor_target_world_pos_goal - this->anchor_target_world_pos_start)*(1.0*this->move_count / this->move_count_max);
	}
	else if (this->move_count == this->move_count_max) {
		this->pos_anchor_world = this->anchor_target_world_pos_goal;
	}
	/*
	if (this->pos_anchor_world.x < this->pos_anchor_window.x) this->pos_anchor_world.x = this->pos_anchor_window.x;
	if (this->pos_anchor_world.y < this->pos_anchor_window.y) this->pos_anchor_world.y = this->pos_anchor_window.y;
	if (this->pos_anchor_world.x > this->worldSize.x  - this->pos_anchor_window.x) this->pos_anchor_world.x = this->worldSize.x  - this->pos_anchor_window.x;
	if (this->pos_anchor_world.y > this->worldSize.y - this->pos_anchor_window.y) this->pos_anchor_world.y = this->worldSize.y - this->pos_anchor_window.y;
	*/

	if (this->zoom_count < this->zoom_count_max) {
		this->zoom_count++;
		this->zoom_magnification = this->zoom_magnification_start + (this->zoom_magnification_end - this->zoom_magnification_start)*this->zoom_count / this->zoom_count_max;
	}
	else if (this->zoom_count == this->zoom_count_max) {
		this->zoom_magnification = this->zoom_magnification_end;
	}
	
	if (this->rotate_count < this->rotate_count_max) {
		this->rotate_count++;
		this->rotate_angle = this->rotate_angle_start + (this->rotate_angle_end - this->rotate_angle_start)*this->rotate_count / this->rotate_count_max;
	}
	else if (this->rotate_count == this->rotate_count_max) {
		this->rotate_angle = this->rotate_angle_end;
	}

	if (this->sway_count < this->sway_count_max) {
		this->sway_count++;
		if (this->sway_update_time == 0) {
			double p = M_PI*2*this->sway_count / this->sway_vib_rotate_freq;
			this->sway = Vec2D::fromPowAng(1.0*this->sway_power_max*(this->sway_count_max - this->sway_count) / this->sway_count_max*sin(p), this->sway_vib_trans_rad);
		}
		else {
			if (this->sway_count%this->sway_update_time == 0) {
				this->sway = Vec2D::getRandomInSquare(this->sway_power_max, this->sway_power_max);
			}
		}
	}
	else {
		this->sway = Vec2D(0.0, 0.0);
	}
}



bool Camera::isVisibleForWindowPos(Vec2D p) const {
	if (p.x < this->moveArea_world.leftup().x) return false;
	if (p.y < this->moveArea_world.leftup().y) return false;
	if (p.x > this->moveArea_world.rightdown().x) return false;
	if (p.y > this->moveArea_world.rightdown().y) return false;

	return true;
}

bool Camera::isVisibleForWindowPos(Vec2D p, Vec2D s) const {
	if (p.x < this->moveArea_world.leftup().x - s.x / 2) return false;
	if (p.y < this->moveArea_world.leftup().y - s.y / 2) return false;
	if (p.x > this->moveArea_world.rightdown().x + s.x / 2) return false;
	if (p.y > this->moveArea_world.rightdown().y + s.y / 2) return false;

	return true;
}

bool Camera::isVisibleForWindowPos(Vec2D p, Vec2D s, double ang) const {
	Vec2D p1 = p + Mat2D::rotation(ang)*(-s / 2);
	Vec2D p2 = p + Mat2D::rotation(ang)*Mat2D::scaling(-1.0, 1.0) * s / 2;
	Vec2D p3 = p + Mat2D::rotation(ang)*Mat2D::scaling(1.0, -1.0) * s / 2;
	Vec2D p4 = p + Mat2D::rotation(ang)*(s / 2);

	Vec2D w1 = Vec2D(0.0, 0.0);
	Vec2D w2 = Vec2D(this->drawArea_window.size().x, 0.0);
	Vec2D w3 = Vec2D(this->drawArea_window.size().x, this->drawArea_window.size().y);
	Vec2D w4 = Vec2D(0.0, this->drawArea_window.size().y);

	if (Vec2D::isCrossLine(p1, p2, w1, w2)) return true;
	if (Vec2D::isCrossLine(p1, p2, w2, w3)) return true;
	if (Vec2D::isCrossLine(p1, p2, w3, w4)) return true;
	if (Vec2D::isCrossLine(p1, p2, w4, w1)) return true;
	if (Vec2D::isCrossLine(p2, p3, w1, w2)) return true;
	if (Vec2D::isCrossLine(p2, p3, w2, w3)) return true;
	if (Vec2D::isCrossLine(p2, p3, w3, w4)) return true;
	if (Vec2D::isCrossLine(p2, p3, w4, w1)) return true;
	if (Vec2D::isCrossLine(p3, p4, w1, w2)) return true;
	if (Vec2D::isCrossLine(p3, p4, w2, w3)) return true;
	if (Vec2D::isCrossLine(p3, p4, w3, w4)) return true;
	if (Vec2D::isCrossLine(p3, p4, w4, w1)) return true;
	if (Vec2D::isCrossLine(p4, p1, w1, w2)) return true;
	if (Vec2D::isCrossLine(p4, p1, w2, w3)) return true;
	if (Vec2D::isCrossLine(p4, p1, w3, w4)) return true;
	if (Vec2D::isCrossLine(p4, p1, w4, w1)) return true;

	if (!Vec2D::isPointWholeInSquare(w1, w2, w3, w4, p1)) return false;
	if (!Vec2D::isPointWholeInSquare(w1, w2, w3, w4, p2)) return false;
	if (!Vec2D::isPointWholeInSquare(w1, w2, w3, w4, p3)) return false;
	if (!Vec2D::isPointWholeInSquare(w1, w2, w3, w4, p4)) return false;

	return true;
}

bool Camera::isVisibleForWorldPos(Vec2D pos_world) const
{
	return this->isVisibleForWindowPos(this->toWindowPosPxFromWorldPos(pos_world));
}

bool Camera::isVisibleForWorldPos(const std::shared_ptr<ObjectBase>& obj) const
{
	return this->isVisibleForWorldPos(obj->getWorldPosition(), obj->getSize(), obj->getRotateAngle());
}

bool Camera::isVisibleForWorldPos(Vec2D pos_world, Vec2D size) const
{
	return this->isVisibleForWindowPos(this->toWindowPosPxFromWorldPos(pos_world), size*this->zoom_magnification);
}

bool Camera::isVisibleForWorldPos(Vec2D pos_world, Vec2D size, double rot_angle_in_window) const
{
	return this->isVisibleForWindowPos(this->toWindowPosPxFromWorldPos(pos_world), size*this->zoom_magnification, rot_angle_in_window - this->rotate_angle);
}
