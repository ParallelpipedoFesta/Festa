#pragma once
#define _CRT_SECURE_NO_WARNINGS

#define USING_IMGUI

#include "frame.h"
#include "shapes.h"
#include "common.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "../3rd/GL/glfw3native.h"
#include<MMSystem.h>
#pragma comment(lib, "Winmm.lib")
//#pragma comment(lib,"MMlib.lib")

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../3rd/imgui/imgui.h"
#include "../3rd/imgui/imgui_impl_glfw.h"
#include "../3rd/imgui/imgui_impl_opengl3.h"
#include "../3rd/imgui/imgui_internal.h"

#include <mutex>
#include <thread>
#include<shlobj.h>

#define KEY_DOWN(x) ((GetAsyncKeyState(x) & 0x8000) ? 1:0)
#define KEY_UP(x) ((GetAsyncKeyState(x) & 0x8000) ? 0:1)
typedef std::function<void()> Command_t;
typedef Command_t TaskFunc;


#ifdef HIDE_CONSOLE
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif

namespace Festa {

	class TaskTimer {
	public:
		TaskFunc task;
		double interval;
		TaskTimer() { task = 0; timer.reset(); interval = 0.0f; }
		template<typename Fn, typename ...Args>
		TaskTimer(double interval, Fn&& fn, Args&&... args) :interval(interval) {
			reset();
			task = [=] {fn(args...); };
		}
		void reset() {
			timer.reset();
		}
		void check() {
			if (timer.interval() >= interval)task();
		}
		void repeat() {
			if (timer.interval() >= interval) {
				task();
				reset();
			}
		}
	private:
		Timer timer;
	};

	typedef bool (*taskfunc_t)(double);
	struct Task {
		double st=0.0, dur=0.0;
		taskfunc_t f=0;
		double _clk=0.0;
		Task() {}
		Task(taskfunc_t _f, double _st, double _dur) :f(_f), st(_st), dur(_dur), _clk(getTime()) {
			
		}
		int check() {
			double t = getTime() - _clk;
			if (t < st)return 0;
			if (dur < 0.0 || t<=st+dur)
				return f(t - st) ? -1 : 1;
			return -1;
		}
	};

	class ThreadTimer
	{
	public:
		std::function<void()> task = 0;
		double delay = 0.0;
		ThreadTimer() {}
		template<typename Fn, typename ...Args>
		ThreadTimer(double _delay, Fn&& fn, Args&&... args)
			:delay(_delay) {
			task = [=] {fn(args...); };
			expired = true; tryToExpire = false;
		}
		ThreadTimer(const ThreadTimer& timer) {
			expired = timer.expired.load();
			tryToExpire = timer.tryToExpire.load();
			task = timer.task; delay=timer.delay;
			expired = true; tryToExpire = false;
		}
		~ThreadTimer() {
			//stop(); 
		}
		void operator=(const ThreadTimer& timer) {
			expired = timer.expired.load();
			tryToExpire = timer.tryToExpire.load();
			task = timer.task; delay=timer.delay;
		}
		void start() {
			if (!expired)return;
			expired = false;
			int d = int(delay * 1000.0);
			std::thread([this, d]() {
				while (!tryToExpire) {
					std::this_thread::sleep_for(std::chrono::milliseconds(d));
					task();
				}
				std::lock_guard<std::mutex> locker(mtx);
				expired = true; tryToExpire = false;
				expiredCond.notify_one();
				}).detach();
		}

