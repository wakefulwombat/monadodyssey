#include "common.h"
#include "DxLib.h"
#include <math.h>

Color_RGB::Color_RGB() {
	this->r = 0;
	this->g = 0;
	this->b = 0;
}

Color_RGB::Color_RGB(int r, int g, int b) {
	this->r = r;
	this->g = g;
	this->b = b;
}

Color_RGB Color_RGB::operator*(const double &d) {
	Color_RGB c;

	c.r = (int)(d*this->r);
	c.g = (int)(d*this->g);
	c.b = (int)(d*this->b);

	return c;
}


Color_HSV::Color_HSV() {
	this->h = 0;
	this->s = 0;
	this->v = 0;
}

Color_HSV::Color_HSV(int h, int s, int v) {
	this->h = h;
	this->s = s;
	this->v = v;
}

Color_HSV Color_RGB::toHSV() {
	double h, s, v;
	double min, max;
	if ((this->r > this->g) && (this->r > this->b)) {
		if (this->g > this->b) {
			max = this->g;
			min = this->b;
		}
		else {
			min = this->g;
			max = this->b;
		}
		h = 60.0 *((this->g - this->b) / (max - min));
	}
	if ((this->g > this->r) && (this->g > this->b)) {
		if (this->r > this->b) {
			max = this->r;
			min = this->b;
		}
		else {
			min = this->r;
			max = this->b;
		}
		h = 60.0 *((this->b - this->r) / (max - min)) + 120.0;
	}
	if ((this->b > this->r) && (this->b > this->g)) {
		
		if (this->r > this->g) {
			max = this->r;
			min = this->g;
		}
		else {
			min = this->r;
			max = this->g;
		}
		h = 60.0 *((this->r - this->g) / (max - min)) + 240.0;
	}
	if (h < 0) h += 360.0;

	s = (max - min) / max*255.0;
	v = max;

	return Color_HSV((int)h, (int)s, (int)v);
}

Color_RGB Color_HSV::toRGB() {
	double r, g, b;
	double min, max;

	max = this->v;
	min = max - (this->s / 255)*max;

	if (this->h <= 60) {
		r = max;
		g = (this->h / 60)*(max - min) + min;
		b = min;
	}
	else if (this->h <= 120) {
		r = ((120.0 - this->h) / 60)*(max - min) + min;
		g = max;
		b = min;
	}
	else if (this->h <= 180) {
		r = min;
		g = max;
		b = ((this->h - 120.0) / 60)*(max - min) + min;
	}
	else if (this->h <= 240) {
		r = min;
		g = ((240.0 - this->h) / 60)*(max - min) + min;
		b = max;
	}
	else if (this->h <= 300) {
		r = ((this->h - 240.0) / 60)*(max - min) + min;
		g = min;
		b = max;
	}
	else {
		r = max;
		g = min;
		b = ((360.0 - this->h) / 60)*(max - min) + min;
	}

	return Color_RGB((int)r, (int)g, (int)b);
}



Color_HSV Color_HSV::rotate(int addAngle) {
	Color_HSV tmp;

	int h = this->h + addAngle;
	while (h < 0) h += 360;
	while (h > 360) h -= 360;

	tmp.h = h;
	tmp.s = this->s;
	tmp.v = this->v;

	return tmp;
}

Time24 Time24::operator+(const Time24& obj) {
	Time24 time;

	time.day = this->day + obj.day;
	time.hour = this->hour + obj.hour;
	time.minute = this->minute + obj.minute;
	time.milli_second = this->milli_second + obj.milli_second;

	time.checkOverTime();

	return time;
}

Time24 Time24::operator-(const Time24& obj) {
	Time24 time;

	time.day = this->day - obj.day;
	time.hour = this->hour - obj.hour;
	time.minute = this->minute - obj.minute;
	time.milli_second = this->milli_second - obj.milli_second;

	time.checkUnderTime();

	return time;
}

Time24 Time24::operator+(const int& obj) {
	Time24 time;

	time.day = this->day;
	time.hour = this->hour;
	time.minute = this->minute;
	time.milli_second = this->milli_second + obj;

	time.checkOverTime();

	return time;
}

