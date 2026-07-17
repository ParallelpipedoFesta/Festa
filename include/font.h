#pragma once

#include "resources.h"
#include <ft2build.h>
#include <sstream>
#include <unordered_map>
#include FT_FREETYPE_H  


namespace Festa {
	class FreetypeFont {
	public:
		struct Character {
			Image buf;
			int left = 0, top = 0, advance = 0;
			Character() {}
			int w()const {
				return buf.width();
			}
			int h()const {
				return buf.height();
			}
			int bottom()const {
				return h() - top;
			}
		};
		struct Box {
			int w = 0, h = 0, top = 0,_advance=0;
			Box() {}
			Box(const Character& ch)
				:w(ch.w()), h(ch.h()), top(ch.top) {
				_advance = ch.advance-w;
			}
			void operator+=(const Box& box) {
				w += _advance + box.w;
				_advance = box._advance;
				int bt = std::max(box.bottom(), bottom());
				top = std::max(top, box.top);
				h = top + bt;
			}
			int bottom()const {
				return h - top;
			}
			int advance()const {
				return w + _advance;
			}
		};
		static FT_Library ft;
		FreetypeFont() {}
		FreetypeFont(const Path& file, uint height, uint width = 0) {
			Load(file, height, width);
		}
		void Load(const Path& file, uint height, uint width = 0);
		uint width()const {
			return mWidth;
		}
		uint height()const {
			return mHeight;
		}
		void Resize(uint height, uint width = 0) {
			mHeight = height;
			mWidth = width;
			FT_Set_Pixel_Sizes(mFace, height, width);
			mCharacters.clear();
		}
		void Release() {
			FT_Done_Face(mFace);
			mFace = 0;
			mCharacters.clear();
			mWidth = 0;
			mHeight = 0;
		}
		~FreetypeFont() {
			Release();
		}
		const Character& LoadCharacter(uint ch) {
			if (mCharacters.find(ch) != mCharacters.end())return mCharacters[ch];
			if (FT_Load_Char(mFace, ch, FT_LOAD_RENDER))LOGGER.error("Failed to load freetype glyph");
			uint w = mFace->glyph->bitmap.width;
			uint h = mFace->glyph->bitmap.rows;
			Character& c = mCharacters[ch];

			c.buf = Image(mFace->glyph->bitmap.buffer, w, h, 1);
			c.left = mFace->glyph->bitmap_left;
			c.top = mFace->glyph->bitmap_top;
			c.advance = mFace->glyph->advance.x >> 6;
			return c;
		}
		int BlitCharacter(Image& img, const Character& c, const vec2& pos, const Color& color = vec3(0.0f)) {
			assert(img.channels() == 3);
			int w = c.buf.width(), h = c.buf.height();
			for (int i = 0; i < w; i++) {
				int u = int(pos.x) + i + c.left;
				if (u < 0 || u >= img.width())continue;
				for (int j = 0; j < h; j++) {
					int v = int(pos.y) + j - c.top;
					if (v < 0 || v >= img.height())continue;
					float a = float(c.buf.get(i, j, 0)) / 255.0f;
					img.SetColor(u, v, Color::mix(img.GetColor(u, v), color, a));
				}
			}
			return int(pos.x) + c.advance;
		}
		int BlitCharacter(Image& img, uint ch, const vec2& pos, const Color& color = vec3(0.0f)){
			return BlitCharacter(img, LoadCharacter(ch), pos, color);
		}
		void BlitText(Image& img, const std::string& text, const vec2& pos, const Color& color = vec3(0.0f)) {
			int x = int(pos.x);
			std::wstring wstr = string2wstring(text);
			for (uint i = 0; i < wstr.size(); i++)
				x = BlitCharacter(img, wstr[i], vec2(x,pos.y), color);
		}
		Box GetBox(const std::wstring& text) {
			Box ret;
			for (uint i = 0; i < text.size(); i++) {
				const Character& ch = LoadCharacter(text[i]);
				ret += Box(ch);
			}
			return ret;
		}
		Box GetBox(const std::string& text) {
			return GetBox(string2wstring(text));
		}
		Image* GetImage(const std::string& text, const Color& fg=vec3(0.0f), const Color& bg=vec3(1.0f)) {
			FreetypeFont::Box box = GetBox(text);
			Image* ret = new Image(bg, box.w, box.h);
			BlitText(*ret, text, vec2(0.0f, box.top), fg);
			return ret;
		}
	private:
		FT_Face mFace = 0;
		std::map<uint, Character> mCharacters;
		uint mWidth = 0;
		uint mHeight = 0;
	};
	class FontRender:public FreetypeFont {
	public:
		struct Character {
			Texture texture;
			uint w = 0, h = 0;
			int left = 0, bottom = 0, advance = 0;
			void init(FreetypeFont& font, uint ch) {
				const FreetypeFont::Character& c=font.LoadCharacter(ch);
				w = c.buf.width(), h = c.buf.height();
				texture.Generate(w, h, c.buf.data(), GL_RED, GL_RED, GL_UNSIGNED_BYTE);
				Texture::wrapping2D(GL_CLAMP_TO_EDGE);
				Texture::minFilter(GL_LINEAR);
				Texture::magFilter(GL_LINEAR);
				left = c.left, bottom = h-c.top, advance = c.advance;
			}
			~Character() {
				texture.Release();
			}
		};
		std::unordered_map<uint, Character> chars;
		vec2 origin;
		int mode = FESTA_ORIGIN;
		
