#include "App.h"
#include "UI.h"
#include "Vacuum.h"
#include "Tutorial.h"

//#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

using namespace Festa;
using namespace std;

Window window(1200, 900, PROJECT_NAME, 4.4);
Camera camera;
GLSLProgram phong;
UserInterface ui;
GLSLProgram highlightRender;
GLSLProgram Text3D::shader;
FreetypeFont font_times("fonts/times.ttf", 40);
//FreetypeFont font_times("fonts/times.ttf", 100);
FreetypeFont* Text3D::font = &font_times;
FreetypeFont* TutorialPage::font = &font_times;
TextureMap QuantumGate::iconMap;

void activateHighlighted(const Camera& camera, const Color& col) {
	highlightRender.bind();
	bindCamera(camera);
	highlightRender.set("col", col.toVec4());
}

void activateHighlighted(const Color& col) {
	highlightRender.bind();
	highlightRender.set("col", col.toVec4());
}



GameControl gameControl;

class Vacuum {
public:
	Camera camera;
	CircuitEntity circuit;
	MyWindow mWindow;
	Vacuum() {
		Init();
	}
	void Render() {
		gameControl.Update(window);

		camera.projection = Projection(window.viewport, 45.0f, 1.0f, 1000.0f);
		phong.bind();
		bindCamera(camera);

		if (gameControl.GetKey(GLFW_KEY_TAB).clicked()) {
			mTutorialActivated = !mTutorialActivated;
		}
		if (mTutorialActivated) {
			const vec3 pos = camera.position();
			camera.position() = vec3(0.0f, 20.0f, 0.0f);
			Light light(vec3(0.0f, 5.0f, 0.0f), vec3(1.0f, 1.0f, 0.9f), 0.0f, 0.04f, 0.01f);
			bindCamera(camera);
			light.bind(phong, "lights[0]");
			RenderGround();
			
			Text3D::Activate(camera, light);
			mTutorial.Render(camera);
			camera.position() = pos;
			return;
		}

		Interact();
		mLight.pos = camera.position(); mLight.pos.y = 5.0f;
		mLight.bind(phong, "lights[0]");
		RenderGround();

		Text3D::Activate(camera, mLight);
		bool notEditing;
		//if(notEditing)notEditing = !RenderUI();
		notEditing = !RenderUI();
		//if (notEditing)circuit.Interact(camera);
		circuit.Render(camera);
		mWindow.Main(camera, notEditing);

	}
private:
	Light mLight;

	struct MenuButton {
		bool open = false;
		std::string label;
		std::vector<int> buttons;
		bool render(CircuitEntity& circuit, MyWindow& mWindow, float& pos) {
			const vec2 margin(10.0f);
			const vec2 half(60.0f);
			const vec2 text(100.0f);
			pos += margin.y;
			const vec2 center = vec2(float(window.width()) - half.x - margin.x, pos + half.y);
			int code = ui.ButtonArea(center, half * 2.0f);
			auto box = ui.getFont().GetBox(label);
			ui.RenderText(label, center, Color(255, 255, 255), ui.getFont().height() * std::min(text.x / float(box.w), text.y / float(box.h)), UI_TEXT_CENTERED);
			if (code == (UI_BUTTON_HOVERED | UI_BUTTON_CLICKED))open = !open;
			bool hovered = code & UI_BUTTON_HOVERED || window.mouse.pos.x > window.width() - int(margin.x) * 2 + int(half.x) * 2;
			pos += half.y * 2.0f;
			if (!open) {
				return hovered;
			}
			float x = center.x;
			for (int b : buttons) {
				x -= half.x * 2.0f + margin.x;
				if (circuit.mGateClasses[b].renderButton(x, center.y, half * 2.0f, hovered))
					mWindow.SelectGate(b);
			}
			return hovered;
		}
	};
	std::list<MenuButton> mMenu;
	Tutorial mTutorial;
	bool mTutorialActivated = false;

	void CompileShaders() {
		Shader phong_vs("shaders/phong.vs");
		Shader phong_fs("shaders/phong.fs");
		phong.attach({ phong_vs, phong_fs });
		Text3D::shader.compile({ "shaders/text3.vs", "shaders/text3.fs" });
		Text3D::font = &font_times;
		ui.setFont(&font_times);
		highlightRender.compile({ "shaders/highlight/highlight.vs", "shaders/highlight/highlight.fs" });
	}

