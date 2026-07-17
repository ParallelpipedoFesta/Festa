#pragma once
#include "App.h"
#include "Vacuum.h"
#include "include/fuman.h"

class TutorialPage {
public:
	typedef FreetypeFont::Box Box;
	struct Expr {
		//ivec2 origin = ivec2(0);
		Box box;
		std::string name;
		char token = 0;
		std::vector<Expr> ch;
		Expr() {}
		Expr(const std::string& str, const Box& b) :name(str), box(b) {}
		void put(const ivec2& pos, Image& img, FreetypeFont& font) {
			switch (token) {
			case 0:
				font.BlitText(img, name, vec2(pos.x, pos.y), Color(255));
				break;
			case '/':
				break;
			case 'm':
				break;
			}
		}
	};
	struct Composition {
		std::list<Expr> expr;
		Box box;
		void add(const Expr& e) {
			expr.push_back(e);
			box += e.box;
		}
	};
	static FreetypeFont* font;
	TutorialPage() {}
	void Render(const Transformation& trans)const {
		Text3D::shader.bind();
		Text3D::shader.set("model", trans.toMatrix());
		Text3D::shader.bindTexture("textMap", 0, mTexture);
		Text3D::shader.set("Diffuse", vec3(0.0f));
		RECT332.get().Draw();
	}
	void Parse(const ivec2& imSize, int lineHeight, const std::string& text, size_t& pos) {
		if (!font)return;
		Box box;
		std::list<Composition> lines;
		Precalculate(box, lines, lineHeight, imSize, text, pos);
		Image img(Color(0), imSize.x, imSize.y);
		ivec2 origin(0, lineHeight);
		for (Composition& line : lines) {
			for (Expr& expr : line.expr) {
				expr.put(origin, img, *font);
				origin.x += expr.box.advance();
			}
			origin.x = 0;
			origin.y += lineHeight;
		}
		// img.show("img");
		mTexture.Generate(img);
	}
private:
	Texture mTexture;
	//std::list<> mCircuit;
	void Precalculate(Box& box, std::list<Composition>& lines,int lineHeight, const ivec2& imSize, const std::string& text, size_t& pos) {
		if (!font)return;
		//while (pos < text.size() && text[pos] == ' ')pos++;
		size_t i = pos;
		if (lineHeight + lineHeight > imSize.y)return;
		lines.push_back(Composition());
		while (pos < text.size()) {
			//std::cout << text[pos];
			Box b;
			bool wordEnd = false;
			std::string prefix = "";
			switch (text[pos]) {
			case ' ':
				wordEnd = true;
				prefix = " ";
				break;
			case '{':
				wordEnd = true;
				break;
			case '}':
				if (text[i] == '{')wordEnd = true;
				break;
			case '\n':
				wordEnd = true;
				prefix = " ";
				break;
			}
			if (!wordEnd) {
				pos++;
				continue;
			}
			const std::string str = prefix+text.substr(i, pos - i);
			b = font->GetBox(str);
			if (b.w + lines.back().box.w > imSize.x) {
				if ((lines.size()+2) * lineHeight > imSize.y) {
					pos = i;
					return;
				}
				lines.push_back(Composition());
			}
			//while(text[pos]==' ')pos++
			switch (text[pos]) {
			case ' ': {
				i = pos + 1;
				lines.back().add(Expr(str, b));
				break;
			}
			case '{':
				i = pos;
				lines.back().add(Expr(str, b));
				break;
			case '}':
				i = pos + 1;
				lines.back().add(ParseExpression(text.substr(i+1, pos - i - 1)));
				break;
			case '\n':
				i = pos + 1;
				lines.back().add(Expr(str, b));
				if (i < text.size() && text[i] == ' ') {
					if ((lines.size() + 2) * lineHeight > imSize.y) {
						pos = i;
						return;
					}
					lines.push_back(Composition());
				}
				break;
			}
			pos++;
		}
		if (i < text.size()) {
			const std::string str = " " + text.substr(i, pos - i);
			Box b = font->GetBox(str);
			if (b.w + lines.back().box.w > imSize.x) {
				if ((lines.size() + 2) * lineHeight > imSize.y) {
					pos = i;
					return;
				}
				lines.push_back(Composition());
			}
			lines.back().add(Expr(str, b));
		}
	}
	Expr ParseExpression(const std::string& str) {
		std::cout << "parsing expr " << str << "\n";
		size_t i = 0;
		size_t j = 0;
		std::stack<Expr> st; 
		while (i < str.size()) {
			switch (str[i]) {
			case ' ':
			{
				const std::string pattern = str.substr(i, i-j);
				if (pattern.size()&&pattern[0] == '@') {
					if (pattern.size() == 2) {
						switch (pattern[1]) {
						case '/':
							break;
						case 'm':
							break;
						case '^':
							break;
						}
						break;
					}
					if (pattern.size() == 1) {
						if (st.size() == 1)return st.top();
						if (!st.size())return Expr(str, font->GetBox(str));
						Expr top = st.top(); st.pop();
						st.top().ch.push_back(top);
						break;
					}
				}
				break;
			}
			}
			i++;
		}
		return st.size()==1?st.top():Expr(str, font->GetBox(str));
	}

	
};

