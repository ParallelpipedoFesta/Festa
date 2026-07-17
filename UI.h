#pragma once

#pragma once
#include "Festa.hpp"
#include "include/font.h"
using namespace Festa;


#define UI_NONE 0
#define UI_CENTERED 1
#define UI_LEFT 2
#define UI_RIGHT 3
#define UI_TOP UI_LEFT
#define UI_BOTTOM UI_RIGHT

#define UI_TEXT_CENTERED ivec2(UI_CENTERED, UI_CENTERED)

#define UI_BUTTON_NONE 0
#define UI_BUTTON_PRESSED 2
#define UI_BUTTON_CLICKED 0b01
#define UI_BUTTON_HOVERED 4


class UserInterface {
public:
	struct Character {
		Texture texture;
		uint w = 0, h = 0;
		int left = 0, bottom = 0, advance = 0;
		void load(FreetypeFont& font, uint ch) {
			const FreetypeFont::Character& c = font.LoadCharacter(ch);
			w = c.buf.width(), h = c.buf.height();
			texture.Generate(w, h, c.buf.data(), GL_RED, GL_RED, GL_UNSIGNED_BYTE);
			Texture::wrapping2D(GL_CLAMP_TO_EDGE);
			Texture::minFilter(GL_LINEAR);
			Texture::magFilter(GL_LINEAR);
			left = c.left, bottom = h - c.top, advance = c.advance;
		}
		~Character() {
			texture.Release();
		}
	};
	UserInterface() {
		LoadIcons();
		CompileShaders();
		DefaultStyle();
	}
	UserInterface(const GLWindow& window) {
		LoadIcons();
		CompileShaders();
		DefaultStyle();
		BindWindow(window);
	}
	Color& backgroundColor() {
		return mBackgroundColor;
	}
	const Color& backgroundColor()const {
		return mBackgroundColor;
	}
	void LoadStyle(const Path& file) {

	}
	int GetButton(const vec2& lt, const vec2& rb) {
		if (!window())return UI_BUTTON_NONE;
		vec2 cursor = window()->mouse.pos;
		return (int((lt.x < cursor.x && cursor.x < rb.x) && (lt.y < cursor.y && cursor.y < rb.y)) << 2) |
			(int(window()->mouse.left.pressed()) << 1) |
			int(window()->mouse.left.previous());
	}
	int ButtonArea(const vec2& center, const vec2& size, const Color& defaultColor = Color(255, 255, 255), const Color& hoveredColor = Color(128, 128, 128), const Color& pressedColor = Color(25, 25, 25), TextureMap* textureMap = 0, const vec2& textureOff = vec2(-1.0f)) {
		int code = GetButton(center - size * 0.5f, center + size * 0.5f);
		Color borderColor = defaultColor;
		if (code & UI_BUTTON_HOVERED) {
			borderColor = hoveredColor;
			if (code & UI_BUTTON_PRESSED)borderColor = pressedColor;
		}
		RoundRect(center, size, mBoxRadius, mLineThickness, Color(0, 0, 0, 0), borderColor, textureMap, textureOff);
		return code;
	}
	void DefaultStyle() {
		mBackgroundColor = Color(17, 12, 33, 200);
		mBorderColor = Color(0.7f, 0.7f, 0.7f, 0.7f);
		mBoxColor = Color(0.1f, 0.1f, 0.1f, 0.2f);
		//mBoxColor = Color(17, 12, 25, 200);
		mBoxRadius = 10.0f;
		mLineThickness = 3.0f;
		mLevelSizes = { 20, 30, 40, 50 };
	}
	void BindWindow(const GLWindow& window) {
		mWindow = &window;
		for (int i = 0; i < 2; i++) {
			mFrames[i].Generate(window.width(), window.height());
			mFrameTextures[i].FromRenderBuffer(mFrames[i]);
		}
	}
	const GLWindow* window()const {
		return mWindow;
	}
	vec2 GetCanvasSize()const {
		return mWindow->viewport.size();
	}
	void BeginRenderFrame() {
		mFrames[mCurrentFrame].Resize(mWindow->size());
		mFrames[mCurrentFrame].Begin();
	}
	uint GetLevelSize(int level) {
		return 0 <= level && level < mLevelSizes.size() ? mLevelSizes[level] : 0u;
	}
	void Text(const std::string& text, const vec2& pos, const Color& color, int level = 0, const ivec2& flags = ivec2(UI_NONE)) {
		RenderText(text, pos, color, GetLevelSize(level), flags);
	}
	void RenderText(const std::string& text, vec2 pos, const Color& color, uint size, const ivec2& flags = ivec2(UI_NONE)) {
		const std::wstring wideString = string2wstring(text);
		const float factor = float(size) / float(mFont->height());
		auto box = mFont->GetBox(wideString);
		switch (flags.x) {
		case UI_NONE:
			break;
		case UI_CENTERED:
			pos.x -= float(box.w) * 0.5f * factor;
			break;
		case UI_LEFT:
			break;
		case UI_RIGHT:
			pos.x -= float(box.w) * factor;
			break;
		default:
			break;
		}
		switch (flags.y) {
		case UI_NONE:
			break;
		case UI_CENTERED:
			pos.y += (-float(box.h) * 0.5f + float(box.top)) * factor;
			break;
		case UI_TOP:
			pos.y += float(box.top) * factor;
			break;
		case UI_BOTTOM:
			pos.y -= float(box.bottom()) * factor;
			break;
		default:
			break;
		}
		for (auto ch : wideString) {
			pos = RenderCharacter(ch, pos, vec2(factor), color);
		}
	}
	void RenderText3D(const Camera& camera, const std::string& text, const vec3& pos, const Color& color, uint size, const ivec2& flags = ivec2(UI_NONE)) {
		const vec3 ndc = camera.world2ndc(pos);
		if (ndc.z < -1.0f || ndc.z>1.0f)return;
		RenderText(text, mWindow->viewport.ndc2screen(ndc), color, size);
	}
	void ItemBox(const vec2& center, const vec2& size, const TextureMap* textureMap = 0, const vec2& textureOff = vec2(-1.0f), const Color& col = vec3(1.0f), bool border = true) {
		vec4 inner = mBoxColor.toVec4() * col.toVec4();
		RoundRect(center, size, mBoxRadius, mLineThickness, inner, border ? mBorderColor.toVec4() * col.toVec4() : vec4(0.0f), textureMap, textureOff);
	}
	void TexturedRect(const vec2& center, const vec2& size, const Texture& texture) {
		mRectRender.bind();
		mRectRender.set("model", CalcTransformation(center, size).toMatrix());
		mRectRender.set("color", mat4(1.0f));
		mRectRender.bindTexture("textureMap", 1, texture);
		RECT2.get().Draw();
	}
	void Rect(const vec2& center, const vec2& size, const Color& color) {
		mRectRender.bind();
		mRectRender.set("model", CalcTransformation(center, size).toMatrix());
		const vec4 col = color.toVec4();
		mRectRender.set("color", mat4(
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			col.x, col.y, col.z, col.w));
		RECT2.get().Draw();
	}
	void RoundRect(const vec2& center, const vec2& size, float radius, float borderThickness, const Color& fillColor, const Color& borderColor, const TextureMap* textureMap = 0, const vec2& textureOff = vec2(-1.0f)) {
		mRoundRectRender.bind();
		mRoundRectRender.set("model", CalcTransformation(center, size).toMatrix());
		mRoundRectRender.set("rectSize", size);
		mRoundRectRender.set("borderColor", borderColor.toVec4());
		mRoundRectRender.set("radius", radius);
		mRoundRectRender.set("borderThickness", borderThickness);
		if (textureMap) {
			mRoundRectRender.bindTexture("textureMap", 1, textureMap->GetTexture());
			mRoundRectRender.set("textureStride", textureMap->textureCoordStride());
			mRoundRectRender.set("textureOff", textureOff);
			mRoundRectRender.set("fillColor", vec4(0.0f));
		}
		else {
			mRoundRectRender.set("fillColor", fillColor.toVec4());
			mRoundRectRender.set("textureOff", vec2(-1.0f));
		}
		RECT2.get().Draw();
	}
	void RenderBlurred(int numGenerations) {
		mBlurRender.bind();
		Transformation trans;
		trans.setPosition(vec3(0.0f, 0.0f, 1.0f));
		trans.setScale(vec3(1.0f, -1.0f, 1.0f));
		mBlurRender.set("model", trans.toMatrix());
		mBlurRender.set("windowSize", GetCanvasSize());
		while (numGenerations--) {
			mCurrentFrame = 1 - mCurrentFrame;
			if (numGenerations == 0) {
				FBO::unbind();
			}
			else {
				mFrames[mCurrentFrame].Begin();
			}
			mBlurRender.bindTexture("frame", 0, mFrameTextures[1 - mCurrentFrame]);
			RECT2.get().Draw();
		}
	}
	void NextFrame() {
		mCurrentFrame = 1 - mCurrentFrame;
	}
	void Crosshair(const vec2& center, const vec2& size, const Color& color) {
		glDisable(GL_DEPTH_TEST);
		mRectRender.bind();
		mRectRender.set("model", CalcTransformation(center, size).toMatrix());
		mRectRender.bindTexture("textureMap", 0, mCrosshair);
		const vec4 col = color;
		mat4 mat(0.0f); mat[3] = col;
		mRectRender.set("color", mat);//color using alpha weight
		RECT2.get().Draw();
		glEnable(GL_DEPTH_TEST);
	}
	int InfoCard(const vec2& pos, const std::string& title, const std::list<std::string>& info) {
		// pos = left-bottom
		// white-bordered once mouse-hovered
		glDisable(GL_DEPTH_TEST);
		const float titleHeight = 40;
		const float textHeight = 20;
		const vec2 margin(10.0f, 10.0f);
		const std::string dot = "•";//draw dot shader using rect
		vec2 size(0.0f);
		auto box = mFont->GetBox(title);
		size.x = std::max(size.x, float(box.w) * titleHeight / float(mFont->height()));
		size.y += titleHeight + margin.y * 2.0f;
		for (const std::string& i : info) {
			box = mFont->GetBox(dot + i);
			size.x = std::max(size.x, float(box.w) * textHeight / float(mFont->height()));
			size.y += textHeight;
		}
		size += margin * 2.0f;
		Rect(vec2(pos.x + size.x * 0.5f, pos.y - size.y * 0.5f), size, mBackgroundColor);
		int ret = GetButton(vec2(pos.x, pos.y - size.y), vec2(pos.x + size.x, pos.y));
		vec2 p(pos.x + margin.x, pos.y - size.y + margin.y);
		RenderText(title, p, Color(255, 255, 255), uint(titleHeight), ivec2(UI_LEFT, UI_TOP));
		p.y += titleHeight + margin.y * 2.0f;
		//draw line
		for (const std::string& i : info) {
			RenderText(dot + i, p, Color(255, 255, 255), uint(textHeight), ivec2(UI_LEFT, UI_TOP));
			p.y += textHeight;
		}
		glEnable(GL_DEPTH_TEST);
		return ret;
	}
	void setFont(FreetypeFont* font) {
		mFont = font;
	}
	FreetypeFont& getFont() {
		return *mFont;
	}
private:
	FreetypeFont* mFont = 0;
	std::map<uint, Character> mCharacters;
	Color mBackgroundColor;
	Color mBorderColor;
	Color mBoxColor;
	float mBoxRadius;
	float mLineThickness;
	std::vector<uint> mLevelSizes;

