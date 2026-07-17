#pragma once

#include "shapes.h"
#include "window.h"
#include "mesh.h"

namespace Festa {
	
	class FestaModelLoader :public Meshes {
	public:
		FestaModelLoader() {

		}
		FestaModelLoader(const Path& file) {
			load(file);
		}
		void load(const Path& file) {
			File f;
			f.open(file, "rb");
			if (f.check())return;
			uint numMeshes; f >> numMeshes;
			for (uint i = 0; i < numMeshes; i++) {
				meshes.push_back(Meshes::TriMesh());
				readMesh(f, meshes.back());
			}
			if (f.current() < f.size()) {
				boneTree = new BoneTree();
				readBoneDatas(f, boneTree);
				boneTree->tree.resize(boneTree->numBones()+1);
				boneTree->root = readBoneTreeNode(f, boneTree);
				uint numAnim; f >> numAnim; animations.resize(numAnim);
				for (uint i = 0; i < numAnim; i++) {
					animations[i] = new Animation();
					readAnimation(f, animations[i], boneTree->numBones());
					animations[i]->boneTree = boneTree;
				}
			}
			f.close();
		}
	private:
		
		void readMesh(File& f, Meshes::TriMesh& mesh) {
			readString(f, mesh.name);
			// new version
			//std::string format; readString(f, format);
			// old version
			uint num = 0; f >> num;
			std::string format;
			for (uint i = 0; i < num; i++) {
				uchar ch = 0; f >> ch;
				format.push_back(char('0'+ch));
			}
			mesh.attributes() = format;

			uint size; f >> size; 
			for (uint i = 0; i < size; i++)f >> ((float*)mesh.pointer())[i];

			readVector(f, mesh.mIndices);
			readMaterial(f, mesh.material);
		}
		void readMaterial(File& f, MaterialData& mat) {
			PhongColor col; f >> col;
			mat.load(col);
			f >> mat.shininess;
			readString(f, mat.diffuseMap.str());
			readString(f, mat.specularMap.str());
			readString(f, mat.normalMap.str());
			readString(f, mat.heightMap.str());
		}
		void readString(File& f, std::string& str) {
			uint s; f >> s; str.resize(s);
			f.read(str, s);
		}
		template<typename T>
		void readVector(File& f, std::vector<T>& vec) {
			uint s; f >> s; vec.resize(s);
			for (uint i = 0; i < s; i++)f >> vec[i];
		}
		void readBoneDatas(File& f, BoneTree*& tree) {
			uint numBones; f >> numBones;
			tree->boneDatas.resize(numBones);
			for (uint i = 0; i < numBones; i++) {
				readString(f, tree->boneDatas[i].name);
				f >> tree->boneDatas[i].offset;
			}
		}
		uint readBoneTreeNode(File& f, BoneTree*& tree) {
			uint boneID,s; f >> boneID>>s;
			for (uint i = 0; i < s; i++)tree->children(boneID).push_back(readBoneTreeNode(f, tree));
			return boneID;
		}
		void readAnimation(File& f, Animation*& animation,uint numBones) {
			readString(f, animation->name);
			f >> animation->duration;
			f >> animation->ticksPerSecond;
			animation->boneTrans.resize(numBones);
			animation->boneMatrices.resize(numBones);
			for (uint i = 0; i < numBones; i++)readBoneTrans(f, animation->boneTrans[i]);
		}
		void readBoneTrans(File& f, BoneTrans& boneTrans) {
			readString(f, boneTrans.name);
			readVector(f, boneTrans.poskeys);
			readVector(f, boneTrans.rotkeys);
			readVector(f, boneTrans.scakeys);
		}
	};
	
	class FestaModelWriter {
	public:
		FestaModelWriter() {}
		FestaModelWriter(const Path& file) {
			f.open(file,"wb");
			if (f.check())return;
		}
		void write(const Meshes& meshes) {
			f<<(uint)meshes.meshes.size();
			for (const Meshes::TriMesh& mesh:meshes.meshes)
				writeMesh(mesh);
			if (meshes.hasAnimations()) {
				writeBoneDatas(meshes.boneTree);
				writeBoneTreeNode(meshes.boneTree,meshes.boneTree->root);
				f << (uint)meshes.animations.size();
				for (uint i = 0; i < meshes.animations.size(); i++)
					writeAnimation(meshes.animations[i]);
			}
			f.close();
		}
	private:
		File f;
		void writeMesh(const Meshes::TriMesh& mesh) {
			writeString(mesh.name);
			writeString(mesh.attributes().ToString());

			uint size = mesh.numBytes() / sizeof(float); f << size;
			for (uint i = 0; i < size; i++)f << ((float*)mesh.pointer())[i];

			//writeVector(mesh.vertices);
			writeVector(mesh.mIndices);
			writeMaterial(mesh.material);
		}
		void writeMaterial(const MaterialData& material) {
			PhongColor col = material;
			f.writeBinaryData(col);
			f << material.shininess;
			writeString(material.diffuseMap);
			writeString(material.specularMap);
			writeString(material.normalMap);
			writeString(material.heightMap);
		}
		void writeString(const std::string& str) {
			f << (uint)str.size();
			f.write(str);
		}
		template<typename T>
		void writeVector(const std::vector<T>& vec) {
			f << (uint)vec.size();
			for (uint i = 0; i < vec.size(); i++)f.writeBinaryData(vec[i]);
		}
		void writeBoneDatas(BoneTree* tree) {
			f << tree->numBones();
			for (uint boneID = 0; boneID < tree->boneDatas.size(); boneID++) {
				writeString(tree->boneDatas[boneID].name);
				f.writeBinaryData(tree->boneDatas[boneID].offset);
			}
		}
		void writeBoneTreeNode(BoneTree* tree,uint boneID) {
			f << boneID;
			f << (uint)tree->children(boneID).size();
			for (uint ch : tree->children(boneID))writeBoneTreeNode(tree, ch);
		}
		void writeAnimation(Animation* animation) {
			writeString(animation->name);
			f << animation->duration;
			f << animation->ticksPerSecond;
			for (uint i = 0; i < animation->boneTrans.size(); i++)
				writeBoneTrans(animation->boneTrans[i]);
		}
		void writeBoneTrans(const BoneTrans& boneTrans) {
			writeString(boneTrans.name);
			writeVector(boneTrans.poskeys);
			writeVector(boneTrans.rotkeys);
			writeVector(boneTrans.scakeys);
		}
	};
}
