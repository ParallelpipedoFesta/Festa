#pragma once

#include "common/frame.h"
#include "common/transformation.h"
#include "common/glsl.h"

namespace Festa {
	
	ShaderSource Festa::TEXTURE_VS(GL_VERTEX_SHADER,
		"#version 440 core\n"
		"layout(location=0)in vec2 Pos;"
		"uniform mat4 posTrans;out vec2 texCoord;"
		"void main(){"
		"   gl_Position=posTrans*vec4(Pos,0.0f,1.0f);\n"
		"	texCoord=vec2(Pos.x*0.5f+0.5f,-Pos.y*0.5f+0.5f);\n"
		"}"),
		Festa::TEXTURE_FS(GL_FRAGMENT_SHADER,
			"#version 440 core\n"
			"uniform sampler2D tex;uniform mat4 colorTrans;\n"
			"in vec2 texCoord;\n"
			"out vec4 FragColor;\n"
			"void main(){FragColor=colorTrans*texture(tex,texCoord);}"
		),
	GLSLProgramSource Festa::TEXTURE_PROGRAM(std::vector<ShaderSource*>{ &TEXTURE_VS, & TEXTURE_FS });
	inline void drawTexture(const Texture& texture, const mat4& posTrans = mat4(1.0f), const mat4& colorTrans = mat4(1.0f)) {
		GLSLProgram& p = TEXTURE_PROGRAM.get();
		p.bind();
		p.setMat4("posTrans", posTrans);
		p.setMat4("colorTrans", colorTrans);
		texture.bind("tex", 0);
		RECT2.get().draw();
	}
	inline void blitImage(const Window& window, const Image& img, const vec2& pos, const vec2& size = vec2(-1.0f)) {
		vec2 s = size;
		//if (s.x < 0.0f)s.x = float(img.width()), s.y = float(img.height());
		const vec2 vsize = window.viewport.size();
		vec2 position = vec2(pos.x - vsize.x / 2.0f, vsize.y / 2.0f - pos.y) / (vsize / 2.0f);
		mat4 trans = translate4(vec3(position, 0.0f)) * scale4(vec3(s / vsize, 0.0f));
		drawTexture(img, trans, identity4());
	}
	inline void drawTexture3D(const Texture& texture, const Camera& camera, const Transformation& trans, const mat4& colorTrans = mat4(1.0f)) {
		static GLSLProgramSource program(std::vector<ShaderSource*>{&STANDARD_VS, & TEXTURE_FS });
		GLSLProgram& p = program.get();
		p.bind();
		camera.bind();

		p.setMat4("colorTrans", colorTrans);
		p.setMat4("model", trans.toMatrix());
		texture.bind("tex", 0);
		RECT332.get().draw();
	}
	inline void drawRect(const Material& material, const Transformation& trans = Transformation()) {
		if (!GLSLProgram::activatedProgram)return;
		GLSLProgram* program = GLSLProgram::activatedProgram;
		program->setMat4("model", trans.toMatrix());
		material.bind("material");
		RECT332.get().draw();
	}
	inline void drawCuboid(const Material& material, const Transformation& trans = Transformation()) {
		if (!GLSLProgram::activatedProgram)return;
		GLSLProgram* program = GLSLProgram::activatedProgram;
		program->setMat4("model", trans.toMatrix());
		material.bind("material");
		CUBE332.get().draw();
	}


	inline void renderTrailer(GLWindow& window, Resources& resources) {
		const float duration = 0.2f * 250000.0f;
		const float duration2 = 1.0; float swt = INFINITY;
		const float light_miny = -2.0f, light_maxy = 4.0f;
		const float sole_miny = -2.0f, sole_maxy = 1.7f;

		FontRender& font = resources["font"]["times"].to<FontRender>();

		Camera camera;
		GLCanvas canvas(&camera);

		GLSLProgram& phong = resources["GLSL"]["phong"].to<GLSLProgram>();
		GLSLProgram& sole = resources["GLSL"]["sole"].to<GLSLProgram>();

		PointLight light(vec3(0.0f, light_miny, 0.0f), vec3(0.5f, 0.31f, 0.57f), vec3(0.7f, 0.57f, 0.7f), vec3(1.0f, 0.77f, 0.98f), 0.5f, 0.01f, 0.01f);

		const std::string text = "Piccola Festa's Games";
		int phase = 0;

		camera.pos = vec3(0.0f, 1.3f, 4.4f);
		camera.rot.pitch = -5.0f;

		Material mat(MaterialData(vec3(0.44f, 0.88f, 0.33f), vec3(0.44f, 0.93f, 0.3f), vec3(1.0f), 2.0f));
		glClearColor(0, 0, 0, 255);

		int width = window.width(), height = window.height();
		FBO fbo(width, height); Texture frame(fbo);
		vec3 solePos = vec3(0.0f, 0.0f, -10.0f) + camera.up() * sole_miny;

		Sound& riseSound = resources["audio"]["space"].to<Sound>();
		Sound& swtSound = resources["audio"]["switch"].to<Sound>();
		riseSound.play();

		while (window.update() == 1) {
			if (getTime() - swt > duration2)break;
			if (phase == 1) {
				auto box = font.getBox(text);
				font.render(text, vec2((window.width() - box.w) * 0.5f, window.height() * 0.5f), Color(255, 255, 255));
				continue;
			}
			if (width != window.width() || height != window.height()) {
				width = window.width(), height = window.height();
				fbo.release();
				frame.release();
				fbo.init(width, height);
				frame.init(fbo);
			}

			if (light.pos.y < light_maxy) {
				//light.pos.y = light.pos.y + (light_maxy - light_miny) / duration * float(window.interval());
				//solePos += camera.up() * (sole_maxy - sole_miny) / duration * float(window.interval());
				light.pos.y = light.pos.y + (light_maxy - light_miny) / duration;
				solePos += camera.up() * (sole_maxy - sole_miny) / duration;
			}
			else if (phase == 0) {
				light.pos.y = light_maxy;
				solePos = vec3(0.0f, 0.0f, -10.0f) + camera.up() * sole_maxy;
				phase = 1;
				swt = getTime();
				riseSound.finish();
				swtSound.play();
			}

			camera.shape = CameraShape(window.viewport, 45.0f, 1.0f, 100.0f);
			fbo.begin();

			phong.bind();
			camera.bind();
			light.bind("pointLights[0]");

			Transformation trans;
			trans._scale(vec3(100.0f, 1.0f, 100.0f));
			drawCuboid(mat, trans);

			FBO::unbind();
			sole.bind();
			frame.bind("tex", 0);
			sole.setVec3("solePos", solePos - vec3(0.0f, 0.7f * float(window.width()) / float(window.height()), 0.0f));
			sole.setFloat("size", float(window.width()));
			RECT2.get().draw();


			auto box = font.getBox(text);
			font.render(text, vec2((window.width() - box.w) * 0.5f, window.height() * 0.5f), Color(vec3(0.5f * sinf(getTime() * 2.0f) + 0.5f)));

			canvas.begin();
			glBegin(GL_QUADS);
			canvas.setColor(Color(96, 194, 64, 250));
			canvas.emitVertex2D(vec3(0.0f, 0.0f, 0.0f));
			canvas.emitVertex2D(vec3(0.0f, 0.0f, 0.0f));
			canvas.setColor(Color(96, 0, 0, 0));
			canvas.emitVertex2D(vec3(0.0f, 0.0f, 0.0f));
			canvas.emitVertex2D(vec3(0.0f, 0.0f, 0.0f));
			glEnd();
		}
		riseSound.finish();
		swtSound.finish();
	}
}

}