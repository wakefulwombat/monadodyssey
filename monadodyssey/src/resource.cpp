#include "resource.h"
#include "DxLib.h"

std::shared_ptr<Image> Resources::img;
std::shared_ptr<BGM> Resources::bgm;
std::shared_ptr<SE> Resources::se;
std::shared_ptr<Font> Resources::font;

void Resources::initialize() {
	Resources::img = std::make_shared<Image>();
	Resources::bgm = std::make_shared<BGM>();
	Resources::se = std::make_shared<SE>();
	Resources::font = std::make_shared<Font>();
}

Font::Font() {
	this->font_size = 30;
	SetFontSize(this->font_size);
}