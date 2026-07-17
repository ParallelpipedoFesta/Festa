#pragma once
#include "../common/window.h"
#include<stack>
#include <commctrl.h>
#pragma comment(lib,"COMCTL32.lib")





namespace Festa {
	class Menu :public CommandComponent {
	public:
		HMENU hmenu;
		Menu() {
			hmenu = CreatePopupMenu();
		}
		template<typename Fn, typename ...Args>
		void add(const String& name, Fn&& f, Args&&... args) {
			AppendMenu(hmenu, MF_STRING, addCommand(f, args...), name.towstring().c_str());
		}
		~Menu() {
			DestroyMenu(hmenu);
		}
	};

	class MenuBar :public Menu {
	public:
		MenuBar() { hmenu = CreateMenu(); }
		void addMenu(const String& name, const Menu& menu) {
			AppendMenu(hmenu, MF_POPUP, (UINT_PTR)menu.hmenu, name.towstring().c_str());
		}

		void bind(HWND hwnd) {
			SetMenu(hwnd, hmenu);
		}
	};

	class IndMenu :public Menu {
	public:
		struct MenuConfig {
			ull id;
			const wchar_t* name;
			MenuConfig(ull id, const String& name) :id(id), name(name.towstring().c_str()) {}
		};
		IndMenu() {}
		template<typename Fn, typename ...Args>
		void add(const String& name, Fn&& func, Args&&... args) {
			config.push_back(MenuConfig(addCommand(func, args...), name));
		}
		void show(HWND hwnd, int x, int y) {
			hmenu = CreatePopupMenu();
			for (MenuConfig c : config)AppendMenu(hmenu, MF_STRING, c.id, c.name);
			TrackPopupMenu(hmenu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hwnd, NULL);
			DestroyMenu(hmenu);
		}
	private:
		std::vector<MenuConfig> config;
	};

	class Notebook {
	public:
		HWND hwnd;
		Notebook() {
			hwnd = 0;
		}
		Notebook(HWND hWnd, int w, int h, int x = 0, int y = 0) {
			hwnd = CreateWindowW(WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE,
				x, y, w, h, hWnd, NULL, GetModuleHandleW(0), NULL);
		}
		void add(const String& name) {
			TCITEM tie;
			tie.mask = TCIF_TEXT;
			wchar_t* tmp = new wchar_t[name.size()];
			lstrcpyW(tmp, name.towstring().c_str());
			tie.pszText = tmp;
			TabCtrl_InsertItem(hwnd, 0, &tie);
		}
	};


	struct ImGuiComponent {
		std::function<void()> func;
		ImGuiComponent() {}
		template <typename Func, typename... Args>
		ImGuiComponent(Func&& f, Args&&... args) {
			func = [=]() { f(args...); };
		}
		void render()const {
			if (func)func();
		}
	};

	class BaseImGuiWindow {
	public:
		std::string title;
		GLWindow* father = 0;
		std::vector<ImGuiComponent> components;
		ImGuiWindowFlags flags = 0;
		virtual void Begin() {

		}
		void End() {
			if (!status[0])return;
			status[0] = 0;
			ImGui::End();
		}
		virtual void Render() {
			Begin(); End();
		}
		VirtualValue<bool> X() {
			return status[2];
		}
		void Add(const ImGuiComponent& component) {
			components.push_back(component);
		}
		bool IsActivated() {
			return !status[1];
		}
		void Activate() {
			status[1] = 0;
		}
		void Deactivate() {
			status[1] = 1;
		}
	protected:
		BoolList status = BoolList(3);//0:dur/end,1:ter/run
		bool _begin() {
			if (status[0] || status[1])return true;
			status[0] = 1;
			bool activated = true;
			ImGui::Begin(title.c_str(), X() ? &activated : 0, flags);
			if (!activated) {
				Deactivate();
				End();
				return true;
			}
			father->imgui->focused |= ImGui::IsWindowFocused();
			return false;
		}
	};