Time24 Time24::operator-(const int& obj) {
	Time24 time;

	time.day = this->day;
	time.hour = this->hour;
	time.minute = this->minute;
	time.milli_second = this->milli_second - obj;

	time.checkUnderTime();

	return time;
}

std::string Time24::toString(std::string separator, bool show_sec) const {
	std::string res = "";
	std::string s;

	s = std::to_string(this->hour);
	if (s.length() == 1) s = "0" + s;
	res += s;
	res += separator;

	s = std::to_string(this->minute);
	if (s.length() == 1) s = "0" + s;
	res += s;

	if (show_sec) {
		res += separator;

		s = std::to_string(this->milli_second / 1000);
		if (s.length() == 1) s = "0" + s;
		res += s;
	}

	return res;
}


Vec2D::Vec2D() {
	this->x = 0.000000;
	this->y = 0.000000;
}

Vec2D::Vec2D(double x, double y) {
	this->x = x;
	this->y = y;
}

Vec2D Vec2D::operator+(const Vec2D& obj) const {
	Vec2D tmp;
	tmp.x = this->x + obj.x;
	tmp.y = this->y + obj.y;

	return tmp;
}

Vec2D Vec2D::operator-() const {
	Vec2D tmp;
	tmp.x = -this->x;
	tmp.y = -this->y;

	return tmp;
}

Vec2D Vec2D::operator-(const Vec2D& obj) const {
	Vec2D tmp;
	tmp.x = this->x - obj.x;
	tmp.y = this->y - obj.y;

	return tmp;
}

Vec2D &Vec2D::operator+=(const Vec2D& obj) {
	this->x += obj.x;
	this->y += obj.y;
	return *this;
}

Vec2D &Vec2D::operator-=(const Vec2D& obj) {
	this->x -= obj.x;
	this->y -= obj.y;
	return *this;
}

Vec2D Vec2D::operator*(const double& d) const {
	Vec2D tmp;
	tmp.x = d * this->x;
	tmp.y = d * this->y;

	return tmp;
}

double Vec2D::operator*(const Vec2D& obj) const {
	return this->x*obj.x + this->y*obj.y;
}

Vec2D Vec2D::operator/(const double& d) const {
	Vec2D tmp;
	tmp.x = this->x / d;
	tmp.y = this->y / d;

	return tmp;
}

bool Vec2D::operator==(const Vec2D& obj) const {
	return (((int)this->x == (int)obj.x) && ((int)this->y == (int)obj.y));
}

bool Vec2D::operator!=(const Vec2D& obj) const {
	return (((int)this->x != (int)obj.x) || ((int)this->y != (int)obj.y));
}

double Vec2D::toPower() const {
	return this->x*this->x + this->y*this->y;
}

double Vec2D::toNorm() const {
	return sqrt(this->x*this->x + this->y*this->y);
}

double Vec2D::toAngle() const {
	return atan2(this->y, this->x);
}

Direction4 Vec2D::toDirection4(double neutral_min) const
{
	if (this->toNorm() < neutral_min) return Direction4::NEUTRAL;

	double ang = this->toAngle();
	if (ang < M_PI / 4) return Direction4::RIGHT;
	else if (ang < M_PI * 3 / 4) return Direction4::DOWN;
	else if (ang < M_PI * 5 / 4) return Direction4::LEFT;
	else if (ang < M_PI * 7 / 4) return Direction4::UP;
	else return Direction4::RIGHT;
}

Direction8 Vec2D::toDirection8(double neutral_min) const
{
	if (this->toNorm() < neutral_min) return Direction8::NEUTRAL;

	double ang = this->toAngle();
	if (ang < M_PI / 8) return Direction8::RIGHT;
	else if (ang < M_PI * 3 / 8) return Direction8::RIGHT_DOWN;
	else if (ang < M_PI * 5 / 8) return Direction8::DOWN;
	else if (ang < M_PI * 7 / 8) return Direction8::LEFT_DOWN;
	else if (ang < M_PI * 9 / 8) return Direction8::LEFT;
	else if (ang < M_PI * 11 / 8) return Direction8::LEFT_UP;
	else if (ang < M_PI * 13 / 8) return Direction8::UP;
	else if (ang < M_PI * 15 / 8) return Direction8::RIGHT_UP;
	else return Direction8::RIGHT;
}

