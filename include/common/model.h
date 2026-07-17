#pragma once

#include "common.h"
#include "shapes.h"
#include "window.h"
#include "festamodel.h"

#define MAX_BONE_INFLUENCE 4
#define MAX_BONES 100

namespace Festa {
	//std::unordered_map<std::string, BoneData> bones;
	class ModelLoader :public Meshes {
	public:
		std::vector<MaterialData> materials;
		std::unordered_map<std::string, Image*> textures;
		BoneMap boneMap;
		Path file, directory;

		ModelLoader() {}
		ModelLoader(const Path& path, bool filpuv = true) {
			load(path, filpuv);
		}
		void release() {
			for (auto& i : textures) {
				delete i.second;
			}
		}
		~ModelLoader() {
			release();
		}
		void load(const Path& path, bool filpuv = true) {
			file = path;
			Assimp::Importer importer;
			uint flag = aiProcess_Triangulate | aiProcess_FixInfacingNormals;
			if (filpuv)flag |= aiProcess_FlipUVs;

			//aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
			std::string tmp = path.toString();
			//printf("begin\n");
			const aiScene* scene = importer.ReadFile(tmp, flag);
			//printf("end\n");
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				LOGGER.error("AssimpError: " + std::string(importer.GetErrorString()));
				return;
			}
			directory = tmp.substr(0, tmp.find_last_of('/'));
			materials.resize(scene->mNumMaterials);
			processNode(scene->mRootNode, scene);
			if (scene->mNumAnimations) {
				boneTree = new BoneTree();
				animations.resize(scene->mNumAnimations);
				for (uint i = 0; i < scene->mNumAnimations; i++)
					animations[i] = new Animation(scene, scene->mAnimations[i], boneMap,boneTree);
				boneTree->build(scene, boneMap);
			}
		}
	private:
		void processNode(aiNode* node, const aiScene* scene) {
			std::string name = node->mName.C_Str();
			//std::cout << "node " << name << " " << node->mNumMeshes << std::endl;
			for (uint i = 0; i < node->mNumMeshes; i++)
				processMesh(scene->mMeshes[node->mMeshes[i]], scene, name + "_" + toString(i), i);
			for (uint i = 0; i < node->mNumChildren; i++)
				processNode(node->mChildren[i], scene);
		}
		void processMesh(aiMesh* mesh, const aiScene* scene, const std::string& name, uint meshid) {
			meshes.push_back(TriMesh());
			TriMesh& m = meshes.back();
			VertexAttributes attributes(std::vector<VertexAttributes::Attribute>{ { 3, GL_FLOAT, VertexAttributes::POSITION } });
			uint stride = 3;
			uint numBones = mesh->mNumBones;
			if (mesh->HasNormals()) {
				attributes.Add({ 3,GL_FLOAT, VertexAttributes::NORMAL });
				stride += 3;
			}
			if (mesh->mTextureCoords[0]) {
				attributes.Add({ 2, GL_FLOAT, VertexAttributes::TEXTURE_COORDS });
				stride += 2;
			}
			if (numBones) {
				attributes.Add({ 4, GL_FLOAT, VertexAttributes::BONE_IDS });
				attributes.Add({ 4, GL_FLOAT, VertexAttributes::BONE_WEIGHTS });
				stride += 8;
			}
			std::vector<float> vertices(stride * mesh->mNumVertices, 0.0f);
			uint idx = 0;
			for (uint i = 0; i < mesh->mNumVertices; i++) {
				vertices[idx++] = mesh->mVertices[i].x;
				vertices[idx++] = mesh->mVertices[i].y;
				vertices[idx++] = mesh->mVertices[i].z;
				if (mesh->HasNormals()) {
					vertices[idx++] = mesh->mNormals[i].x;
					vertices[idx++] = mesh->mNormals[i].y;
					vertices[idx++] = mesh->mNormals[i].z;
				}
				if (mesh->mTextureCoords[0]) {
					vertices[idx++] = mesh->mTextureCoords[0][i].x;
					vertices[idx++] = mesh->mTextureCoords[0][i].y;
				}
				if (numBones) {
					for (uint j = 0; j < 4; j++)vertices[idx++] = -1.0f;
					for (uint j = 0; j < 4; j++)vertices[idx++] = 0.0f;
				}
			}
			if (numBones)
				for (uint i = 0; i < numBones; i++)processBone(mesh->mBones[i], stride, &vertices[0]);

			m.Init(vertices, attributes);

			uint numIndices = 0;
			for (uint i = 0; i < mesh->mNumFaces; i++)
				numIndices += mesh->mFaces[i].mNumIndices;
			m.mIndices.resize(numIndices); idx = 0;
			for (uint i = 0; i < mesh->mNumFaces; i++) {
				aiFace& face = mesh->mFaces[i];
				for (uint j = 0; j < face.mNumIndices; j++)
					m.mIndices[idx++] = face.mIndices[j];
			}
			uint boneid = boneMap.findID(name);
			m.material = getMaterial(mesh, scene);
			m.boneid = boneid;
			m.name = name;
		}
		void getTexture(aiMaterial* material, aiTextureType type, Path& file) {
			aiString str; material->GetTexture(type, 0, &str);
			file = directory + Path(str.C_Str());
		}
		MaterialData& getMaterial(aiMesh* mesh, const aiScene* scene) {
			MaterialData& mat = materials[mesh->mMaterialIndex];
			if (mat)return mat;
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiColor3D color;
			if (material->GetTextureCount(aiTextureType_DIFFUSE))
				getTexture(material, aiTextureType_DIFFUSE, mat.diffuseMap);
			else if (material->GetTextureCount(aiTextureType_HEIGHT))
				//cout << "height\n";
				getTexture(material, aiTextureType_HEIGHT, mat.diffuseMap);
			else {
				material->Get(AI_MATKEY_COLOR_AMBIENT, color);
				mat.ambient = vec3(color.r, color.g, color.b);
				material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				mat.diffuse = vec3(color.r, color.g, color.b);
			}

			if (material->GetTextureCount(aiTextureType_SPECULAR))
				getTexture(material, aiTextureType_SPECULAR, mat.specularMap);
			else {
				material->Get(AI_MATKEY_COLOR_SPECULAR, color);
				mat.specular = vec3(color.r, color.g, color.b);
			}
			material->Get(AI_MATKEY_SHININESS, mat.shininess);
			if (mat.shininess < EPS_FLOAT)mat.shininess = 1.0f;
			return mat;
		}
		void processBone(aiBone* bone, uint stride, float* vertices) {
			uint boneID = 0;
			std::string name = bone->mName.C_Str();
			if (!boneMap.find(name))
				boneID = boneMap.addBone(BoneData{ name,convertMatrix(bone->mOffsetMatrix) });
			else boneID = boneMap.findID(name);
			for (uint i = 0; i < bone->mNumWeights; i++)
				setBoneData(vertices, stride, bone->mWeights[i].mVertexId, boneID, bone->mWeights[i].mWeight);
		}
		void setBoneData(float* vertices, uint stride, uint vid, uint boneid, float weight)const {
			for (int i = 0; i < 4; i++) {
				uint idx = vid * stride + stride - 8 + i;
				if (vertices[idx] < 0.0f) {
					vertices[idx] = float(boneid);
					vertices[idx + 4] = weight;
					return;
				}
			}
		}
	};

	class Model {
	public:
		struct TriMesh {
			VAO vao;
			Material material;
			AABB aabb; uint boneid = 0;
			TriMesh() {}
			void init(const Mesh& mesh, const MaterialData& m) {
				vao.LoadMesh(mesh);
				material.init(m);
			}
			void release() {
				vao.Release();
				material.release();
			}
			~TriMesh() {
				release();
			}
		};
		int currentAnimation = -1;
		std::vector<TriMesh> meshes;
		std::vector<amr_ptr<Animation>> animations;
		AABB aabb;
		Model() {}
		Model(const Meshes& m) {
			load(m);
		}
		Model(const std::string& path) {
			load(path);
		}
		void release() {
			//std::cout << "release model\n";
			for (TriMesh& mesh : meshes)mesh.release();
			meshes.clear();
			currentAnimation = -1;
			if (!animations.size() || !animations[0])return;
			//BoneTree* tree = animations[0]->tree;
			//SafeDelete(tree);
			for (auto& anim : animations)anim.release();
			animations.clear();
		}
		~Model() {
			release();
		}
		void setAnimation(uint idx) {
			if (idx >= animations.size())LOGGER.error("Invaild animation index");
			currentAnimation = idx;
		}
		void setMatrices()const {
			GLSLProgram* program = GLSLProgram::activatedProgram;
			if (!program)return;
			program->set("model", trans.toMatrix());
			if (currentAnimation != -1)
				animations[currentAnimation]->setMatrices();
		}
		void renderMesh(uint i) {
			if (!GLSLProgram::activatedProgram)return;
			GLSLProgram::activatedProgram->set("material", meshes[i].material);
			meshes[i].vao.Draw();
		}
		void renderInstancedMesh(uint i, uint count) {
			if (!GLSLProgram::activatedProgram)return;
			GLSLProgram::activatedProgram->set("material", meshes[i].material);
			meshes[i].vao.DrawInstanced(count);
		}
		void updateAnimation(float speed = 1.0f) {
			if (!hasAnimations() || currentAnimation==-1)return;
			animations[currentAnimation]->update(speed);
		}
		void render() {
			setMatrices();
			for (uint i = 0; i < meshes.size(); i++)renderMesh(i);
		}
		void renderInstancedMeshes(uint count) {
			setMatrices();
			for (uint i = 0; i < meshes.size(); i++)renderInstancedMesh(i, count);
		}
		Animation* animation()const {
			if (!hasAnimations() || currentAnimation == -1)return 0;
			else return animations[currentAnimation].ptr;
		}
		bool hasAnimations()const {
			return animations.size();
		}
		/*void instanceBuffer(GLBuffer buffer, const std::string& format) {
			for (int i = 0; i < meshes.size(); i++)meshes[i].vao.instanceBuffer(buffer, format);
		}*/
		AABB getAABB() {
			if (hasAnimations()) {
				aabb.clear();
				Animation& anim = *animation();
				for (TriMesh& mesh : meshes) {
					aabb.update(mesh.aabb * anim.boneMatrices[mesh.boneid]);
				}
			}
			return aabb * trans.toMatrix();
		}
		void load(const Meshes& m) {
			uint size = uint(m.meshes.size());
			meshes.resize(size);
			size_t i = 0;
			for (const Meshes::TriMesh& mesh:m.meshes) {
				meshes[i].init(mesh, mesh.material);
				meshes[i].aabb.update(mesh);
				aabb.update(meshes[i].aabb);
				meshes[i].boneid = mesh.boneid;
				i++;
			}
			animations.resize(m.animations.size());
			for (uint i = 0; i < m.animations.size(); i++)
				animations[i].ptr = m.animations[i];
			if (animations.size()) currentAnimation = 0;
		}
		void load(const Path& path) {
			const std::string extension = path.extension();
			if (extension == "fm")load(FestaModelLoader(path));
			else load(ModelLoader(path));
		}
		void setTransformation(const Transformation& t) {
			trans.loadFrom(t);
		}
		Transformation& getTransformation() {
			return trans;
		}
		void clearTransformation() {
			trans.clear();
		}
		void translate(const vec3& t) {
			trans.translate(t);
		}
		void _translate(const vec3& t) {
			trans._translate(t);
		}
		void setPosition(const vec3& pos) {
			trans.setPosition(pos);
		}
		const vec3& position()const {
			return trans.getPosition();
		}
		vec3& position() {
			return trans.getPosition();
		}
		void setRotation(const Rotation& rot) {
			trans.setRotation(rot);
		}
		void _rotate(const Rotation& rot) {
			trans._rotate(rot);
		}
		void rotate(const Rotation& rot) {
			trans.rotate(rot);
		}
		void setScale(const vec3& scale) {
			trans.setScale(scale);
		}
		void _scale(const vec3& scaling) {
			trans._scale(scaling);
		}
		void scale(const vec3& scaling) {
			trans.scale(scaling);
		}
		void _transform(const Transformation& t) {
			trans._transform(t);
		}
		void transform(const Transformation& t) {
			trans.transform(t);
		}
	protected:
		Transformation trans;
	};



}