		FontRender() { window = 0; origin = vec2(0.0f); }
		FontRender(const Window& _window, const Path& file, uint height, uint width = 0){
			_init(_window,file, height, width);
		}
		void setMode(int _mode) {
			mode = _mode;
		}
		void _init(const Window& _window, const Path& file, uint height, uint width = 0){
			window = &_window;
			origin = vec2(0.0f);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			Load(file, height, width);
		}
		void setOrigin(const vec2& pos) {
			origin = pos;
		}
		void render(const std::string& str, const Color& color = vec3(0.0f), const vec2& scale = vec2(1.0f)) {
			std::wstring wstr = string2wstring(str);
			for (auto i:wstr)
				render(i, color, scale);
		}
		void render(const std::string& str, const vec2& pos, const Color& color = vec3(0.0f), const vec2& scale = vec2(1.0f)) {
			origin = pos;
			//Box box = getBox(str);
			//origin.y += box.top;
			render(str, color, scale);
		}
		void render3D(const Camera& camera,
			const std::string& str, const vec3& pos, const Color& color = vec3(0.0f), const vec2& scale = vec2(1.0f)) {
			vec3 ndc = camera.world2ndc(pos);
			if (ndc.z < -1.0f || ndc.z>1.0f)return;
			origin = window->viewport.ndc2screen(ndc);
			render(str, color, scale);
		}
		
		~FontRender() {
			
		}
	private:
		const Window* window;
		Character& getChar(uint ch) {
			if (chars.find(ch) != chars.end())return chars[ch];
			Character& c = chars[ch];
			c.init(*this,ch);
			return c;
		}
		void render(uint ch, const Color& color = vec3(0.0f), const vec2& scale = vec2(1.0f)) {
			Character& c = getChar(ch);
			const vec2 size(float(c.w), float(c.h)),
				vsize=window->viewport.size();
			vec2 position;
			switch (mode) {
			/*case FESTA_LEFT_BOTTOM:
				position = origin +
					scale * vec2(float(c.left) + size.x / 2.0f,-size.y / 2.0f) -
					vec2(float(window->viewport.x), float(window->viewport.y));
				break;
			case FESTA_LEFT_TOP:
				position = origin +
					scale * vec2(float(c.left) + size.x / 2.0f, -float(c.h-c.bottom) + size.y / 2.0f) -
					vec2(float(window->viewport.x), float(window->viewport.y));
				break;*/
			case FESTA_ORIGIN:
				position = origin - window->viewport.pos() +
					scale * vec2(float(c.left) + size.x / 2.0f, float(c.bottom) - size.y / 2.0f);
				break;
			}
			
			position = vec2(position.x-vsize.x / 2.0f,vsize.y/2.0f-position.y)/(vsize/2.0f);
			mat4 trans =  translate4(vec3(position, 0.0f))*
				scale4(vec3(size * scale / vsize, 0.0f));
			vec3 col = color.toVec3();
			/*drawTexture(c.texture, trans, mat4(0.0f, 0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				col.x, col.y, col.z, 0.0f));*/
			origin.x += c.advance * scale.x;
		}
		void render(uint ch, const vec2& pos, const vec3& color = vec3(0.0f), const vec2& scale = vec2(1.0f)) {
			origin = pos;
			render(ch, color, scale);
		}	
	};


}

