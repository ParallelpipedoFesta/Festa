#pragma once

#include "common.h"
#include "frame.h"

#define INFOLOG_LEN 512

namespace Festa {
	class Shader {
	public:
		Shader() {}
		Shader(int type, const char* source) :mType(type) {
			compile(source);
		}
		Shader(int type, const std::string& source) :mType(type) {
			compile(source.c_str());
		}
		Shader(const Path& file) {
			load(file);
		}
		void release() {
			if(id)glDeleteShader(id);
			id = 0;
			mType = 0;
		}
		~Shader() {
			release();
		}
		operator uint()const {
			return id;
		}
		bool load(const Path& file) {
			const std::string ext = file.extension();
			if (ext == "vs")mType = GL_VERTEX_SHADER;
			else if (ext == "fs")mType = GL_FRAGMENT_SHADER;
			else if (ext == "gs")mType = GL_GEOMETRY_SHADER;
			else if (ext == "cs")mType = GL_COMPUTE_SHADER;

			if (!mType) {
				LOGGER.error("Invalid shader extension: " + file.toString());
				return true;
			}
			std::string source; File f(file); f.readLines(source);
			return compile(source.c_str(), file);
		}
		void load(int type, const Path& file) {
			std::string source;  
			File f(file); 
			f.readLines(source);
			mType = type;
			compile(source.c_str(), file);
		}
		bool compile(const char* source, const std::string& name = "shader") {
			id = glCreateShader(mType);
			glShaderSource(id, 1, &source, NULL);
			glCompileShader(id);

			int success;
			char infoLog[INFOLOG_LEN];
			glGetShaderiv(id, GL_COMPILE_STATUS, &success);

			if (success)return false;

			glGetShaderInfoLog(id, INFOLOG_LEN, NULL, infoLog);
			LOGGER.error(name + " ShaderCompilationError: " + std::string(infoLog));
			return true;
		}
		uint ID()const {
			return id;
		}
		int type()const {
			return mType;
		}
	private:
		int mType = 0;
		uint id = 0;
	};

	

	class GLSLProgram {
	public:
		typedef enum : uchar {
			GLSL_FLOAT,
			GLSL_VEC2,
			GLSL_VEC3,
			GLSL_VEC4,
			GLSL_MAT2,
			GLSL_MAT3,
			GLSL_MAT4,

			GLSL_INT,
			GLSL_VEC2I,
			GLSL_VEC3I,
			GLSL_VEC4I,

			GLSL_STRUCTURE
		}GLSLDataType;
		struct GLSLStructure {
			GLSLStructure() {}
			virtual void bind(const GLSLProgram& p, const std::string& name)const = 0;
			template<typename T>
			void addAttribute(const GLSLProgram& p, const std::string& name, const std::string& attrName, const T& value)const {
				p.set(name+"."+attrName, value);
			}
		};

		static GLSLProgram* activatedProgram;
		GLSLProgram() {
			id = 0;
		}
		