double Vec2D::getPowerFrom(Vec2D origin) const {
	return (this->x - origin.x)*(this->x - origin.x) + (this->y - origin.y)*(this->y - origin.y);
}

double Vec2D::getNorm(Vec2D origin) const {
	return sqrt((this->x - origin.x)*(this->x - origin.x) + (this->y - origin.y)*(this->y - origin.y));
}

double Vec2D::getAngle(Vec2D origin) const {
	return atan2(this->y - origin.y, this->x - origin.x);
}

double Vec2D::dot(Vec2D vec) const {
	return this->x*vec.x + this->y*vec.y;
}

double Vec2D::cross(Vec2D vec) const {
	return this->x*vec.y - this->y*vec.x;
}

Vec2D Vec2D::fromXY(double x, double y) {
	Vec2D tmp;
	tmp.x = x;
	tmp.y = y;
	return tmp;
}

Vec2D Vec2D::fromPowAng(double pow, double ang) {
	Vec2D tmp;
	tmp.x = pow * cos(ang);
	tmp.y = pow * sin(ang);
	return tmp;
}

double Vec2D::getDistance(Vec2D p2) const {
	return sqrt(((this->x - p2.x)*(this->x - p2.x) + (this->y - p2.y)*(this->y - p2.y)));
}

bool Vec2D::isInSquareBox(Vec2D leftup, Vec2D rightdown) const
{
	if (this->x < leftup.x) return false;
	if (this->y < leftup.y) return false;
	if (this->x > rightdown.x) return false;
	if (this->y > rightdown.y) return false;
	return true;
}

bool Vec2D::isInSquareSize(Vec2D center, Vec2D size) const
{
	if (this->x < center.x - size.x / 2) return false;
	if (this->x > center.x + size.x / 2) return false;
	if (this->y < center.y - size.y / 2) return false;
	if (this->y > center.y + size.y / 2) return false;
	return true;
}

Vec2D Vec2D::getRandomOnCircle(double r) {
	Vec2D tmp;
	double angle = M_PI * 2.0 * GetRand(3600) / 360;

	tmp.x = r * cos(angle);
	tmp.y = r * sin(angle);

	return tmp;
}

Vec2D Vec2D::getRandomInCircle(double r) {
	Vec2D tmp;
	double angle = M_PI * 2.0 * GetRand(3600) / 360;
	double expansion = 1.0*GetRand(1000) / 1000.0;

	tmp.x = expansion * r*cos(angle);
	tmp.y = expansion * r*sin(angle);

	return tmp;
}

Vec2D Vec2D::getRandomOnSquare(int width_half, int height_half) {
	Vec2D tmp;

	int val = GetRand(2 * width_half + 2 * height_half);
	if (val <= width_half) {
		tmp.x = val;
		tmp.y = -height_half;
	}
	else if (val <= width_half + height_half) {
		tmp.x = width_half;
		tmp.y = val - width_half;
	}
	else if (val <= 2 * width_half + height_half) {
		tmp.x = val - width_half - height_half;
		tmp.y = height_half;
	}
	else {
		tmp.x = -width_half;
		tmp.y = val - 2 * width_half - height_half;
	}

	return tmp;
}

Vec2D Vec2D::getRandomInSquare(int width_half, int height_half) {
	Vec2D tmp;

	tmp.x = GetRand(2 * width_half + 1) - width_half - 1;
	tmp.y = GetRand(2 * height_half + 1) - height_half - 1;

	return tmp;
}

