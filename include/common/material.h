#pragma once

#include "frame.h"
#include "transformation.h"
#include "glsl.h"

namespace Festa {
	struct PhongColor {
		vec3 ambient = vec3(1.0f), diffuse = vec3(1.0f), specular = vec3(1.0f);
		PhongColor() {
		}
		PhongColor(const vec3& ambient, const vec3& diffuse, const vec3& specular)
			:ambient(ambient), diffuse(diffuse), specular(specular) {
		}
		void load(const PhongColor& color) {
			ambient = color.ambient; diffuse = color.diffuse; specular = color.specular;
		}
	};
	struct MaterialData :public PhongColor {
		float shininess = -1.0f;
		Path diffuseMap, specularMap, normalMap, heightMap;
		operator bool()const {
			return shininess > 0.0f;
		}
		MaterialData() {

		}
		MaterialData(const vec3& ambient_, const vec3& diffuse_, const vec3& specular_, float _shininess) :shininess(_shininess) {
			ambient = ambient_; diffuse = diffuse_; specular = specular_;
		}
		MaterialData(const PhongColor& color, float _shininess) :shininess(_shininess) {
			load(color);
		}
		MaterialData(const Path& _diffuseMap, const vec3& _specular, float _shininess) :diffuseMap(_diffuseMap), shininess(_shininess) {
			specular = _specular;
		}

		void init() {
			if (diffuseMap.implemented())ambient = diffuse = vec3(-1.0f);
			if (specularMap.implemented())specular = vec3(-1.0f);
		}
	};
	struct Material :public PhongColor, public GLSLStructure {
		float shininess;
		Texture diffuseMap, specularMap, normalMap, heightMap;
		Material() { shininess = -1.0f; }
		Material(const MaterialData& m) {
			init(m);
		}
		void useDiffuseMap() {
			ambient = diffuse = vec3(-1.0f);
		}
		void useSpecularMap() {
			specular = vec3(-1.0f);
		}
		void init(const MaterialData& m) {
			load(m);
			shininess = m.shininess;
			if (m.diffuseMap.implemented())diffuseMap.Generate(m.diffuseMap), useDiffuseMap();
			if (m.specularMap.implemented())specularMap.Generate(m.specularMap), useSpecularMap();
			if (m.normalMap.implemented())normalMap.Generate(m.normalMap);
			if (m.heightMap.implemented())heightMap.Generate(m.heightMap);
		}
		void release() {
			diffuseMap.Release();
			specularMap.Release();
			normalMap.Release();
			heightMap.Release();
		}
		~Material() {
			release();
		}
		void bind(const GLSLProgram& p, const std::string& name)const {
			addAttribute(p, name, "ambient", ambient);
			addAttribute(p, name, "diffuse", diffuse);
			addAttribute(p, name, "specular", specular);
			addAttribute(p, name, "shininess", shininess);
			std::string prefix = name + ".";
			if (diffuseMap)
				p.bindTexture(prefix + "diffuseMap", 1, diffuseMap);
			if (specularMap)
				p.bindTexture(prefix + "specularMap", 2, specularMap);
			if (normalMap)
				p.bindTexture(prefix + "normalMap", 3, normalMap);
			if (heightMap)
				p.bindTexture(prefix + "heightMap", 4, heightMap);
		}
		operator bool()const {
			return shininess > 0.0f;
		}
		void Transfer(Material& material) {
			if (diffuseMap)diffuseMap.Transfer(material.diffuseMap);
			if (specularMap)specularMap.Transfer(material.specularMap);
			if (normalMap)normalMap.Transfer(material.normalMap);
			if (heightMap)heightMap.Transfer(material.heightMap);
			material.shininess = shininess;
			material.ambient = ambient;
			material.diffuse = diffuse;
			material.specular = specular;

		}
	};