	class IMGUIWindow:public BaseImGuiWindow {
	public:
		IMGUIWindow() {}
		IMGUIWindow(Window* _father, const std::string& _title,
			const ivec2& _size = ivec2(-1), const ivec2& _pos = ivec2(-1),bool x=true, ImGuiWindowFlags _flags = 0) {
			Init(_father, _title, _size, _pos,x, _flags);
		}
		void Init(Window* _father, const std::string& _title,
			const ivec2& _size = ivec2(-1), const ivec2& _pos = ivec2(-1),
			bool x=true,ImGuiWindowFlags _flags = 0) {
			father = _father; title = _title; size = _size; pos = _pos; flags = _flags;
			X() = x;
		}
		void Begin() {
			if (_begin())return;
			if (pos.x != -1)ImGui::SetWindowPos(ImVec2(float(pos.x), float(pos.y)));
			if (size.x != -1)ImGui::SetWindowSize(ImVec2(float(size.x), float(size.y)));
			for (uint i = 0; i < components.size(); i++)
				components[i].render();
		}
		void Resize(const ivec2& size) {
			bool activated = true;
			ImGui::Begin(title.c_str(), &activated, flags);
			ImGui::SetWindowSize(ImVec2(float(size.x), float(size.y)));
			ImGui::End();
		}
		void SetPosition(const ivec2& pos) {
			bool activated = true;
			ImGui::Begin(title.c_str(), &activated, flags);
			ImGui::SetWindowPos(ImVec2(float(pos.x), float(pos.y)));
			ImGui::End();
		}
	private:
		ivec2 size = ivec2(-1), pos = ivec2(-1);
	};