class Button3D {
public:
	struct ButtonStyle {
		Color text;
		Color normal;
		Color hovered;
		Color pressed;
		Color locked;
		bool lock = false;
		float padding = 0.1f;
		float textScale = -1.0f;
	};
	static ButtonStyle defaultButtonStyle;
	Button3D() {

	}
	Button3D(const std::string& text, const vec2& pos, const vec2& dim) {
		Init(text, pos, dim);
	}
	void Init(const std::string& text, const vec2& pos, const vec2& dim) {
		mText.GenerateTexture(text, vec2(1.0f));
		mPos = vec2(pos.x, -pos.y);
		mDim = dim;
	}
	bool locked()const {
		return mLocked;
	}
	void Unlock() {
		mLocked = false;
	}
	void Lock() {
		mLocked = true;
	}
	void SetStyle(const ButtonStyle* style) {
		mStyle = style;
	}
	bool Main(const Camera& camera, float& dist) {
		if (mLocked) {
			Render(mStyle->locked);
			return true;
		}
		AABB aabb; aabb.set(vec3(mPos.x, 0.0f, mPos.y), vec3(mDim.x, 1.0f, mDim.y));
		const float t = intersect(aabb, camera.cursor2world(window.mouse.pos, window.viewport));
		bool hovered = t>0.0f && t<dist;
		if (hovered)dist = t;
		bool clicked = window.mouse.left.clicked();
		if (clicked && hovered) {
			if (mStyle->lock) {
				Lock();
				Render(mStyle->locked);
				return true;
			}
			Render(mStyle->pressed);
			return true;
		}
		Render(hovered ? (window.mouse.left.pressed()?mStyle->pressed : mStyle->hovered) : mStyle->normal);
		return false;
	}
	void Render(const Color& fg) {
		phong.bind();
		const vec3 pos = vec3(mPos.x, mLocked ? -0.1f : 0.0f, mPos.y);
		Transformation trans;
		trans.scale(vec3(mDim.x, 1.0f, mDim.y));
		trans.translate(pos);
		phong.set("model", trans.toMatrix());
		Material mat(MaterialData(fg, fg, vec3(1.0f), 0.8f));
		mat.bind(phong, "material");
		CUBE332.get().Draw();

		trans.clear();
		trans.x = vec3(1.0f, 0.0f, 0.0f);
		trans.y = vec3(0.0f, 0.0f, -1.0f);
		trans.z = vec3(0.0f, 1.0f, 0.0f);

		vec2 target = mDim * (1.0f - mStyle->padding);
		float factor = mStyle->textScale<0.0f?std::min(target.x/mText.dim.x, target.y/mText.dim.y):mStyle->textScale;
		trans.scaleAxes(vec3(mText.dim.x*factor, mText.dim.y*factor, 1.0f));
		trans.w = pos;
		trans.w.y += 0.55f;
		mText.col = mStyle->text;
		mText.Render(trans);
	}
private:
	const ButtonStyle* mStyle = &defaultButtonStyle;
	Text3D mText;
	vec2 mPos = vec2(0.0f);
	vec2 mDim = vec2(0.0f);
	bool mLocked = false;
};

