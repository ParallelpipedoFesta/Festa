#pragma once

#include "common/glsl.h"
#include "common/model.h"
#include "common/shapes.h"

#define INVERSE_COLOR mat4(-1.0f, 0.0f, 0.0f, 0.0f,\
							0.0f, -1.0f, 0.0f, 0.0f,\
							0.0f, 0.0f, -1.0f, 0.0f,\
							1.0f, 1.0f, 1.0f, 1.0f)
#define GRAY_COLOR_(r,g,b) mat4(r,r,r, 0.0f,\
							g, g, g, 0.0f,\
							b, b, b, 0.0f,\
							0.0f, 0.0f, 0.0f, 1.0f)
#define GRAY_COLOR GRAY_COLOR_(0.2126f,0.7152f,0.0722f)


namespace Festa {
	struct VAOSource {
		VAOSource(Mesh& mesh) :mesh(&mesh) {}
		void release() {
			SafeDelete(vao);
		}
		~VAOSource() {
			release();
		}
		VAO& get() {
			if (!vao)vao = new VAO(*mesh);
			return *vao;
		}
		Mesh* mesh = 0;
		VAO* vao=0;
	};



	struct ShaderSource {
		ShaderSource(int type, const char* code) :type(type), code(code) {
			
		}
		void release() {
			SafeDelete(shader);
		}
		~ShaderSource() {
			release();
		}
		Shader& get() {
			if(!shader) shader = new Shader(type, code);
			return *shader;
		}
		Shader* shader=0;
		const char* code;
		int type;
	};

	struct GLSLProgramSource {
		GLSLProgramSource(const std::vector<ShaderSource*>& _shaders) :shaders(_shaders) {
			
		}
		void release() {
			SafeDelete(program);
		}
		~GLSLProgramSource() {
			release();
		}
		GLSLProgram& get() {
			if(!program) {
				std::vector<uint> inp; inp.resize(shaders.size());
				for (uint i = 0; i < shaders.size(); i++)inp[i] = shaders[i]->get().ID();
				program = new GLSLProgram(inp);
			}
			return *program;
		}
		
		GLSLProgram* program=0;
		std::vector<ShaderSource*> shaders;
	};

	extern Mesh RECT2MESH, RECT332MESH, CUBE332MESH;
	extern VAOSource RECT2, RECT332, CUBE332;

	extern GLSLProgramSource SKYBOX_PROGRAM;