	class DockingWindow:public BaseImGuiWindow {
	public:
		DockingMode mode = DOCKING_NONE;
		int size = -1;
		float dm=0.0f;
		ImVec2 lp = ImVec2(-1.0f, -1.0f), ls = ImVec2(-1.0f, -1.0f);
		DockingWindow() {}
		DockingWindow(GLWindow* _father, const std::string& _title,
			DockingMode _mode, int _size = -1,bool x=false,
			ImGuiWindowFlags _flags = ImGuiWindowFlags_NoCollapse) {
			Init(_father, _title, _mode, _size,x, _flags);
		}
		void Init(GLWindow* _father, const std::string& _title,
			DockingMode _mode, int _size = -1,bool x=false, 
			ImGuiWindowFlags _flags = ImGuiWindowFlags_NoCollapse) {
			father = _father; title = _title; mode = _mode; size = _size, flags = _flags; X() = x;
			if (mode < 0 || mode>4)LOGGER.error("DockedWindow::Invaild mode: " + toString(mode));
		}
		void Begin() {
			if (_begin()|| ImGui::IsWindowCollapsed())return;
			const float ww = float(father->viewport.w), wh = float(father->viewport.h),
				wx = float(father->viewport.x), wy = float(father->viewport.y);
			ImVec2 pos = ImGui::GetWindowPos(), s = ImGui::GetWindowSize();
			float xoff = size == -1 ? s.x : float(size), yoff = size == -1 ? s.y : float(size);
			float x = wx, y = wy, w = ww, h = wh;
			bool motion = false;
			if (lp.x == -1 || (pos == lp && s == ls)) {
				s = ImVec2(ww, wh);
				switch (mode) {
				case DOCKING_LEFT:
					pos.x = wx, pos.y = wy, s.x = xoff, x += xoff, w -= xoff;
					break;
				case DOCKING_RIGHT:
					pos.x = wx + ww - xoff, pos.y = wy, s.x = xoff, w -= xoff;
					break;
				case DOCKING_TOP:
					pos.x = wx, pos.y = wy, s.y = yoff, y += yoff, h -= yoff;
					break;
				case DOCKING_BOTTOM:
					pos.x = wx, pos.y = wy + wh - yoff, s.y = yoff, h -= yoff;
					break;
				case DOCKING_FULLSCREEN:
					pos.x = wx, pos.y = wy;
					w = h = 0;
					break;
				}
			}
			else motion = true;
			switch (mode) {
			case DOCKING_LEFT:
				s.x += dm, x += dm, w -= dm;
				break;
			case DOCKING_RIGHT:
				pos.x += dm, s.x -= dm, w += dm;
				break;
			case DOCKING_TOP:
				s.y += dm, y += dm, h -= dm;
				break;
			case DOCKING_BOTTOM:
				pos.y += dm, s.y -= dm, h += dm;
				break;
			}
			if (motion)updateWindows(pos, s, father->viewport);
			if (mode != DOCKING_FULLSCREEN)
				father->imgui->dockedWindows[mode] = this;

			normalize(pos.x, pos.y); normalize(s.x, s.y);
			normalize(x, y); normalize(w, h);
			ImGui::SetWindowPos(pos);
			ImGui::SetWindowSize(s);
			lp = pos; ls = s;
			dm = 0.0f;
			//unfolded window
			if (!ImGui::IsWindowCollapsed())
				father->setViewport(Viewport(x, y, w, h));
			for (uint i = 0; i < components.size(); i++)
				components[i].render();
		}
		template<typename T>
		void normalize(T& x, T& y) {
			x = std::max(std::min(x, T(father->width() - 1)), T(0));
			y = std::max(std::min(y, T(father->height() - 1)), T(0));
		}
		void Resize(const ivec2& s) {
			ImGui::Begin(title.c_str());
			ImGui::SetWindowSize(ImVec2(float(s.x), float(s.y)));
			ImGui::End();
		}
		void SetPosition(const ivec2& pos) {
			bool activated = true;
			ImGui::Begin(title.c_str(), &activated);
			ImGui::SetWindowPos(ImVec2(float(pos.x), float(pos.y)));
			ImGui::End();
		}
	private:
		void updateWindows(int m, const ImVec2& p, const ImVec2& s, const Viewport& v) {
			const ImVec2 rp = ImGui::GetWindowPos();

			switch (m) {
			case DOCKING_LEFT:
				if (father->imgui->dockedWindows[DOCKING_LEFT]) {
					DockingWindow& w = *(DockingWindow*)father->imgui->dockedWindows[DOCKING_LEFT];
					if (lp != p && s != p && w.dm == 0)w.dm = p.x - lp.x;
				}
				break;
			case DOCKING_RIGHT:
				if (father->imgui->dockedWindows[DOCKING_RIGHT]) {
					DockingWindow& w = *(DockingWindow*)father->imgui->dockedWindows[DOCKING_RIGHT];
					if (lp == p && s != p && w.dm == 0)w.dm = p.x + s.x - lp.x - ls.x;
				}
				break;
			case DOCKING_TOP:
				if (father->imgui->dockedWindows[DOCKING_TOP]) {
					DockingWindow& w = *(DockingWindow*)father->imgui->dockedWindows[DOCKING_TOP];
					if (lp != p && s != p && w.dm == 0)w.dm = p.y - lp.y;
				}
				break;
			case DOCKING_BOTTOM:
				if (father->imgui->dockedWindows[DOCKING_BOTTOM]) {
					DockingWindow& w = *(DockingWindow*)father->imgui->dockedWindows[DOCKING_BOTTOM];
					if (lp == p && s != p && w.dm == 0)w.dm = p.y + s.y - lp.y - ls.x;
				}
				break;
			}
		}
		void updateWindows(const ImVec2& p, const ImVec2& s, const Viewport& v) {
			switch (mode) {
			case DOCKING_LEFT:
				updateWindows(DOCKING_LEFT, p, s, v);
				updateWindows(DOCKING_TOP, p, s, v);
				updateWindows(DOCKING_BOTTOM, p, s, v);
				break;
			case DOCKING_RIGHT:
				updateWindows(DOCKING_RIGHT, p, s, v);
				updateWindows(DOCKING_TOP, p, s, v);
				updateWindows(DOCKING_BOTTOM, p, s, v);
				break;
			case DOCKING_TOP:
				updateWindows(DOCKING_LEFT, p, s, v);
				updateWindows(DOCKING_RIGHT, p, s, v);
				updateWindows(DOCKING_TOP, p, s, v);
				break;
			case DOCKING_BOTTOM:
				updateWindows(DOCKING_LEFT, p, s, v);
				updateWindows(DOCKING_RIGHT, p, s, v);
				updateWindows(DOCKING_BOTTOM, p, s, v);
				break;
			case DOCKING_FULLSCREEN:
				updateWindows(DOCKING_LEFT, p, s, v);
				updateWindows(DOCKING_RIGHT, p, s, v);
				updateWindows(DOCKING_TOP, p, s, v);
				updateWindows(DOCKING_BOTTOM, p, s, v);
				break;
			}
		}
	};