//#define ANIM "swift"
#define ANIM "wave"

class Tutor {
public:
	Tutor() {
		FumanEngine::Init("D:\\Libri\\C++\\Festa\\fuman");
		mFuman.Load("fuman/Worshipretty.fuman");
		mFuman.GetAnimation(ANIM)->Activate();
	}
	void Render(const Camera& camera, const vec2& pos) {
		Transformation trans; 
		trans.setRotation(Rotation(-90.0f, vec3(1.0f, 0.0f, 0.0f)));
		trans.setPosition(vec3(pos.x, 1.0f, -pos.y));

		if (window.mouse.left.clicked()) {
			mFuman.GetAnimation(ANIM)->Reset();
		}
		if (window.mouse.right.firstPressed()) {
			mFuman.GetAnimation("crouch")->Reset();
		}
		if (window.mouse.right.pressed()) {
			mFuman.GetAnimation("crouch")->Activate();
		}
		else {
			mFuman.GetAnimation("crouch")->Deactivate();
		}
		mFuman.UpdateAnimations(window.interval());
		mFuman.CalculateBones();
		OrientateToCursor(camera, mFuman.GetBoneTransformation(FumanEngine::HEAD), trans.getPosition());
		mFuman.ApplyBones();
		mFuman.Render(trans);

		Material mat(MaterialData(vec3(1.0f), vec3(1.0f), vec3(1.0f), 8.0f));
		Transformation t;
		t.scale(vec3(2.0f, 2.0f, 0.5f));
		t.setPosition(trans.getPosition()+vec3(0.0f, 0.0f, -mFuman.CalcAABB().min.y));
		phong.set("model", t.toMatrix());
		mat.bind(phong, "material");
		CUBE332.get().Draw();

	}
	void OrientateToCursor(const Camera& camera, Transformation& trans, const vec3& pos) {
		ivec2 cursorPos = window.mouse.pos;
		const vec3 cursor(cursorPos.x, window.height()-cursorPos.y, 0.8f);
		const vec3 world = camera.screen2world(cursor, window.viewport);
		trans.z = Rotation(90.0f, vec3(1.0f, 0.0f, 0.0f))*glm::normalize(world-pos);
		trans.x = glm::normalize(glm::cross(trans.z, vec3(0.0f, 1.0f, 0.0f)));
		trans.y = -glm::normalize(glm::cross(trans.z, trans.x));
	}
private:
	Fuman mFuman;
};