	/*class GeometricModel :public Model {
	public:
		struct GeometricData {
			uint mesh=0;
			Material material;
			mat4 trans;
		};
		std::vector<GeometricData> data;
		GeometricModel() {}
		void operator*=(const mat4& mat) {
			for (GeometricData& d:data)d.trans *= mat;
		}
		AABB getAABB() {
			AABB ret;
			for (size_t i = 0; i < data.size(); i++)
				ret.update(meshes[data[i].mesh].aabb * data[i].trans);
			return ret;
		}
		void render()const {
			GLSLProgram* program = GLSLProgram::activatedProgram;
			mat4 mat = toMatrix();
			for (size_t i = 0; i < data.size(); i++) {
				program->setMat4("model", mat * data[i].trans);
				data[i].material.bind("material");
				//Material(MaterialData(vec3(0), vec3(0, 1, 0), vec3(0, 0, 0), 1)).bind("material");
				meshes[data[i].mesh].vao.draw();
			}
			if (currentAnimation != -1)
				animations[currentAnimation]->setMatrices();
		}
		void renderInstancedMeshes(uint count)const {
			GLSLProgram* program = GLSLProgram::activatedProgram;
			mat4 mat = toMatrix();
			for (size_t i = 0; i < data.size(); i++) {
				program->setMat4("model", mat * data[i].trans);
				data[i].material.bind("material");
				meshes[data[i].mesh].vao.drawInstanced(count);
			}
			if (currentAnimation != -1)
				animations[currentAnimation]->setMatrices();
		}
		void cube(const MaterialData& material, const mat4& trans) {
			meshes.resize(1); 
			meshes[0].vao.init(CUBE332MESH);
			meshes[0].aabb.update(CUBE332MESH);
			data.resize(1);
			data[0].material.init(material);
			data[0].mesh = 0;
			data[0].trans = trans;
		}
		void cone(const MaterialData& material, const mat4& trans, uint n = 25) {
			Mesh mesh;
			mesh.cone(n);
			meshes.resize(1);
			meshes[0].vao.init(mesh);
			meshes[0].aabb.update(mesh);
			data.resize(1);
			data[0].material.init(material);
			data[0].mesh = 0;
			data[0].trans = trans;
		}
		void cylinder(const MaterialData& material, const mat4& trans, uint n = 25) {
			Mesh mesh;
			mesh.cylinder(n);
			meshes.resize(1);
			meshes[0].vao.init(mesh);
			meshes[0].aabb.update(mesh);
			data.resize(1);
			data[0].material.init(material);
			data[0].mesh = 0;
			data[0].trans = trans;
		}
		void axis(const MaterialData& material, float ra = 0.5f, float ha = 1.5f, float rp = 0.15f, float hp = 2.5f, uint n = 25) {
			Mesh cone, cylinder;
			cone.cone(n), cylinder.cylinder(n);
			meshes.resize(2);
			meshes[0].vao.init(cone);
			meshes[0].aabb.update(cone);
			meshes[0].vao.init(cylinder);
			meshes[0].aabb.update(cylinder);
			data.resize(2);
			data[0].material.init(material);data[0].mesh = 0;data[0].trans = translate4(vec3(0.0f, hp, 0.0f)) * scale4(vec3(ra * 2.0f, ha, ra * 2.0f));
			data[1].material.init(material);data[1].mesh = 1;data[1].trans = scale4(vec3(rp * 2.0f, hp, rp * 2.0f));
		}
		void axes(float ambient = 0.2f, float diffuse = 0.8f, float ra = 0.5f, float ha = 1.5f, float rp = 0.15f, float hp = 2.5f, uint n = 25) {
			Mesh cone, cylinder;
			cone.cone(n), cylinder.cylinder(n);
			meshes.resize(2);
			meshes[0].vao.init(cone);
			meshes[0].aabb.update(cone);
			meshes[1].vao.init(cylinder);
			meshes[1].aabb.update(cylinder);
			mat4 conet = translate4(vec3(0.0f, hp, 0.0f)) * scale4(vec3(ra * 2.0f, ha, ra * 2.0f)),
				cylindert = scale4(vec3(rp * 2.0f, hp, rp * 2.0f));
			mat4 x = Rotation(-90.0f, VEC3Z).toMatrix(), y = identity4(), 
				z = Rotation(90.0f, VEC3X).toMatrix();
			MaterialData matx(VEC3X * ambient, VEC3X * diffuse, vec3(1.0f), 8.0f),
				maty(VEC3Y * ambient, VEC3Y * diffuse, vec3(1.0f), 8.0f),
				matz(VEC3Z * ambient, VEC3Z * diffuse, vec3(1.0f), 8.0f);
			data.resize(6);
			data[0].mesh = 0; data[0].material.init(matx); data[0].trans = x * conet;
			data[1].mesh = 1; data[1].material.init(matx); data[1].trans = x * cylindert;
			data[2].mesh = 0; data[2].material.init(maty); data[2].trans = y * conet;
			data[3].mesh = 1; data[3].material.init(maty); data[3].trans = y * cylindert;
			data[4].mesh = 0; data[4].material.init(matz); data[4].trans = z * conet;
			data[5].mesh = 1; data[5].material.init(matz); data[5].trans = z * cylindert;
		}
		void sphere(const MaterialData& material, const mat4& trans, uint longtitude = 30, uint latitude = 14) {
			Mesh mesh;mesh.sphere(longtitude,latitude);
			meshes.resize(1);
			meshes[0].vao.init(mesh);
			meshes[0].aabb.update(mesh);
			data.resize(1);
			data[0].material.init(material);
			data[0].mesh = 0;
			data[0].trans = trans;
		}
	};*/