		GLSLProgram(const std::vector<uint>& shaders) {
			attach(shaders);
		}
		GLSLProgram(const std::vector<Path>& shaders) {
			compile(shaders);
		}
		GLSLProgram(const GLSLProgram& program) {
			id = program.id;
		}
		void release() {
			if (id) {
				glDeleteProgram(id);
				id = 0;
			}
		}
		~GLSLProgram() {
			release();
		}
		void attachOne(uint shader)const {
			glAttachShader(id, shader);
		}
		void attach(const std::vector<uint>& shaders) {
			id = glCreateProgram();
			for (uint shader : shaders)attachOne(shader);
			link();
		}
		void compileOne(const Path& file) {
			Shader shader(file);
			attachOne(shader);
		}
		void compile(const std::vector<Path>& shaders) {
			id = glCreateProgram();
			for (const Path& file : shaders)compileOne(file);
			link();
		}
		void transformFeedbackVaryings(const std::vector<const char*>& varyings, uint mode = GL_INTERLEAVED_ATTRIBS) {
			glTransformFeedbackVaryings(id, int(varyings.size()), &varyings[0], mode);
		}
		void generate() {
			id = glCreateProgram();
		}
		void link()const {
			glLinkProgram(id);
			int success;
			glGetProgramiv(id, GL_LINK_STATUS, &success);
			if (!success) {
				char infoLog[INFOLOG_LEN];
				glGetProgramInfoLog(id, INFOLOG_LEN, NULL, infoLog);
				LOGGER.error(std::string("ProgramLinkingError: ") + std::string(infoLog));
			}
		}
		void bind() {
			glUseProgram(id);
			activatedProgram = this;
		}
		static void unbind() {
			glUseProgram(0);
			activatedProgram = 0;
		}
		int Locate(const std::string& name)const {
			return glGetUniformLocation(id, name.c_str());
		}
		int GetLocation(const std::string& name)const {
			int ret = glGetUniformLocation(id, name.c_str());
			/*if () {
				LOGGER.error("Variable not found: " + name);
				return -1;
			}*/
			return ret;
		}
		void set(const std::string& name, bool value) const{
			glUniform1i(GetLocation(name), (int)value);
		}
		void set(const std::string& name, int value) const{
			glUniform1i(GetLocation(name), value);
		}
		void set(const std::string& name, float value) const{
			glUniform1f(GetLocation(name), value);
		}
		void set(const std::string& name, const vec2& value) const{
			glUniform2fv(GetLocation(name), 1, &value[0]);
		}
		void set(const std::string& name, const vec3& value) const{
			glUniform3fv(GetLocation(name), 1, &value[0]);
		}
		void set(const std::string& name, const vec4& value) const{
			glUniform4fv(GetLocation(name), 1, &value[0]);
		}
		void set(const std::string& name, const mat2& mat) const{
			glUniformMatrix2fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
		}
		void set(const std::string& name, const mat3& mat) const{
			glUniformMatrix3fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
		}
		void set(const std::string& name, const mat4& mat) const{
			glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, &mat[0][0]);
		}
		void set(const std::string& name, void* ptr, GLSLDataType dt)const {
			switch (dt) {
			case GLSL_FLOAT:
				glUniform1f(GetLocation(name), *(float*)ptr);
				break;
			case GLSL_VEC2:
				glUniform2fv(GetLocation(name), 1, (float*)ptr);
				break;
			case GLSL_VEC3:
				glUniform3fv(GetLocation(name), 1, (float*)ptr);
				break;
			case GLSL_VEC4:
				glUniform4fv(GetLocation(name), 1, (float*)ptr);
				break;
			case GLSL_MAT2:
				glUniformMatrix2fv(GetLocation(name), 1, GL_FALSE, (float*)ptr);
				break;
			case GLSL_MAT3:
				glUniformMatrix3fv(GetLocation(name), 1, GL_FALSE, (float*)ptr);
				break;
			case GLSL_MAT4:
				glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, (float*)ptr);
				break;
			case GLSL_INT:
				glUniform1i(GetLocation(name), *(int*)ptr);
				break;
			case GLSL_VEC2I:
				glUniform2iv(GetLocation(name), 1, (int*)ptr);
				break;
			case GLSL_VEC3I:
				glUniform3iv(GetLocation(name), 1, (int*)ptr);
				break;
			case GLSL_VEC4I:
				glUniform4iv(GetLocation(name), 1, (int*)ptr);
				break;
			case GLSL_STRUCTURE:
				set(name, *(GLSLStructure*)ptr);
				break;
			}
		}
		void set(const std::string& name, GLSLStructure& s)const {
			s.bind(*this, name);
		}
		void set(GLSLStructure& s)const {
			s.bind(*this, "");
		}
		template<uint Target>
		void bindTexture(const std::string& name, int texture_id, const GLTexture<Target>& texture)const {
			if (texture_id < 0 || texture_id>31) {
				texture.bind();
				return;
			}
			glActiveTexture(GL_TEXTURE0 + texture_id);
			texture.bind();
			set(name, texture_id);
		}
		uint ID()const {
			return id;
		}
	private:
		uint id;
	};
	typedef GLSLProgram::GLSLStructure GLSLStructure;
}
