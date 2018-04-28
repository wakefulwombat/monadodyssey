#pragma once

#include <memory>
#include "common.h"
#include "base_objectBase.h"

class Image {
private:
	int map_chip[12];
	int mousePointer;
	int targetMarker;
	int plane[10];
	int icon_arrival, icon_departure;

public:
	Image() { }
};

class BGM {
public:

};

class SE {
public:

};

class Font {
public:
	int font_size;

	Font();
};

class Resources {
private:
	static std::shared_ptr<Image> img;
	static std::shared_ptr<BGM> bgm;
	static std::shared_ptr<SE> se;
	static std::shared_ptr<Font> font;

public:
	static void initialize();
	static std::shared_ptr<Image> getImage() { return Resources::img; }
	static std::shared_ptr<BGM> getBGM() { return Resources::bgm; }
	static std::shared_ptr<SE> getSE() { return Resources::se; }
	static std::shared_ptr<Font> getFont() { return Resources::font; }
};