	inline ImGuiComponent ImGuiSameLine() {
		return ImGuiComponent([]() {ImGui::SameLine(); });
	}

	inline ImGuiComponent ImGuiButton(const std::string& name, Command_t&& command) {
		return ImGuiComponent([name, command]() {
			if (ImGui::Button(name.c_str()))command();
			});
	}
	inline ImGuiComponent ImGuiText(const std::string& text) {
		return ImGuiComponent([text]() {ImGui::Text(text.c_str()); });
	}
	inline ImGuiComponent ImGuiBulletText(const std::string& text) {
		return ImGuiComponent([text]() {ImGui::BulletText(text.c_str()); });
	}



	struct TreeNode {
		std::string name;
		std::vector<TreeNode> children;
		ImGuiComponent component;
		TreeNode() {}
		TreeNode(const std::string& name, bool leaf = true)
			:name(name) {
			pos = leaf ? 0 : 2;
		}
		void render()const {
			if (isLeaf()) {
				component.render();
				return;
			}
			if (pos == 1)component.render();
			if (ImGui::TreeNode(name.c_str())) {
				if (pos == 2)component.render();
				for (uint i = 0; i < children.size(); i++) {
					if (children[i].pos != -1)children[i].render();
				}
				if (pos == 3)component.render();
				ImGui::TreePop();
			}
		}
		void add(TreeNode child) {
			pos = 2;
			children.push_back(child);
		}
		void add(const std::string& treenode) {
			pos = 2;
			children.push_back(TreeNode(treenode));
		}
		bool isLeaf()const {
			return pos == 0;
		}
		void setBrachPos(int position) {
			pos = position;
		}
		void release() {
			if (pos == -1)return;
			name.clear();
			pos = -1;
			children.clear();
		}
		void setPos(int _pos) {
			pos = _pos;
		}
	private:
		int pos = 0;
	};


	inline std::string ImGuiLeftLabel(const std::string& label) {
		ImGui::Text(label.c_str());
		ImGui::SameLine();
		return "##" + label;
	}


	template<uint bufsize=128>
	class ImGuiInputBox {
	public:
		char buf[bufsize] = {0};
		std::string label;
		ImGuiInputBox() {}
		ImGuiInputBox(const std::string& _label) {
			SetLabel(_label);
		}
		void SetLabel(const std::string& _label) {
			label = _label;
			Clear();
		}
		void SetContent(const std::string& str) {
			strcpy(buf, str.c_str());
		}
		~ImGuiInputBox() {
		}
		void Clear() {
			buf[0] = 0;
		}
		void Render(bool showLabel=true) {
			if(showLabel)ImGui::InputText(ImGuiLeftLabel(label).c_str(), buf, bufsize);
			else ImGui::InputText(("##"+label).c_str(), buf, bufsize);
		}
		VirtualValue<std::string> str() {
			return VirtualValue<std::string>(
				[=](const std::string& t) {strcpy(buf, t.c_str()); },
				[=] {return std::string(buf); }
			);
		}
		
		std::string GetContent()const {
			return buf;
		}
		operator std::string()const {
			return GetContent();
		}
	};

	class ImGuiTabs {
	public:
		struct Tab {
			std::string label;
			std::function<void()> f;
		};
		std::string label;
		std::list<Tab> tabs;
		ImGuiTabs() {}
		ImGuiTabs(const std::string& label) :label(label) {}
		void render() {
			ImGui::BeginTabBar(label.c_str());
			for (std::list<Tab>::iterator it = tabs.begin(); it != tabs.end(); it++) {
				bool x = true;
				if (ImGui::BeginTabItem((*it).label.c_str(), &x)) {
					(*it).f();
					ImGui::EndTabItem();
				}
				if (!x) {
					tabs.erase(it);
					ImGui::EndTabBar();
					return;
				}
			}
			ImGui::EndTabBar();
		}
		void add(const Tab& tab) {
			tabs.push_back(tab);
			// ImGui::InputText();
		}
	};

}