	class Cubemap {
	public:
		Cubemap() {
			id = 0;
		}
		Cubemap(const std::vector<const Image*>& images) {
			init(images);
		}
		Cubemap(const std::vector<std::string>& files) {
			load(files);
		}
		Cubemap(const std::string& folder, const std::vector<std::string>& files) {
			load(folder, files);
		}
		void release() {
			glDeleteTextures(1, &id);
		}
		~Cubemap() {
			//release();
		}
		void init(const std::vector<const Image*>& images) {
			glGenTextures(1, &id);
			bind();
			for (uint i = 0; i < images.size(); i++)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
					images[i]->width(), images[i]->height(), 0, GL_BGR, GL_UNSIGNED_BYTE, images[i]->data());
			filter();
			wrapping();
		}
		void load(const std::vector<std::string>& files) {
			std::vector<const Image*> images(files.size());
			for (uint i = 0; i < files.size(); i++)
				images[i] = new Image(files[i]);
			init(images);
		}
		void load(const std::string& folder, const std::vector<std::string>& files) {
			std::vector<const Image*> images(files.size());
			const std::string prefix = folder + "/";
			for (uint i = 0; i < files.size(); i++)
				images[i] = new Image(prefix + files[i]);
			init(images);
		}
		void bind()const {
			glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		}
		static void wrapping(int param = GL_CLAMP_TO_EDGE) {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, param);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, param);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, param);
		}
		static void filter(int param = GL_LINEAR) {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, param);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, param);
		}
		void draw(const Camera& camera, GLSLProgram* program = 0) {
			bool cullFace = glIsEnabled(GL_CULL_FACE);
			if (cullFace)glDisable(GL_CULL_FACE);
			if (!program)program = &SKYBOX_PROGRAM.get();
			program->bind();
			program->set("projection", camera.projectionMatrix());
			program->set("view", mat4(mat3(camera.viewMatrix())));
			bind();
			CUBE332.get().Draw();
			if (cullFace)glEnable(GL_CULL_FACE);
		}
	private:
		uint id;
	};
	struct GLCanvas {
		const Camera* camera=0;
		vec3 prev = vec3(0.0f);
		GLCanvas() {}
		GLCanvas(const Camera* _camera) :camera(_camera) {}
		static void enable(int st) {
			glEnable(st);
		}
		static void disable(int st) {
			glDisable(st);
		}
		void setCamera(const Camera* _camera) {
			camera = _camera;
		}
		static void begin() {
			GLSLProgram::unbind();
			VAO::Unbind();
			Texture::unbind();
		}
		static void setColor(const Color& color) {
			const vec4 c = color.toVec4();
			glColor4f(c.x, c.y, c.z, c.w);
		}
		void emitVertex2D(const vec3& vertex) {//ndc with z in [0,1]
			if (vertex.z < 0.0f || vertex.z>1.0f) {
				glVertex3f(prev.x, prev.y, prev.z);
				return;
			}
			glVertex3f(vertex.x,vertex.y,vertex.z);
			prev = vertex;
		}
		void emitVertex2D(const Color& color, const vec3& vertex) {
			setColor(color);
			emitVertex2D(vertex);
		}
		void emitVertex3D(const vec3& vertex) {
			if (!camera)return;
			vec3 ndc = camera->world2ndc(vertex);
			ndc.z = ndc.z * 0.5f + 0.5f;
			emitVertex2D(ndc);
			//if (KEY_DOWN('1'))printvec3(pos);
		}
		void emitVertex3D(const Color& color, const vec3& vertex) {
			setColor(color);
			emitVertex3D(vertex);
		}
		void rect2D(const vec2& a, const vec2& b) {
			glDisable(GL_DEPTH_TEST);
			glBegin(GL_QUADS);
			emitVertex2D(vec3(a.x, a.y, 0.0f));
			emitVertex2D(vec3(a.x, b.y, 0.0f));
			emitVertex2D(vec3(b.x, b.y, 0.0f));
			emitVertex2D(vec3(b.x, a.y, 0.0f));
			glEnd();
			glEnable(GL_DEPTH_TEST);
		}
	};

	

}