	const GLWindow* mWindow = 0;
	FrameRenderBuffer mFrames[2];
	Texture mFrameTextures[2];
	int mCurrentFrame = 0;
	GLSLProgram mRoundRectRender;
	GLSLProgram mRectRender;
	GLSLProgram mBlurRender;
	Texture mCrosshair;

	Transformation CalcTransformation(const vec2& center, const vec2& size)const {
		const vec2 screenSize = GetCanvasSize();
		Transformation trans;
		trans.setScale(vec3(size.x / screenSize.x, size.y / screenSize.y, 1.0f));
		vec2 off = center / screenSize; off.x = off.x * 2.0f - 1.0f; off.y = 1.0f - off.y * 2.0f;
		trans.translate(vec3(off.x, off.y, 0.0f));
		return trans;
	}
	void LoadIcons() {
		mCrosshair.Generate(Image("icon/crosshair.png"));
	}
	void CompileShaders() {
		Shader rect_vs("shaders/UI/rect.vs");
		Shader rect_fs("shaders/UI/rect.fs");
		Shader roundRect_fs("shaders/UI/roundRect.fs");
		Shader blur_fs("shaders/UI/blur.fs");

		mRoundRectRender.attach({ rect_vs, roundRect_fs });
		mRectRender.attach({ rect_vs, rect_fs });
		mBlurRender.attach({ rect_vs, blur_fs });
	}
	const Character& GetCharacter(uint ch) {
		if (mCharacters.find(ch) != mCharacters.end())return mCharacters[ch];
		Character& c = mCharacters[ch];
		c.load(*mFont, ch);
		return c;
	}
	vec2 RenderCharacter(uint ch, const vec2& origin, const vec2& scale, const Color& color) {
		const Character& c = GetCharacter(ch);
		const vec2 center = origin + scale * vec2(float(c.left) + float(c.w) * 0.5f, float(c.bottom) - float(c.h) * 0.5f);
		const vec2 size = vec2(float(c.w), float(c.h)) * scale;
		mRectRender.bind();
		mRectRender.set("model", CalcTransformation(center, size).toMatrix());
		mRectRender.bindTexture("textureMap", 0, c.texture);

		const vec4 col = color.toVec4();
		mRectRender.set("color", mat4(
			0.0f, 0.0f, 0.0f, col.w,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			col.x, col.y, col.z, 0.0f));
		RECT2.get().Draw();

		return vec2(origin.x + c.advance * scale.x, origin.y);
	}

};