bool Vec2D::isCrossLine(Vec2D p1_s, Vec2D p1_e, Vec2D p2_s, Vec2D p2_e) {
	double ta = (p2_s.x - p2_e.x) * (p1_s.y - p2_s.y) + (p2_s.y - p2_e.y) * (p2_s.x - p1_s.x);
	double tc = (p1_s.x - p1_e.x) * (p2_s.y - p1_s.y) + (p1_s.y - p1_e.y) * (p1_s.x - p2_s.x);
	double td = (p1_s.x - p1_e.x) * (p2_e.y - p1_s.y) + (p1_s.y - p1_e.y) * (p1_s.x - p2_e.x);
	double tb = (p2_s.x - p2_e.x) * (p1_e.y - p2_s.y) + (p2_s.y - p2_e.y) * (p2_s.x - p1_e.x);

	return tc * td < 0 && ta * tb < 0;
}

bool Vec2D::isInSquare(Vec2D leftup, Vec2D rightdown, Vec2D p) {
	if (p.x < leftup.x) return false;
	if (p.y < leftup.y) return false;
	if (p.x > rightdown.x) return false;
	if (p.y > rightdown.y) return false;
	return true;
}

bool Vec2D::isPointWholeInSquare(Vec2D p1_lu, Vec2D p1_ru, Vec2D p1_rd, Vec2D p1_ld, Vec2D p2) {
	double d1 = (p2 - p1_lu).cross(p1_ru - p1_lu);
	double d2 = (p2 - p1_ru).cross(p1_rd - p1_ru);
	double d3 = (p2 - p1_rd).cross(p1_ld - p1_rd);
	double d4 = (p2 - p1_ld).cross(p1_lu - p1_ld);

	if (d1 < 0.0) {
		if (d2 > 0.0) return false;
		if (d3 > 0.0) return false;
		if (d4 > 0.0) return false;
	}
	else {
		if (d2 < 0.0) return false;
		if (d3 < 0.0) return false;
		if (d4 < 0.0) return false;
	}
	return true;
}



Mat2D::Mat2D() {
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			this->matrix[x][y] = 0.0;
		}
	}
}

Mat2D::Mat2D(int m00, int m10, int m01, int m11) {
	this->matrix[0][0] = m00;
	this->matrix[0][1] = m01;
	this->matrix[1][0] = m10;
	this->matrix[1][1] = m11;
}

Vec2D Mat2D::operator*(const Vec2D& obj) const {
	Vec2D tmp;

	tmp.x = this->matrix[0][0] * obj.x + this->matrix[1][0] * obj.y;
	tmp.y = this->matrix[0][1] * obj.x + this->matrix[1][1] * obj.y;

	return tmp;
}

Mat2D Mat2D::operator+(const Mat2D& obj) const {
	Mat2D tmp;
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			tmp.matrix[x][y] = this->matrix[x][y] + obj.matrix[x][y];
		}
	}
	return tmp;
}

Mat2D Mat2D::operator-(const Mat2D& obj) const {
	Mat2D tmp;
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			tmp.matrix[x][y] = this->matrix[x][y] - obj.matrix[x][y];
		}
	}
	return tmp;
}

Mat2D Mat2D::operator*(const Mat2D& obj) const {
	Mat2D tmp;
	tmp.matrix[0][0] = this->matrix[0][0] * obj.matrix[0][0] + this->matrix[1][0] * obj.matrix[0][1];
	tmp.matrix[1][0] = this->matrix[0][0] * obj.matrix[1][0] + this->matrix[1][0] * obj.matrix[1][1];
	tmp.matrix[0][1] = this->matrix[0][1] * obj.matrix[0][0] + this->matrix[1][1] * obj.matrix[0][1];
	tmp.matrix[1][1] = this->matrix[0][1] * obj.matrix[1][0] + this->matrix[1][1] * obj.matrix[1][1];
	return tmp;
}

Mat2D Mat2D::init_rotation(double angle) {
	Mat2D tmp;
	tmp.matrix[0][0] = cos(angle);
	tmp.matrix[1][0] = -sin(angle);
	tmp.matrix[0][1] = sin(angle);
	tmp.matrix[1][1] = cos(angle);
	return tmp;
}