	struct Light :public GLSLStructure {
		vec3 pos = vec3(0.0f);
		vec3 color = vec3(0.0f);
		float constant = 0.0f;
		float linear = 0.0f;
		float quadratic = 0.0f;
		bool enabled = true;
		Light() {}
		Light(const vec3& _pos, const vec3& _color, float _constant, float _linear, float _quadratic)
			:pos(_pos), color(_color), constant(_constant), linear(_linear), quadratic(_quadratic) {
		}
		float radius()const {
			const float maxBrightness = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
			return (-linear + std::sqrt(linear * linear - 4.0f * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
		}
		void bind(const GLSLProgram& p, const std::string& name)const {
			if (!enabled)return;
			const std::string prefix = name + ".";
			p.set(prefix + "Position", pos);
			p.set(prefix + "Color", color);
			p.set(prefix + "Constant", constant);
			p.set(prefix + "Linear", linear);
			p.set(prefix + "Quadratic", quadratic);
			p.set(prefix + "Radius", radius());
		}
	};

	struct DirLight :public PhongColor, public GLSLStructure {
		vec3 dir;
		DirLight() { dir = vec3(0.0f); }
		DirLight(const vec3& ambient_, const vec3& diffuse_, const vec3& specular_) {
			ambient = ambient_; diffuse = diffuse_; specular = specular_;
			dir = vec3(0.0f, -1.0f, 0.0f);
		}
		DirLight(const vec3& dir, const vec3& ambient_, const vec3& diffuse_, const vec3& specular_) :dir(dir) {
			ambient = ambient_; diffuse = diffuse_; specular = specular_;
		}
		void rotate(const Rotation& rot) {
			dir = rot * dir;
		}
		void bind(const GLSLProgram& p, const std::string& name)const {
			addAttribute(p, name, "ambient", ambient);
			addAttribute(p, name, "diffuse", diffuse);
			addAttribute(p, name, "specular", specular);
			addAttribute(p, name, "dir", dir);
		}
	};

	struct PointLight :public PhongColor, public GLSLStructure {
		vec3 pos = vec3(0.0f);
		float constant = 0.0f, linear = 0.0f, quadratic = 0.0f;
		PointLight() {}
		PointLight(const vec3& pos, const vec3& ambient_, const vec3& diffuse_, const vec3& specular_, float constant = 0.0f, float linear = 0.0f, float quadratic = 0.0f) :
			pos(pos), constant(constant), linear(linear), quadratic(quadratic) {
			ambient = ambient_; diffuse = diffuse_; specular = specular_;
		}
		void bind(const GLSLProgram& p, const std::string& name)const {
			addAttribute(p, name, "ambient", ambient);
			addAttribute(p, name, "diffuse", diffuse);
			addAttribute(p, name, "specular", specular);
			addAttribute(p, name, "pos", pos);
			addAttribute(p, name, "constant", constant);
			addAttribute(p, name, "linear", linear);
			addAttribute(p, name, "quadratic", quadratic);
		}
	};

	struct SpotLight :public PhongColor , public GLSLStructure {
		vec3 pos = vec3(0.0f), dir = vec3(0.0f);
		float cutOff = 2.0f, outerCutOff = 2.0f;
		SpotLight() {}
		SpotLight(const vec3& pos, const vec3& dir, const vec3& ambient_, const vec3& diffuse_, const vec3& specular_, float delta, float outerDelta = -1.0f) :pos(pos), dir(dir) {
			cutOff = cosf(delta / 2.0f);//delta half the angle
			if (outerDelta > 0.0f)outerCutOff = cosf(outerDelta / 2.0f);
			else outerCutOff = 2.0f;
			ambient = ambient_; diffuse = diffuse_; specular = specular_;
		}
		void rotate(const Rotation& rot) {
			dir = rot * dir;
		}
		void bind(const GLSLProgram& p, const std::string& name)const {
			addAttribute(p, name, "ambient", ambient);
			addAttribute(p, name, "diffuse", diffuse);
			addAttribute(p, name, "specular", specular);
			addAttribute(p, name, "pos", pos);
			addAttribute(p, name, "dir", dir);
			addAttribute(p, name, "cutOff", cutOff);
			addAttribute(p, name, "outerCutOff", outerCutOff);
		}
	};

}