#define CODE_MOUSEKEY (1<<16)

class GameControl {
public://key bindings joystick
	int mKeysToMove[6];
	int mKeysToInteract[4];//pickup drop/throw interactWithGI/building useItem
	// sort inventory
	int mInventoryKeys[5];// open/close select divide
	int mConstructionKeys[5];// place rotate cancel fixCamera
	int mKeysToShoot[4];
	int mViewToggleKeys[10];
	int mToggleCamera;
	float mMouseSensitivity;
	float mCameraVel0 = 0.0f;
	float mCameraVel1 = 0.0f;
	GameControl() {
		LoadDefault();
	}
	void Load(const Path& file) {

		InitKeys();
	}
	void Save(const Path& file)const {

	}
	void LoadDefault() {
		mKeysToMove[0] = GLFW_KEY_W;
		mKeysToMove[1] = GLFW_KEY_A;
		mKeysToMove[2] = GLFW_KEY_S;
		mKeysToMove[3] = GLFW_KEY_D;
		mKeysToMove[4] = GLFW_KEY_LEFT_SHIFT; // sprint
		mKeysToMove[5] = GLFW_KEY_SPACE;// crouch & jump

		mKeysToInteract[0] = GLFW_KEY_E;
		mKeysToInteract[1] = GLFW_KEY_Q;
		mKeysToInteract[2] = GLFW_KEY_F;
		mKeysToInteract[3] = GLFW_MOUSE_BUTTON_LEFT | CODE_MOUSEKEY;
		//mKeysToInteract[3] = GLFW_KEY_F;

		mInventoryKeys[0] = GLFW_KEY_TAB;
		mInventoryKeys[1] = GLFW_MOUSE_BUTTON_LEFT | CODE_MOUSEKEY;
		mInventoryKeys[2] = GLFW_MOUSE_BUTTON_RIGHT | CODE_MOUSEKEY;

		mConstructionKeys[0] = GLFW_MOUSE_BUTTON_LEFT | CODE_MOUSEKEY;
		mConstructionKeys[1] = GLFW_KEY_R;
		mConstructionKeys[2] = GLFW_KEY_C;
		mConstructionKeys[3] = GLFW_KEY_F;

		mViewToggleKeys[0] = GLFW_KEY_ESCAPE;
		for (int i = 1; i < 10; i++)mViewToggleKeys[i] = GLFW_KEY_F1 + i - 1;

		mToggleCamera = GLFW_KEY_T;
		InitKeys();

		mMouseSensitivity = 0.1f;
		mCameraVel0 = 7.0f;
		mCameraVel1 = 15.0f;
	}
	void UpdateKey(const Window& window, int key) {
		mKeys[key].update(window.GetKey(key));
	}
	void Update(const Window& window) {
		for (auto& it : mKeys) {
			if (it.first & CODE_MOUSEKEY) {
				it.second.update(glfwGetMouseButton(window.ptr, it.first & 0xffff));
				continue;
			}
			it.second.update(window.GetKey(it.first));
		}
	}
	const ButtonStatus& GetKey(int key) {
		return mKeys[key];
	}
	void RegisterKey(int key) {
		mKeys.insert({ key, ButtonStatus() });
	}
	vec2 InteractToMove(const vec2& front, const vec2& left, bool& moved) {
		vec2 direction(0.0f);
		if (GetKey(mKeysToMove[0]).pressed())
			direction += front, moved = true;
		if (GetKey(mKeysToMove[1]).pressed())
			direction += left, moved = true;
		if (GetKey(mKeysToMove[2]).pressed())
			direction -= front, moved = true;
		if (GetKey(mKeysToMove[3]).pressed())
			direction -= left, moved = true;
		return direction;
	}
	vec3 InteractToMove3D(const vec3& front, const vec3& left, bool& moved) {
		vec3 direction(0.0f);
		if (GetKey(mKeysToMove[0]).pressed())
			direction += front, moved = true;
		if (GetKey(mKeysToMove[1]).pressed())
			direction += left, moved = true;
		if (GetKey(mKeysToMove[2]).pressed())
			direction -= front, moved = true;
		if (GetKey(mKeysToMove[3]).pressed())
			direction -= left, moved = true;
		return direction;
	}
	float GetCameraVelocity() {
		return GetKey(mKeysToMove[4]).pressed() ? mCameraVel1 : mCameraVel0;
	}
private:
	std::map<int, ButtonStatus> mKeys;
	void InitKeys() {
		mKeys.clear();
		for (int i = 0; i < 6; i++)RegisterKey(mKeysToMove[i]);
		for (int i = 0; i < 4; i++)RegisterKey(mKeysToInteract[i]);
		for (int i = 0; i < 5; i++)RegisterKey(mInventoryKeys[i]);
		for (int i = 0; i < 5; i++)RegisterKey(mConstructionKeys[i]);
		for (int i = 0; i < 10; i++)RegisterKey(mViewToggleKeys[i]);
		RegisterKey(mToggleCamera);
	}
};