class Tutorial {
public:
	Tutorial() {}
	static Transformation GetTextTransform(const vec2& size, const vec2& pos) {
		Transformation trans;
		trans.x = vec3(1.0f, 0.0f, 0.0f);
		trans.y = vec3(0.0f, 0.0f, -1.0f);
		trans.z = vec3(0.0f, 1.0f, 0.0f);
		trans.scaleAxes(vec3(size.x, size.y, 1.0f));
		trans.w = vec3(pos.x, 0.05f, -pos.y);
		return trans;
	}
	void Config(const JsonData& cfg, const ivec2& imSize, int lineHeight) {
		mTrans = GetTextTransform(vec2(12.0f, 9.0f), vec2(0.0f));
		mImageSize = imSize;
		mLineHeight = lineHeight;
		mTitles.resize(cfg.size());
		mFiles.resize(cfg.size());
		uint idx = 0;
		for (const auto& i : cfg) {
			mTutorialMap[i.second[0].ToString()] = idx;
			mTitles[idx] = i.second[0].ToString();
			mFiles[idx++] = i.second[1].ToString();
		}
		mTitle.trans = GetTextTransform(vec2(1.0f), vec2(0.0f, 5.0f));
		//mTitle.col = Color(255, 255, 128);
		mPageNumberIndicator.trans = GetTextTransform(vec2(1.0f), vec2(0.0f, -5.0f));
		//mPageNumberIndicator.col = Color();
		InitButtons();
	}
	void Render(const Camera& camera) {
		if (mSelected < 0)return;
		mCurrentPage->Render(mTrans);
		mTitle.Render();
		mPageNumberIndicator.Render();
		float dist = INFINITY;
		if (mPrevButton.Main(camera, dist)) {
			PrevPage();
		}
		if (mNextButton.Main(camera, dist)) {
			NextPage();
		}
		
		bool set = false;
		for (int i = 0; i < mTutorialMenu.size(); i++) {
			if (i != mSelected)mTutorialMenu[i].Unlock();
			if (mTutorialMenu[i].Main(camera, dist)&&i!=mSelected&&!set) {
				Select(i); set = true;
			}
		}
		phong.bind();
		mTutor.Render(camera, vec2(8.0f, 0.0f));
	}
	void PrevPage() {
		if (!mPageNumber)return;
		mPageNumber--;
		mCurrentPage--;
		GeneratePageNumberIndicator();
	}
	void NextPage() {
		if (mPageNumber+1==mPages.size())return;
		mPageNumber++;
		mCurrentPage++;
		GeneratePageNumberIndicator();
	}
	void Select(int i) {
		mSelected = i;
		if (mSelected < 0 || mSelected >= mFiles.size()) {
			mSelected = -1;
			return;
		}
		File f(mFiles[mSelected], "r");
		if (f.check())return;
		std::string text; f.readLines(text);
		size_t pos = 0;
		mPages.clear();
		while (pos < text.size()) {
			mPages.push_back(TutorialPage());
			mPages.back().Parse(mImageSize, mLineHeight, text, pos);
		}
		mCurrentPage = mPages.begin();
		mTitle.GenerateTexture(" " + mTitles[mSelected] + " ", vec2(0.03f));
		mPageNumber = 0;
		GeneratePageNumberIndicator();
	}
	void Select(const std::string& title) {
		if (mTutorialMap.find(title) == mTutorialMap.end())return;
		Select(mTutorialMap[title]);
	}
	void GeneratePageNumberIndicator() {
		mPageNumberIndicator.GenerateTexture(" "+toString(mPageNumber+1) + "/" + toString(mPages.size())+" ", vec2(0.025f));
	}
private:
	typedef std::list<TutorialPage>::const_iterator iterator;
	Tutor mTutor;

	iterator mCurrentPage;
	Transformation mTrans;
	ivec2 mImageSize = ivec2(0);
	int mLineHeight = 0;
	int mSelected = -1;
	std::map<std::string, int> mTutorialMap;
	std::vector<std::string> mTitles;
	std::vector<Path> mFiles;
	std::list<TutorialPage> mPages;
	int mPageNumber = 0;

	Text3D mTitle;
	Text3D mPageNumberIndicator;

	Button3D::ButtonStyle mButtonStyles[2];
	Button3D mPrevButton;
	Button3D mNextButton;
	std::vector<Button3D> mTutorialMenu;

	void InitButtons() {
		mButtonStyles[0] = {
			Color(255, 255, 255),
			Color(0, 0, 255),
			Color(128, 128, 255),
			Color(0, 0, 128),
			Color(0, 0, 50),
			false
		};
		mButtonStyles[1] = {
			Color(0, 0, 0),
			Color(255, 255, 255),
			Color(128, 128, 128),
			Color(50, 50, 50),
			Color(50, 50, 50),
			true,
			0.0f,
			0.008f
		};
		mPrevButton.Init("<", vec2(-3.0f, -5.0f), vec2(1.0f, 1.0f));
		mPrevButton.SetStyle(&mButtonStyles[0]);
		mNextButton.Init(">", vec2(3.0f, -5.0f), vec2(1.0f, 1.0f));
		mNextButton.SetStyle(&mButtonStyles[0]);

		const vec2 dim = vec2(5.0f, 1.0f);
		vec2 pos = vec2(-7.0f - dim.x * 0.5f, 4.0f + dim.y * 0.5f);
		mTutorialMenu.resize(mTitles.size());
		for (int i = 0; i < mTitles.size(); i++) {
			mTutorialMenu[i].Init(" " + mTitles[i] + " ", pos, dim);
			mTutorialMenu[i].SetStyle(&mButtonStyles[1]);
			pos.y -= dim.y;
		}
	}
};