Mat2D Mat2D::init_scaling(double x, double y) {
	Mat2D tmp;
	tmp.matrix[0][0] = x;
	tmp.matrix[1][0] = 0.0;
	tmp.matrix[0][1] = 0.0;
	tmp.matrix[1][1] = y;
	return tmp;
}

Mat2D Mat2D::init_stretch(double x, double y) {
	Mat2D tmp;
	tmp.matrix[0][0] = 0.0;
	tmp.matrix[1][0] = x;
	tmp.matrix[0][1] = y;
	tmp.matrix[1][1] = 0.0;
	return tmp;
}

Vec2D Mat2D::trans(Vec2D vec, Vec2D origin) const {
	Vec2D tmp;

	tmp.x = this->matrix[0][0] * (vec.x - origin.x) + this->matrix[1][0] * (vec.y - origin.y) + origin.x;
	tmp.y = this->matrix[0][1] * (vec.x - origin.x) + this->matrix[1][1] * (vec.y - origin.y) + origin.y;

	return tmp;
}

Mat2D Mat2D::rotation(double angle) {
	Mat2D mat;
	return mat.init_rotation(angle);
}

Vec2D Mat2D::rotation(double angle, Vec2D vec) {
	Mat2D mat;
	return mat.init_rotation(angle)*vec;
}

Vec2D Mat2D::rotation(double angle, Vec2D vec, Vec2D origin) {
	Mat2D mat;
	return mat.init_rotation(angle)*(vec - origin) + origin;
}

Mat2D Mat2D::scaling(double x, double y) {
	Mat2D mat;
	return mat.init_scaling(x, y);
}

Vec2D Mat2D::scaling(double x, double y, Vec2D vec) {
	Mat2D mat;
	return mat.init_scaling(x, y)*vec;
}

Vec2D Mat2D::scaling(double x, double y, Vec2D vec, Vec2D origin) {
	Mat2D mat;
	return mat.init_scaling(x, y)*(vec - origin) + origin;
}

Mat2D Mat2D::stretch(double x, double y) {
	Mat2D mat;
	return mat.init_stretch(x, y);
}

Vec2D Mat2D::stretch(double x, double y, Vec2D vec) {
	Mat2D mat;
	return mat.init_stretch(x, y)*vec;
}

Vec2D Mat2D::stretch(double x, double y, Vec2D vec, Vec2D origin) {
	Mat2D mat;
	return mat.init_stretch(x, y)*(vec - origin) + origin;
}


int Round(double x){
	return (int)(x < 0.0 ? x - 0.4999 : x + 0.4999);
}


Direction4 Vec2D::toDirection4(Vec2D v, double neutral_min)
{
	if (v.toNorm() < neutral_min) return Direction4::NEUTRAL;

	double ang = atan2(v.y, v.x);
	if (ang < M_PI / 4) return Direction4::RIGHT;
	else if (ang < M_PI * 3 / 4) return Direction4::DOWN;
	else if (ang < M_PI * 5 / 4) return Direction4::LEFT;
	else if (ang < M_PI * 7 / 4) return Direction4::UP;
	else return Direction4::RIGHT;
}

Direction8 Vec2D::toDirection8(Vec2D v, double neutral_min)
{
	if (v.toNorm() < neutral_min) return Direction8::NEUTRAL;

	double ang = atan2(v.y, v.x);
	if (ang < M_PI / 8) return Direction8::RIGHT;
	else if (ang < M_PI * 3 / 8) return Direction8::RIGHT_DOWN;
	else if (ang < M_PI * 5 / 8) return Direction8::DOWN;
	else if (ang < M_PI * 7 / 8) return Direction8::LEFT_DOWN;
	else if (ang < M_PI * 9 / 8) return Direction8::LEFT;
	else if (ang < M_PI * 11 / 8) return Direction8::LEFT_UP;
	else if (ang < M_PI * 13 / 8) return Direction8::UP;
	else if (ang < M_PI * 15 / 8) return Direction8::RIGHT_UP;
	else return Direction8::RIGHT;
}