		void startOnce()
		{
			int d = int(delay * 1000.0);
			std::thread([this, d]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(d));
				task();
				}).detach();
		}

		void stop() {
			if (expired || tryToExpire)return;
			tryToExpire = true;
			std::unique_lock<std::mutex> locker(mtx);
			expiredCond.wait(locker, [this] {return expired == true; });
		}
	private:
		std::atomic<bool> expired;
		std::atomic<bool> tryToExpire;
		std::mutex mtx;
		std::condition_variable expiredCond;
	};


	class ButtonStatus {
	public:
		ButtonStatus() {}
		ButtonStatus(bool p) {
			update(p);
		}
		void operator=(const ButtonStatus& b) {
			status = b.status;
		}
		template<typename T>
		void operator=(const T& b) = delete;
		void update(bool p) {
			status >>= 1;
			status |= uchar(p) << 1;
		}
		void clear() {
			status = 0;
		}
		void operator<<(bool p) {
			update(p);
		}
		bool previous()const {
			return status & 1;
		}
		bool pressed()const {
			return status & 2;
		}
		bool down()const {
			return pressed();
		}
		bool up()const {
			return !pressed();
		}
		bool released()const {
			return (!pressed()) && previous();
		}
		bool clicked()const {
			return released();
		}
		bool firstPressed()const {
			return pressed() && (!previous());
		}
		operator bool()const {
			return pressed();
		}
	private:
		uchar status = 0;
	};



	struct Event {
		MSG msg;
		Event() { msg = {}; }
		int get(HWND hwnd = 0) {// 1 if running;0 if quit;-1 if error
			return PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE);
		}
		void update() {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		uint message()const {
			return msg.message;
		}
		bool checkEvent(uint ev)const {
			return message() == ev;
		}
		ull wlow()const {
			return LOWORD(msg.wParam);
		}
		ull whigh()const {
			return HIWORD(msg.wParam);
		}
		ull w()const {
			return msg.wParam;
		}
		ull llow()const {
			return LOWORD(msg.lParam);
		}
		ull lhigh()const {
			return HIWORD(msg.lParam);
		}
		ull l()const {
			return msg.lParam;
		}
	};

	class CommandComponent {
	public:
		static void update(const std::vector<Command_t>& commands,ull id) {
			if (1 <= id && id <= commands.size())commands[id - 1]();
		}
		template<typename Fn, typename ...Args>
		uint addCommand(std::vector<Command_t>& commands,Fn&& f, Args&&... args) {
			commands.push_back([=] {f(args...); });
			return uint(commands.size());
		}
	};


	Path askOpenFileName(HWND hwnd, const wchar_t* filter, const wchar_t* title = 0, const wchar_t* initDir=0);
	Path askSaveFileName(HWND hwnd, const wchar_t* filter, const wchar_t* title = 0, const wchar_t* initDir = 0);
	Path selectDirectory();
	
	enum DockingMode {
		DOCKING_NONE=-1, DOCKING_LEFT, DOCKING_RIGHT, DOCKING_TOP, DOCKING_BOTTOM, DOCKING_FULLSCREEN
	};

	class MFCWindow {
	public:
		typedef std::function<int(const Event&)> EventCallback;

		struct Mouse {
			int scroll = 0;
			ivec2 pos = ivec2(0), prevPos = ivec2(0);
			ButtonStatus left, right;
			MFCWindow* window = 0;
			bool inWindow()const {
				ivec2 size = window->size();
				return 0 <= pos.x && pos.x < size.x &&
					0 <= pos.y && pos.y < size.y;
			}
			bool moved()const {
				if (prevPos.x == -1)return false;
				return prevPos.x != pos.x || prevPos.y !=pos.y;
			}
			void updatePos(const ivec2& _pos) {
				prevPos = pos;
				pos = _pos;
			}
			ivec2 dir()const {
				return pos - prevPos;
			}
			void moveTo(int x, int y) {
				ivec2 _pos = window->position() + ivec2(x, y);
				SetCursorPos(_pos.x,_pos.y);
				updatePos(ivec2(x, y));
			}
			void moveTo(const ivec2& pos) {
				ivec2 _pos = window->position() + pos;
				SetCursorPos(_pos.x, _pos.y);
				updatePos(pos);
			}
			void showCursor(int show) {
				ShowCursor(show);
			}
		};
		HINSTANCE hinstance = 0;
		HWND hwnd = 0;
		Mouse mouse;
		MFCWindow() {}
		MFCWindow(const std::wstring& classname, const std::wstring& title, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT) {
			Init(classname,title,x,y,w,h);
		}
		MFCWindow(HWND _hwnd) {
			Init(_hwnd);
		}
		~MFCWindow() {
			release();
		}
		void release() {
			
		}
		void Init(const std::wstring& classname, const std::wstring& title, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT);
		void Init(HWND _hwnd) {
			hwnd = _hwnd;
			hinstance = GetModuleHandle(0);
			mouse.window = this;
		}
		virtual void proc(MSG& msg) {

		}

		virtual int pollEvents() {
			mInterval = timer.interval();
			if (!IsWindow(hwnd))return 0;
			int state;
			mouse.scroll = 0;
			bool l = false, r = false;
			while (state = ev.get(hwnd)) {
				if (state == -1)return -1;
				if (eventCallback) {
					int code = eventCallback(ev);
					if(code!=1)return code;
				}
				switch (ev.message()) {
				case WM_COMMAND:
					CommandComponent::update(commands,ev.wlow());
					break;
				case WM_MOUSEWHEEL:
					mouse.scroll += sgn(GET_WHEEL_DELTA_WPARAM(ev.w()));
					break;
				case WM_LBUTTONDOWN:
					l = true;
					break;
				case WM_RBUTTONDOWN:
					r = true;
					break;
				}
				ev.update();
			}
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			mouse.updatePos(ivec2(cursorPos.x, cursorPos.y) - position());
			mouse.left.update(l);
			mouse.right.update(r);
			//glfwGetMouseButton(ptr, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS
			return 1;
		}

		virtual int update() {
			return pollEvents();
		}

		static int Messagebox(HWND hwnd, const String& text, const String& title, uint buttonFlag = MB_OKCANCEL, uint iconFlag = MB_ICONINFORMATION) {
			return MessageBoxW(hwnd, text.towstring().c_str(), title.towstring().c_str(), buttonFlag | iconFlag);
		}

		static void SetWindowStyle(HWND hwnd, int style) {
			int _style = GetWindowLong(hwnd, GWL_STYLE);
			_style &= ~style;
			SetWindowLong(hwnd, GWL_STYLE, _style);
		}
		void Pin(int x, int y){
			SetWindowPos(hwnd, HWND_TOPMOST, x, y, width(), height(), SWP_NOMOVE | SWP_NOSIZE);
		}
		void Unpin(int x, int y) {
			SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, 0, 0, 0);
		}
		void Resize(int w, int h) {
			ivec2 pos = position();
			SetWindowPos(hwnd, HWND_TOPMOST, pos.x, pos.y, w, h, 0);
		}
		ivec2 position()const {
			RECT rect;
			GetWindowRect(hwnd, &rect);
			return ivec2(rect.left, rect.top);
		}
		virtual ivec2 size()const {
			RECT rect;
			GetWindowRect(hwnd, &rect);
			return ivec2(rect.right - rect.left, rect.bottom - rect.top);
		}
		int width()const {
			return size().x;
		}
		int height()const {
			return size().y;
		}

		int messagebox(const String& text, const String& title, uint buttonFlag = MB_OKCANCEL, uint iconFlag = MB_ICONINFORMATION)const {
			return Messagebox(hwnd, text, title, buttonFlag, iconFlag);
		}
		//WS_...
		void setStyle(int style) {
			SetWindowStyle(hwnd, style);
		}
		//SW_...
		void showMode(int mode) {
			ShowWindow(hwnd, mode);
		}
		void titled() {
			setStyle(WS_TILEDWINDOW);
		}
		void sizeBox() {
			setStyle(WS_SIZEBOX);
		}
		void maximize() {
			showMode(SW_MAXIMIZE);
		}
		void minimize() {
			showMode(SW_MINIMIZE);
		}
		void show() {
			showMode(SW_SHOW);
		}
		void hide() {
			showMode(SW_HIDE);
		}
		void showNormal() {
			showMode(SW_SHOWNORMAL);
		}
		void showDefault() {
			showMode(SW_SHOWDEFAULT);
		}
		void setEventCallback(EventCallback callback) {
			eventCallback = callback;
		}
		double interval()const {
			return mInterval;
		}
		double fps()const {
			return 1.0/mInterval;
		}
		static void showCursor() {
			while (ShowCursor(TRUE) < 0)
				ShowCursor(TRUE);
		}
		static void hideCursor() {
			while (ShowCursor(FALSE) >= 0)
				ShowCursor(FALSE);
		}
	protected:
		double mInterval = 0.0;
		FrameTimer timer;
		Event ev;
		EventCallback eventCallback = 0;
		std::vector<Command_t> commands;
	};

	class GLWindow:public MFCWindow {
	public:
		struct IMGUI {
			bool first = true;
			bool focused = false;
			GLWindow* window=0;
			void* dockedWindows[4] = { 0,0,0,0 };
			IMGUI() {}
			IMGUI(GLWindow* window):window(window) {
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
				ImGui_ImplGlfw_InitForOpenGL(window->ptr, true);
				std::string f = window->glVersion.glFormat();
				ImGui_ImplOpenGL3_Init(f.c_str());
			}
			void render() {
				focused = false;
				dockedWindows[0] = dockedWindows[1] = 
					dockedWindows[2] = dockedWindows[3] = 0;
				if (!first)ImGui::Render();
				first = false;
				ImDrawData* data = ImGui::GetDrawData();
				if (data)ImGui_ImplOpenGL3_RenderDrawData(data);
			}
			void updateIsFocused() {
				focused |= ImGui::IsWindowFocused();
			}
			void begin() {
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
			}
		};

		GLFWwindow* ptr = 0;
		IMGUI* imgui = 0;
		Viewport viewport;
		Version2 glVersion;
		GLWindow() {}
		GLWindow(int w, int h,const std::string& title,const Version2& glVersion = 4.4, std::function<void()> initf=0) {
			Init(w, h, title, glVersion, initf);
		}
		void Init(int w, int h,const std::string& title, const Version2& glVersion = 4.4, std::function<void()> initf=0);
		GLWindow(GLFWwindow* ptr) :ptr(ptr) {
			mouse.window = this;
			hwnd = glfwGetWin32Window(ptr);
			hinstance = GetModuleHandle(0);
			viewport = Viewport(width(), height());
			viewport.apply();
		}
		void Release();
		~GLWindow() {
			Release();
		}
		ivec2 size()const {
			ivec2 ret;
			glfwGetWindowSize(ptr, &ret.x, &ret.y);
			return ret;
		}
		int width()const {
			return size().x;
		}
		int height()const {
			return size().y;
		}
		bool isFocused()const {
			if (!imgui)return true;
			return !(imgui->focused);
		}
#ifdef USING_IMGUI
		void EnableImGui() {
			if(!imgui)imgui = new IMGUI(this);
		}
		bool ImGuiEnabled()const { 
			return imgui; 
		}
#endif
		void setViewport(const Viewport& v) {
			viewport = v;
			viewport.apply();
		}
		void updateViewport() {
			viewport.x = viewport.y = 0;
			glfwGetWindowSize(ptr, &viewport.w, &viewport.h);
			viewport.apply();
		}
		int pollEvents() {
			mInterval = timer.interval();
			if (glfwWindowShouldClose(ptr))return 0;
			int state;
			mouse.scroll = 0;
			while (state = ev.get(hwnd)) {
				if (state == -1)return -1;
				if (eventCallback) {
					int code = eventCallback(ev);
					if (code == 0)break;
					if (code != 1)return code;
				}
				switch (ev.message()) {
				case WM_COMMAND:
					//CommandComponent::update(ev.wlow());
					break;
				case WM_MOUSEWHEEL:
					mouse.scroll += sgn(GET_WHEEL_DELTA_WPARAM(ev.w()));
					break;
				}
				ev.update();
			}
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			mouse.updatePos(ivec2(cursorPos.x, cursorPos.y)-position());
			mouse.left.update(glfwGetMouseButton(ptr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
			mouse.right.update(glfwGetMouseButton(ptr, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
			updateViewport();
			return 1;
		}
		int update() {
#ifdef USING_IMGUI
			if (imgui)imgui->render();
#endif
			glfwSwapBuffers(ptr);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#ifdef USING_IMGUI
			if (imgui)imgui->begin();
#endif
			return pollEvents();
		}
		void render() {
#ifdef USING_IMGUI
			if (imgui)imgui->render();
#endif
			glfwSwapBuffers(ptr);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#ifdef USING_IMGUI
			if (imgui)imgui->begin();
#endif
		}
		void SetIcon(const Image& icon) {
			Image image=icon;
			image.bgr2rgb();
			image.resetChannels(4);
			GLFWimage img[1];
			img[0].pixels = image.data();
			img[0].height = image.height();
			img[0].width = image.width();
			glfwSetWindowIcon(ptr, 1, img);
		}
		bool shouldClose()const {
			return glfwWindowShouldClose(ptr);
		}
		void SetShouldClose(bool val)const {
			glfwSetWindowShouldClose(ptr, val);
		}
		int GetKey(int key)const {
			return glfwGetKey(ptr, key);
		}
	private:
		static bool first;
	};
	typedef GLWindow Window;

	inline ImFont* ImGuiFreetypeFont(const std::string& filename, float pixelSize) {
		ImGuiIO& io = ImGui::GetIO();
		ImFont* font = io.Fonts->AddFontFromFileTTF(
			filename.c_str(),
			pixelSize,
			nullptr,
			io.Fonts->GetGlyphRangesChineseFull()
		);
		IM_ASSERT(font);
		return font;
	}

	inline void setEnglishLayout() {
		LoadKeyboardLayout(L"0x0409", KLF_ACTIVATE);
	}

	class Canvas {
	public:
		HWND hwnd = 0;
		HDC hdc = 0;
		HPEN pen = 0; 
		HBRUSH brush = 0;
		Canvas() {}
		Canvas(HWND _hwnd) {
			init(_hwnd);
		}
		Canvas(const MFCWindow& window) {
			init(window.hwnd);
		}
		void init(HWND _hwnd = 0) {
			hwnd = _hwnd;
			hdc = GetDC(hwnd);
		}
		~Canvas() {
			release();
		}
		void release() {
			if (pen)DeleteObject(pen);
			if (brush)DeleteObject(brush);
			ReleaseDC(hwnd,hdc);
		}
		ivec2 moveTo(int x, int y)const {
			POINT pos;
			MoveToEx(hdc, x, y, &pos);
			return ivec2(pos.x, pos.y);
		}
		void lineTo(int x, int y)const {
			LineTo(hdc, x, y);
		}
		void createPen(const Color& color, int width, int style = PS_SOLID) {
			if (pen)DeleteObject(pen);
			pen = CreatePen(style, width, color.MFC());
			SelectObject(hdc, pen);
		}
		void solidBrush(const Color& color) {
			if (brush)DeleteObject(brush);
			brush = CreateSolidBrush(color.MFC());
			SelectObject(hdc, brush);
		}
		void setPixel(int x, int y, const COLORREF& color)const {
			SetPixel(hdc, x, y, color);
		}
		Color getPixel(int x, int y)const {
			return GetPixel(hdc, x, y);
		}
		void setTextColor(const Color& color)const {
			SetTextColor(hdc, color.MFC());
		}
		void text(const String& str, int x, int y)const {
			TextOutW(hdc, x, y, str.towstring().c_str(), int(str.size()));
		}
		void text(const String& str, int x, int y, const Color& color)const {
			setTextColor(color);
			text(str, x, y);
		}
		void line(int x1, int y1, int x2, int y2)const {
			MoveToEx(hdc, x1, y1, 0);
			LineTo(hdc, x2, y2);
		}
		void rectangle(int x1, int y1, int x2, int y2)const {
			Rectangle(hdc, x1, y1, x2, y2);
		}
		void circle(int x, int y, int r)const {
			Ellipse(hdc, x-r, y-r, x+r, y+r);
		}

		void blitImage(const Image& img) {
			HDC hdc = GetDC(hwnd);
			HINSTANCE hInstanc = GetModuleHandle(NULL);

			HANDLE bk = LoadImageW(hInstanc, L"bk.bmp", IMAGE_BITMAP, 1400, 770, LR_LOADFROMFILE);

			HBITMAP hBit1 = CreateCompatibleBitmap(hdc, 1500, 770);
			HDC hBk = CreateCompatibleDC(NULL);
			HDC hDst = CreateCompatibleDC(NULL);
			SelectObject(hDst, hBit1);
			SelectObject(hBk, bk);
			BitBlt(hDst, 0, 0, 1500, 770, hBk, 10, 10, SRCCOPY);
			TransparentBlt(hdc, 0, 0,
				GetSystemMetrics(SM_CXFULLSCREEN),
				GetSystemMetrics(SM_CYFULLSCREEN),
				hDst, 0, 0, 1500, 770, SRCCOPY);
		}
	};
}