	void BuildMenu() {
		MenuButton IO;
		IO.label = "  I/O  ";
		IO.buttons = { GATE_0, GATE_1, GATE_INPUT, GATE_OUTPUT, GATE_MEASURE, GATE_BLOCHSPHERE };
		mMenu.push_back(IO);

		MenuButton Single;
		Single.label = "Single Qubit";
		Single.buttons = { GATE_X, GATE_Y, GATE_Z, GATE_HADAMARD };
		mMenu.push_back(Single);

		MenuButton Controlled;
		Controlled.label = "Controlled";
		Controlled.buttons = { GATE_CNOT };
		mMenu.push_back(Controlled);

		MenuButton Oracle;
		Oracle.label = "Oracle";
		Oracle.buttons = { GATE_PERFORMER, GATE_ORACLE };
		mMenu.push_back(Oracle);
	}

	void Init() {
		CompileShaders();

		camera.position() = vec3(0.0f, 10.0f, 0.0f);
		camera.rot.pitch = -90.0f;
		camera.ApplyRotation();

		mLight = Light(vec3(0.0f, 5.0f, 0.0f), vec3(1.0f, 1.0f, 0.9f), 0.0f, 0.07f, 0.04f);
		circuit.Init(vec3(200.0f));

		BuildMenu();

		InitGates();

		mWindow.BindCircuit(&circuit);
		gameControl.RegisterKey(GLFW_KEY_TAB);

		JsonData cfg; cfg.load("cfg.json");
		mTutorial.Config(cfg["tutorial"], ivec2(1200, 900), 50);
		mTutorial.Select("1.1 Qubit");
	}
	void InitGates();
	void RenderGround() {
		phong.bind();
		Material mat(MaterialData(vec3(1.0f), vec3(1.0f), vec3(1.0f), 8.0f));
		Transformation trans;
		trans.y = vec3(0.0f, 0.0f, 1.0f);
		trans.z = vec3(0.0f, -1.0f, 0.0f);
		trans._scale(vec3(100.0f, 1.0f, 100.0f));
		phong.set("model", trans.toMatrix());
		mat.bind(phong, "material");
		RECT332.get().Draw();
	}

	void Interact() {
		float advance = 8.0f * window.interval();
		if (window.GetKey(GLFW_KEY_LEFT_SHIFT))advance *= 3.0f;
		float advance_y = 0.5f;
		if (window.GetKey(GLFW_KEY_W)) {
			camera.position().z -= advance;
		}
		if (window.GetKey(GLFW_KEY_S)) {
			camera.position().z += advance;
		}
		if (window.GetKey(GLFW_KEY_A)) {
			camera.position().x -= advance;
		}
		if (window.GetKey(GLFW_KEY_D)) {
			camera.position().x += advance;
		}
		if (window.mouse.scroll < 0 && camera.position().y + advance_y < CAMERA_Y_MAX) {
			camera.position().y += advance_y;
		}
		if (window.mouse.scroll > 0 && camera.position().y - advance_y > CAMERA_Y_MIN) {
			camera.position().y -= advance_y;
		}
	}
	bool RenderUI() {
		bool hovered = false;
		float y = 0.0f;
		for (auto& menu : mMenu) {
			hovered |= menu.render(circuit, mWindow, y);
		}
		return hovered;
	}
};


Vacuum vacuum;

void test() {
	srand(time(0));
	srand(randint() + time(0));
	randf(); randf();
	QuantumEntanglement circuit;
	uchar num = 4;
	circuit.Allocate(num);

	complex F = sqrtf(0.5f);
	ComplexMatrix Hadamard({
		F, F,
		F, -F
		}, 2, 2);
	ComplexMatrix CNOT({
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1,
		0, 0, 1, 0
		}, 4, 4);

	circuit.Transform({ 0 }, Hadamard);
	circuit.Transform({ 0, 1 }, CNOT);

	for (uchar i = 0; i < num; i++) {
		//circuit.Measure(i);
	}
	circuit.Measure(0);
	for (uchar i = 0; i < num; i++) {
		std::cout << "q" << i << " " << int(circuit.GetState(i)) - 1 << "\n";
	}
}

void Vacuum::InitGates() {
	QuantumGate::iconMap.Init(ivec2(100), ivec2(4), 4);
	for (QuantumGate& gate : circuit.mGateClasses)gate.createIcon();
	QuantumGate::iconMap.GenerateTexture();
}

int main() {
	window.EnableImGui();

	ui.BindWindow(window);

	Model duck("models/duck/duck.obj");
	duck.position() = vec3(0.0f);

	bool firstFrame = true;
	bool operating = false;

	while (window.update()) {
		if (KEY_DOWN(VK_ESCAPE))break;
		vacuum.Render();
	}
	return 0;
}