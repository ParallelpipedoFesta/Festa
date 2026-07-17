#pragma once

#include "Festa.hpp"
#include "include/font.h"
#include "UI.h"

#define PROJECT_NAME "Quantum Computing Laboratory"


using namespace Festa;
using namespace std;

inline void bindCamera(const Camera& camera) {
	if (!GLSLProgram::activatedProgram)return;
	GLSLProgram::activatedProgram->set("projection", camera.projection());
	GLSLProgram::activatedProgram->set("view", camera.viewMatrix());
}

void activateHighlighted(const Camera& camera, const Color& col);
void activateHighlighted(const Color& col);

extern Window window;
extern GLSLProgram phong;
extern UserInterface ui;

struct Text3D {
	static GLSLProgram shader;
	static FreetypeFont* font;
	Texture texture;
	vec3 dim = vec3(0.0f);
	vec3 col = vec3(0.0f);
	Transformation trans;
	void GenerateTexture(const std::string& text, const vec2& scale) {
		if (!font)return;
		Image* img = font->GetImage(text, Color(255), Color(0));
		texture.Generate(*img);
		Texture::wrapping2D(GL_CLAMP_TO_BORDER);
		Texture::borderColor();
		dim.x = img->width() * scale.x;
		dim.y = img->height() * scale.y;
		dim.z = 1.0;
		delete img;
	}
	static void Activate(const Camera& camera, const Light& light) {
		shader.bind();
		light.bind(shader, "lights[0]");
		bindCamera(camera);
	}
	void Render() {
		Transformation t = trans; t._scale(dim);
		shader.bind();
		shader.set("model", t.toMatrix());
		shader.bindTexture("textMap", 0, texture);
		shader.set("Diffuse", col);
		RECT332.get().Draw();
	}
	void Render(const Transformation& t) {
		shader.bind();
		shader.set("model", t.toMatrix());
		shader.bindTexture("textMap", 0, texture);
		shader.set("Diffuse", col);
		RECT332.get().Draw();
	}
};
