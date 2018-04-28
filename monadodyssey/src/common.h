#pragma once
#include "DxLib.h"
#include <string>

enum class Direction4 {
	RIGHT,
	DOWN,
	LEFT,
	UP,
	NEUTRAL
};

enum class Direction8 {
	RIGHT,
	RIGHT_DOWN,
	DOWN,
	LEFT_DOWN,
	LEFT,
	LEFT_UP,
	UP,
	RIGHT_UP,
	NEUTRAL
};

class Color_HSV;

class Color_RGB {
public:
	int r, g, b;

	Color_RGB();
	Color_RGB(int r, int g, int b);
	Color_RGB operator*(const double &d);

	Color_HSV toHSV();
	unsigned int toColor() { return GetColor(this->r, this->g, this->b); }
};

class Color_HSV {
public:
	int h, s, v;

	Color_HSV();
	Color_HSV(int h, int s, int v);

	Color_RGB toRGB();
	Color_HSV rotate(int addAngle);
};

class Time24 {
private:
	void checkOverTime() { while (this->milli_second >= 60*1000) { this->milli_second -= 60*1000; this->minute++; } while (this->minute >= 60) { this->minute -= 60; this->hour++; } while (this->hour >= 24) { this->hour -= 24; this->day++; } }
	void checkUnderTime() { while (this->milli_second < 0) { this->milli_second += 60*1000; this->minute--; } while (this->minute < 0) { this->minute += 60; this->hour--; } while (this->hour < 0) { this->hour += 24; this->day--; } if (day < 0) { this->day = 0; this->hour = 0; this->minute = 0; this->milli_second = 0; } }

public:
	int day;
	int hour, minute, milli_second;

	Time24(int hour = 0, int minute = 0, int second = 0) { this->day = 0; this->hour = hour; this->minute = minute; this->milli_second = second; this->checkOverTime(); this->checkUnderTime(); }
	Time24 operator+(const Time24& obj);
	Time24& operator+=(const Time24& obj) { this->day += obj.day; this->hour += obj.hour; this->minute += obj.minute; this->milli_second += obj.milli_second; this->checkOverTime(); return (*this); }
	Time24 operator-(const Time24& obj);
	Time24& operator-=(const Time24& obj) { this->day -= obj.day; this->hour -= obj.hour; this->minute -= obj.minute; this->milli_second -= obj.milli_second; this->checkUnderTime(); return (*this); }
	Time24 operator+(const int& obj);
	Time24& operator+=(const int& obj) { this->milli_second += obj; this->checkOverTime(); return (*this); }
	Time24 operator-(const int& obj);
	Time24& operator-=(const int& obj) { this->milli_second -= obj; this->checkUnderTime(); return (*this); }
	bool operator<(const Time24& obj) { return this->toSecond() < obj.toSecond(); }
	bool operator>(const Time24& obj) { return this->toSecond() > obj.toSecond(); }
	long toSecond() const { return (24 * 60 * 60 * this->day + 60 * 60 * this->hour + 60 * this->minute + this->milli_second /1000); }
	void addSecond(long milli_sec) { this->milli_second += milli_sec; if (milli_sec >= 0) { this->checkOverTime(); } else { this->checkUnderTime(); } }
	void addMinute(long min) { if (min >= 0) { this->minute += min; this->checkOverTime(); } else { this->minute -= min; this->checkUnderTime(); } }
	std::string toString(std::string separator = ":", bool show_sec = false) const;
	bool isSameHourAndMinute(Time24 obj) { return ((this->hour == obj.hour) && (this->minute == obj.minute)); }
};

class Vec2D {
public:
	double x, y;
	Vec2D();
	Vec2D(double x, double y);
	Vec2D operator+(const Vec2D& obj) const;
	Vec2D& operator+=(const Vec2D& obj);
	Vec2D operator-() const;
	Vec2D operator-(const Vec2D& obj) const;
	Vec2D& operator-=(const Vec2D& obj);
	Vec2D operator*(const double& d) const;
	double operator*(const Vec2D& obj) const;
	Vec2D operator/(const double& d) const;
	bool operator==(const Vec2D& obj) const;
	bool operator!=(const Vec2D& obj) const;

	double getDistance(const Vec2D p2) const;
	bool isInSquareBox(const Vec2D leftup, const Vec2D rightdown) const;
	bool isInSquareSize(const Vec2D center, const Vec2D size) const;
	double toPower() const;
	double toNorm() const;
	double toAngle() const;
	Direction4 toDirection4(double neutral_min = 0.01) const;
	Direction8 toDirection8(double neutral_min = 0.01) const;
	double getPowerFrom(Vec2D origin) const;
	double getNorm(Vec2D origin) const;
	double getAngle(Vec2D origin) const;
	double dot(Vec2D vec) const;
	double cross(Vec2D vec) const;
	static Vec2D fromXY(double x, double y);
	static Vec2D fromPowAng(double pow, double ang);
	static Vec2D getRandomOnCircle(double r);
	static Vec2D getRandomInCircle(double r);
	static Vec2D getRandomOnSquare(int width_half, int height_half);
	static Vec2D getRandomInSquare(int width_half, int height_half);
	static bool isCrossLine(Vec2D p1_start, Vec2D p1_end, Vec2D p2_start, Vec2D p2_end);
	static bool isInSquare(Vec2D leftup, Vec2D rightdown, Vec2D p);
	static bool isPointWholeInSquare(Vec2D p1_lu, Vec2D p1_ru, Vec2D p1_rd, Vec2D p1_ld, Vec2D p2);
	static double getDistance(Vec2D v1, Vec2D v2) { (v1 - v2).toNorm(); }
	static double cross(Vec2D v1, Vec2D v2) { return (v1.x*v2.y - v1.y*v2.x); }
	static Direction4 toDirection4(Vec2D v, double neutral_min = 0.1);
	static Direction8 toDirection8(Vec2D v, double neutral_min = 0.1);
};

class Mat2D {
public:
	double matrix[2][2];
	Mat2D();
	Mat2D(int m00, int m10, int m01, int m11);
	Vec2D operator*(const Vec2D& obj) const;
	Mat2D operator*(const Mat2D& obj) const;
	Mat2D operator+(const Mat2D& obj) const;
	Mat2D operator-(const Mat2D& obj) const;

	Mat2D init_rotation(double angle);
	Mat2D init_scaling(double x, double y);
	Mat2D init_stretch(double x, double y);

	Vec2D trans(Vec2D vec, Vec2D origin) const;

	static Mat2D rotation(double angle);
	static Vec2D rotation(double angle, Vec2D vec);
	static Vec2D rotation(double angle, Vec2D vec, Vec2D origin);
	static Mat2D scaling(double x, double y);
	static Vec2D scaling(double x, double y, Vec2D vec);
	static Vec2D scaling(double x, double y, Vec2D vec, Vec2D origin);
	static Mat2D stretch(double x, double y);
	static Vec2D stretch(double x, double y, Vec2D vec);
	static Vec2D stretch(double x, double y, Vec2D vec, Vec2D origin);
};

int Round(double x);

const static double M_PI = 3.14159265358979;
const static Vec2D WINDOW_SIZE = Vec2D(960, 720);