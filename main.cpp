#include "Festa.hpp"
#include "include/utils/game.h"
#include "include/fuman.h"

using namespace std;
using namespace Festa;

void init() {
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void bindCamera(const Camera& camera) {
	if (!GLSLProgram::activatedProgram)return;
	GLSLProgram::activatedProgram->set("projection", camera.projection());
	GLSLProgram::activatedProgram->set("view", camera.viewMatrix());
}
ShaderSource TEXTURE_VS(GL_VERTEX_SHADER,
	"#version 330 core\n"
	"layout(location=0)in vec2 aPos;"
	"uniform mat4 posTrans;out vec2 TexCoords;"
	"void main(){"
	"   gl_Position=posTrans*vec4(aPos,0.0f,1.0f);\n"
	"	TexCoords=vec2(aPos.x*0.5f+0.5f, -aPos.y*0.5f+0.5f);\n"
	"}"),
	TEXTURE_FS(GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"uniform sampler2D tex;uniform mat4 colorTrans;\n"
		"in vec2 TexCoords;\n"
		"out vec4 FragColor;\n"
		"void main(){FragColor=colorTrans * texture(tex,TexCoords);}"
	);
GLSLProgramSource TEXTURE_PROGRAM(std::vector<ShaderSource*>{ &TEXTURE_VS, &TEXTURE_FS });
inline void drawTexture(const Texture& texture, const mat4& posTrans = mat4(1.0f), const mat4& colorTrans = mat4(1.0f)) {
	GLSLProgram& p = TEXTURE_PROGRAM.get();
	p.bind();
	p.set("posTrans", posTrans);
	p.set("colorTrans", colorTrans);
	p.bindTexture("tex", 0, texture);
	RECT2.get().Draw();
}

GLWindow window(1200, 900, "Lab di Festa", 4.4, init);

Shader vs("shaders/phong.vs");
Shader fs("shaders/albedo.fs");
GLSLProgram phong({vs, fs});
Camera camera;

Cubemap skybox("skybox", { "right.jpg","left.jpg","top.jpg","bottom.jpg","front.jpg","back.jpg" });
PointLight light(vec3(0.0f, 10.0f, 0.0f), vec3(1.0f), vec3(1.0f), vec3(1.0f));

Model board("models/chessboard/main.obj");
Model duck("models/duck/duck.obj");
Model wolf("models/wolf/7.4.fbx");
//Model natale("models/natale.fm");


ShaderSource POSTPROCESS_VS(GL_VERTEX_SHADER,
	"#version 330 core\n"
	"layout(location=0)in vec2 aPos;"
	"out vec2 TexCoords;"
	"void main(){"
	"   gl_Position=vec4(aPos,1.0f,1.0f);\n"
	"	TexCoords=vec2(aPos.x*0.5f+0.5f, aPos.y*0.5f+0.5f);\n"
	"}");

struct DeferredShading {
	GBuffer gBuffer;
	//FrameRenderBuffer gBuffer;
	GLSLProgram stage1;
	GLSLProgram stage2;
	Texture frame;
	DeferredShading() {
		init();
	}
	void init() {
		Shader gbuffer_vs("shaders/gbuffer.vs");
		Shader gbuffer_fs("shaders/gbuffer.fs");
		stage1.attach({ gbuffer_vs, gbuffer_fs });

		Shader df_fs("shaders/df1.fs");
		stage2.attach({POSTPROCESS_VS.get(), df_fs });
		gBuffer.Generate(window.width(), window.height());
		//gBuffer.init(window.width(), window.height());
		//gBuffer.init(1200, 900);
		//frame.init(gBuffer);
	}
	void begin() {
		gBuffer.Begin();
		stage1.bind();
		//phong.bind();
	}
	void render() {
		FBO::unbind();
		stage2.bindTexture("gPosition", 0, gBuffer.gPosition);
		stage2.bindTexture("gNormal", 1, gBuffer.gNormal);
		stage2.bindTexture("gAlbedoSpec", 2, gBuffer.gAlbedoSpec);
		stage2.set("viewPos", camera.position());
		RECT2.get().Draw();
	}
}df;

struct Light :public GLSLStructure {
	vec3 pos = vec3(0.0f);
	vec3 color = vec3(0.0f);
	float radius = 0.0f, linear = 0.0f, quadratic = 0.0f;
	Light() {}
	Light(const vec3& _pos, const vec3& _color, float _constant, float _linear, float _quadratic)
	:pos(_pos), color(_color), linear(_linear), quadratic(_quadratic){
		const float maxBrightness = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
		radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (_constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
	}
	void bind(const GLSLProgram& p, const std::string& name)const {
		addAttribute(p, name, "Position", pos);
		addAttribute(p, name, "Color", color);
		addAttribute(p, name, "Linear", linear);
		addAttribute(p, name, "Quadratic", quadratic);
		addAttribute(p, name, "Radius", radius);
	}
};

inline void drawCuboid(const Material& material, const Transformation& trans = Transformation()) {
	if (!GLSLProgram::activatedProgram)return;
	GLSLProgram* program = GLSLProgram::activatedProgram;
	program->set("model", trans.toMatrix());
	program->set("material", material);
	CUBE332.get().Draw();
}

const int num_lights = 100;
std::vector<Light> lights; 
void generate_lights() {
	for (int i = 0; i < num_lights; i++) {
		const float constant = 1.0f;
		//const float linear = 0.7f;
		//const float quadratic = 1.8f;
		const float linear = float(randf());
		const float quadratic = float(randf(0.1, 2.0));
		double sigma = 6.0;
		const double col_min = 0.0f;
		const double col_max = 1.0f;
		//vec3 pos(0.0f, 1.0f, 0.0f);
		//vec3 color(1.0f);
		vec3 pos = vec3(float(normalf(0.0, sigma)), 1.0f, float(normalf(0.0, sigma)));
		vec3 color = vec3(float(randf(col_min, col_max)), float(randf(col_min, col_max)), float(randf(col_min, col_max)));
		lights[i] = Light(pos, color, constant, linear, quadratic);
	}
}


int main() {
	FumanEngine::instance.Init("t.fanim","fuman/types.json");
	std::list<Fuman> fumans;
	Path folder = "fuman";
	std::list<Path> files;
	folder.glob(files, "*.fuman");
	for (Path path : files) {
		std::cout << path << endl;
		fumans.push_back(Fuman());
		fumans.back().Load(path);
		//fumans.back().LoadAnimations();
	}
	auto currentAnimation = FumanEngine::instance.mAnimations.begin();
	for (Fuman& fuman : fumans) {
		fuman.GetAnimation(currentAnimation->first)->SetIsLooped(true);
		fuman.GetAnimation(currentAnimation->first)->Activate();
	}


	lights.resize(num_lights);
	generate_lights();
	board.translate(vec3(0.0f, -2.0f, 0.0f));
	board.setRotation(Rotation(-90.0f, VEC3X));
	wolf.scale(vec3(0.05f));
	wolf.setPosition(vec3(2.0f, 0.0f, 0.0f));
	wolf.setAnimation(1);
	duck.position() = vec3(0.0f, 1.5f, 0.0f);

	CameraController1 cc(&window, &camera);
	camera.position() = vec3(0.0f, 1.0f, 5.0f);

	while (window.update()) {
		camera.projection = Projection(window.viewport, 45.0f, 1.0f, 1000.0f);
		cc.update();

		//skybox.draw(camera);
		df.begin();
		bindCamera(camera);
		//phong.set("pointLights[0]", light);
		board.render();
		duck.render();
		wolf.render();

		if (window.mouse.right.clicked()) {
			for (Fuman& fuman : fumans) {
				fuman.GetAnimation(currentAnimation->first)->Deactivate();
			}
			currentAnimation++;
			if (currentAnimation == FumanEngine::instance.mAnimations.end())
				currentAnimation = FumanEngine::instance.mAnimations.begin();
			for (Fuman& fuman : fumans) {
				fuman.GetAnimation(currentAnimation->first)->SetIsLooped(true);
				fuman.GetAnimation(currentAnimation->first)->Activate();
			}
		}
		const float stride = 2.0f;
		float pos = -float(fumans.size()) * stride * 0.5f;
		for (Fuman& fuman : fumans) {
			fuman.Update(window.interval());
			Transformation trans; trans.setPosition(vec3(pos, 1.0f, 0.0f));
			pos += stride;
			fuman.Render(trans);
		}

		if (wolf.animation()->isFinished())wolf.animation()->reset();
		wolf.updateAnimation();
		//natale.render();
		df.stage2.bind();
		for (int i = 0; i < num_lights;i++) {
			vec3 pos = lights[i].pos;
			lights[i].pos = pos + vec3(cosf(getTime()*0.5f), 0.0f, sinf(getTime())) * 2.0f;
			df.stage2.set("lights[" + toString(i) + "]", lights[i]);
			lights[i].pos = pos;
		}
		df.render();
		if (KEY_DOWN('P'))duck.position().y += 0.001f;
		if (KEY_DOWN('L'))duck.position().y -= 0.001f;
		if(KEY_DOWN('9'))generate_lights();
		if (!KEY_DOWN('0'))continue;
		phong.bind();
		phong.set("pointLights[0]", light);
		bindCamera(camera);
		for (int i = 0; i < num_lights; i++) {
			Material mat(MaterialData(lights[i].color, lights[i].color, vec3(0.0f), 1.0f));
			Transformation trans;
			trans.setPosition(lights[i].pos);
			drawCuboid(mat, trans);
			phong.set("model", trans.toMatrix());
			phong.set("material", mat);
			CUBE332.get().Draw();
		}
	}
	return 0;
}