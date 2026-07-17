#pragma once

#include "../3rd/assimp/scene.h"
#include "../3rd/assimp/Importer.hpp"
#include "../3rd/assimp/postprocess.h"

#include "glsl.h"
#include "material.h"

namespace Festa {
	struct Vertex {
		vec3 position, normal;
		vec2 texCoords;
		Vertex() {
			position = vec3(0.0f); normal = vec3(0.0f); texCoords = vec2(0.0f);
		}
		Vertex(vec3 _position, vec3 _normal, vec2 _texCoords) :position(_position), normal(_normal), texCoords(_texCoords) {}
		Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
			position = vec3(x, y, z);
			normal = vec3(nx, ny, nz);
			texCoords = vec2(u, v);
		}
		Vertex(float* v) {
			position = vec3(*v, *(v + 1), *(v + 2));
			normal = vec3(*(v + 3), *(v + 4), *(v + 5));
			texCoords = vec2(*(v + 6), *(v + 7));
		}
	};

	class VertexAttributes {
	public:
		enum AttributeName: uchar {
			POSITION,
			NORMAL,
			TEXTURE_COORDS,
			BONE_IDS,
			BONE_WEIGHTS,
			MYATTR_X,
			MYATTR_Y,
			MYATTR_Z,
			NUM_NAMES
		};
		struct Attribute {
			uint length = 0;
			uint type = 0;
			AttributeName name = NUM_NAMES;
			Attribute() {}
			Attribute(uint _length, uint _type) :length(_length), type(_type) {}
			Attribute(uint _length, uint _type, AttributeName _name) :length(_length), type(_type), name(_name) {}
			uint numBytes()const {
				switch (type) {
				case GL_FLOAT:
					return 4u * length;
				case GL_INT:
					return 4u * length;
				}
				return 0u;
			}
			bool operator==(const Attribute& a)const {
				return a.length == length && a.type == type && a.name == name;
			}
		};
		VertexAttributes() {
			mOffsets.resize(NUM_NAMES, -1);
		}
		VertexAttributes(const std::string& s) {
			mOffsets.resize(NUM_NAMES, -1);
			FromString(s);
		}
		VertexAttributes(const char* s) {
			mOffsets.resize(NUM_NAMES, -1);
			FromString(s);
		}
		VertexAttributes(const std::vector<Attribute>& attributes) :mAttributes(attributes) {
			mOffsets.resize(NUM_NAMES, -1);
			for (const Attribute& attr : mAttributes)mBytes += attr.numBytes();
			CalcOffsets();
		}
		template<typename T>
		void CountBytes() {
			mBytes = sizeof(T);
		}
		uint FromString(const std::string& s) {
			uint count = 0;
			for (char c : s)if ('0' <= c && c <= '9')count++;
			mAttributes.resize(count);
			uint idx = 0;
			for (uint i = 0; i < s.size(); i++) {
				mAttributes[idx] = { uint(s[i] - '0'), GL_FLOAT };
				mBytes += mAttributes[idx].numBytes();
				if (i + 1 < s.size() && !('0' <= s[i+1] && s[i+1] <= '9')) {
					switch (s[++i]) {
					case 'p':
						mAttributes[idx].name = POSITION;
						break;
					case 'n':
						mAttributes[idx].name = NORMAL;
						break;
					case 't':
						mAttributes[idx].name = TEXTURE_COORDS;
						break;
					case 'b':
						mAttributes[idx].name = BONE_IDS;
						break;
					case 'w':
						mAttributes[idx].name = BONE_WEIGHTS;
						break;
					case 'x':
						mAttributes[idx].name = MYATTR_X;
						break;
					case 'y':
						mAttributes[idx].name = MYATTR_Y;
						break;
					case 'z':
						mAttributes[idx].name = MYATTR_Z;
						break;
					}
				}
				idx++;
			}
			//std::cout << "w " << s << " " << ToString() << '\n';
			CalcOffsets();
			return count;
		}
		std::string ToString()const {
			std::string ret;
			for (const Attribute& attr : mAttributes) {
				ret.push_back('0' + attr.length);
				switch (attr.name) {
				case POSITION:
					ret.push_back('p'); break;
				case NORMAL:
					ret.push_back('n'); break;
				case TEXTURE_COORDS:
					ret.push_back('t'); break;
				case BONE_IDS:
					ret.push_back('b'); break;
				case BONE_WEIGHTS:
					ret.push_back('w'); break;
				case MYATTR_X:
					ret.push_back('x'); break;
				case MYATTR_Y:
					ret.push_back('y'); break;
				case MYATTR_Z:
					ret.push_back('z'); break;
				}
			}
			return ret;
		}
		void Clear() {
			mBytes = 0;
			mAttributes.clear();
			for (auto& i : mOffsets)i = NUM_NAMES;
		}
		int numAttributes()const {
			return int(mAttributes.size());
		}
		size_t numBytes()const {
			return mBytes;
		}
		const Attribute& operator[](size_t i)const {
			return mAttributes[i];
		}
		Attribute& operator[](size_t i) {
			return mAttributes[i];
		}
		
		int GetOffset(AttributeName name)const {
			return name < NUM_NAMES ? mOffsets[name] : -1;
		}
		void Add(const Attribute& attr) {
			mAttributes.emplace_back(attr);
			if (attr.name != NUM_NAMES)mOffsets[attr.name] = (int)mBytes;
			mBytes += mAttributes.back().numBytes();
		}
		void Enable()const {
			size_t _numBytes = numBytes();
			size_t addr = 0;
			for (int i = 0; i < numAttributes();i++) {
				glVertexAttribPointer(i, (int)mAttributes[i].length, mAttributes[i].type, GL_FALSE, (int)_numBytes, (void*)addr);
				glEnableVertexAttribArray(i);
				addr += mAttributes[i].numBytes();
			}
		}
		void Disable()const {
			for (int i = 0; i < numAttributes(); i++)
				glDisableVertexAttribArray(i);
		}
		void operator+=(const VertexAttributes& attr) {
			mAttributes.insert(mAttributes.end(), attr.mAttributes.begin(), attr.mAttributes.end());
			mBytes += attr.numBytes();
		}
		VertexAttributes operator+(const VertexAttributes& attr) {
			VertexAttributes ret = *this;
			ret += attr;
			return ret;
		}
		bool operator==(const VertexAttributes& va)const {
			if (numAttributes() != va.numAttributes() || numBytes() != va.numBytes())return false;
			for (int i = 0; i < numAttributes(); i++) {
				if (!(mAttributes[i] == va.mAttributes[i]))return false;
			}
			return true;
		}
		bool operator!=(const VertexAttributes& va)const {
			return !(*this == va);
		}
		void Write(File& f)const {
			f << int(mAttributes.size());
			for (uint i = 0; i < uint(numAttributes()); i++) {
				f << mAttributes[i].length << mAttributes[i].type << mAttributes[i].name;
			}
		}
		void Read(File& f) {
			int num = 0; f >> num;
			Clear();
			mAttributes.resize(num);
			mBytes = 0;
			for (int i = 0; i < num; i++) {
				f >> mAttributes[i].length >> mAttributes[i].type >> mAttributes[i].name;
				mBytes += mAttributes[i].numBytes();
			}
			CalcOffsets();
		}
	private:
		std::vector<Attribute> mAttributes;
		size_t mBytes = 0;
		std::vector<int> mOffsets;

		void CalcOffsets() {
			size_t addr = 0;
			for (const Attribute& at : mAttributes) {
				if (at.name != NUM_NAMES)mOffsets[at.name] = (int)addr;
				addr += at.numBytes();
			}
		}
	};

	struct Triangle {
		vec3 tri[3] = { vec3(0.0f),vec3(0.0f),vec3(0.0f) };
		Triangle() {

		}
		Triangle(const vec3& a, const vec3& b, const vec3& c) {
			tri[0] = a; tri[1] = b; tri[2] = c;
		}
		vec3 normal()const {
			vec3 ac = tri[2] - tri[0], ab = tri[1] - tri[0];
			//**
			return glm::normalize(cross(ac, ab));
		}
		const void* getptr()const {
			return &tri[0];
		}
		vec3& operator[](uint i) {
			return tri[i];
		}
		vec3 operator[](uint i)const {
			return tri[i];
		}
		Triangle operator*(const mat4& mat)const {
			return Triangle(
				mat * vec4(tri[0], 1.0f), 
				mat * vec4(tri[1], 1.0f),
				mat * vec4(tri[2], 1.0f)
			);
		}
		void operator*=(const mat4& mat) {
			tri[0] = mat * vec4(tri[0], 1.0f);
			tri[1] = mat * vec4(tri[1], 1.0f);
			tri[2] = mat * vec4(tri[2], 1.0f);
		}
	};

	class VertexArray {
	public:
		typedef VertexAttributes::AttributeName AttributeName;
		VertexArray() {}
		template<typename T>
		VertexArray(const std::vector<T>& vertices, const VertexAttributes& attributes) {
			Init(vertices, attributes);
		}
		~VertexArray() {
			Release();
		}
		void Release() {
			if(mData)delete[] mData;
			mData = 0;
			mSize = 0;
		}
		template<typename T>
		void Init(const std::vector<T>& vertices, const VertexAttributes& attributes) {
			LoadVertices(vertices);
			mAttributes = attributes;
		}
		void Init(size_t size, const VertexAttributes& attributes) {
			Release();
			mSize = size;
			mData = new uchar[mSize];
			memset(mData, 0, mSize);
			mAttributes = attributes;
		}
		template<typename T>
		void LoadVertices(const std::vector<T>& vertices) {
			Release();
			mSize = sizeof(T) * vertices.size();
			mData = new uchar[mSize];
			memcpy(mData, &vertices[0], mSize);
		}
		operator bool()const {
			return mData;
		}
		void* pointer()const {
			return mData;
		}
		size_t numBytes()const {
			return mSize;
		}
		uint numVertices()const {
			uint bytes = uint(mAttributes.numBytes());
			return bytes ? mSize / bytes : 0;
		}
		void Resize(size_t size) {
			if (size > mSize) {
				Release();
				mSize = size;
				mData = new uchar[mSize];
				memset(mData, 0, mSize);
				return;
			}
			mSize = size;
		}
		const VertexAttributes& attributes()const {
			return mAttributes;
		}
		VertexAttributes& attributes() {
			return mAttributes;
		}
		template<typename T>
		const T& Index(size_t i, size_t off = 0)const {
			T* ptr = (T*)(mData + i * mAttributes.numBytes() + off);
			return *ptr;
		}
		template<typename T>
		T& Index(size_t i, size_t off = 0) {
			T* ptr = (T*)(mData + i * mAttributes.numBytes() + off);
			return *ptr;
		}
		template<typename T>
		const T& GetAttribute(size_t i, AttributeName name)const {
			int off = mAttributes.GetOffset(name);
			assert(off>=0);
			T* ptr = (T*)(mData + i * mAttributes.numBytes() + off);
			return *ptr;
		}
		template<typename T>
		T& GetAttribute(size_t i, AttributeName name) {
			int off = mAttributes.GetOffset(name);
			assert(off >= 0);
			T* ptr = (T*)(mData + i * mAttributes.numBytes() + off);
			return *ptr;
		}
		int FindAttribute(AttributeName name)const {
			return mAttributes.GetOffset(name);
		}
		void Copy(const VertexArray& va) {
			Init(va.mSize, va.mAttributes);
			memcpy(mData, va.mData, va.mSize);
		}
		void operator=(const VertexArray& va) {
			Copy(va);
		}
		VertexArray operator+(const VertexArray& va)const{
			assert(va.mAttributes == mAttributes);
			VertexArray ret;ret.Init(va.mSize + mSize, mAttributes);
			memcpy(ret.mData, mData, mSize);
			memcpy(ret.mData+mSize, va.mData, va.mSize);
			return ret;
		}

		int CheckVertexID(uint i)const {
			if (i > numVertices()) {
				LOGGER.error("Invalid vertex index: " + toString(i));
				//assert(false, "Invalid vertex index: " + toString(i));
				return 1;
			}
			return 0;
		}
		const vec3& position(uint i)const {
			CheckVertexID(i);
			return GetAttribute<vec3>(i, VertexAttributes::POSITION);
		}
		vec3& position(uint i) {
			CheckVertexID(i);
			return GetAttribute<vec3>(i, VertexAttributes::POSITION);
		}
		const vec3& normal(uint i)const {
			CheckVertexID(i);
			return GetAttribute<vec3>(i, VertexAttributes::NORMAL);
		}
		vec3& normal(uint i) {
			CheckVertexID(i);
			return GetAttribute<vec3>(i, VertexAttributes::NORMAL);
		}
		const vec2& textureCoords(uint i)const {
			CheckVertexID(i);
			return GetAttribute<vec2>(i, VertexAttributes::TEXTURE_COORDS);
		}
		vec2& textureCoords(uint i) {
			CheckVertexID(i);
			return GetAttribute<vec2>(i, VertexAttributes::TEXTURE_COORDS);
		}
		const vec4& boneIDs(uint i)const {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::BONE_IDS);
		}
		vec4& boneIDs(uint i) {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::BONE_IDS);
		}
		const vec4& boneWeights(uint i)const {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::BONE_WEIGHTS);
		}
		vec4& boneWeights(uint i) {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::BONE_WEIGHTS);
		}
		template<typename T>
		const T& myAttrX(size_t i)const {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::MYATTR_X);
		}
		template<typename T>
		T& myAttrX(size_t i) {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::MYATTR_X);
		}
		template<typename T>
		const T& myAttrY(size_t i)const {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::MYATTR_Y);
		}
		template<typename T>
		T& myAttrY(size_t i) {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::MYATTR_Y);
		}
		template<typename T>
		const T& myAttrZ(size_t i)const {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::MYATTR_Z);
		}
		template<typename T>
		T& myAttrZ(size_t i) {
			CheckVertexID(i);
			return GetAttribute<vec4>(i, VertexAttributes::MYATTR_Z);
		}
	protected:
		uchar* mData = 0;
		size_t mSize = 0;
		VertexAttributes mAttributes;
	};

	struct Mesh:public VertexArray {
		struct Face {
			uint v[3] = { 0,0,0 };
			Face() {}
			Face(uint a, uint b, uint c) {
				v[0] = a, v[1] = b, v[2] = c;
			}
			uint operator[](uint i)const {
				return v[i];
			}
			uint& operator[](uint i) {
				return v[i];
			}
			void sort() {
				std::sort(&v[0], &v[0] + 3);
			}
			bool operator==(const Face& face)const {
				return v[0] == face[0] && v[1] == face[1] && v[2] == face[2];
			}
		};
		std::vector<uint> mIndices;
		Mesh() {}
		template<typename T>
		Mesh(const std::vector<T>& vertices, const VertexAttributes& attributes) {
			Init(vertices, attributes);
			//std::cout << "mesh\n";
		}
		template<typename T>
		Mesh(const std::vector<T>& vertices, const std::vector<uint>& _indices, const VertexAttributes& attributes) :mIndices(_indices) {
			Init(vertices, attributes);
		}
		Mesh(std::vector<Vertex>& v) {
			Init(v, "3p3n2t");
		}
		Mesh(std::vector<Vertex>& v, std::vector<uint>& indices) :mIndices(indices) {
			Init(v, "3p3n2t");
		}
		void Clear() {
			Release();
			mAttributes.Clear();
			mIndices.clear();
		}
		void Circle(uint n = 25, const std::string& attributes = "3p3n2t");
		void Cone(uint n = 25, const std::string& attributes = "3p3n2t");
		void Cylinder(uint n = 25, bool discs = true, const std::string& attributes = "3p3n2t");
		void HalfSphere(uint longitude = 30, uint latitude = 14, const std::string& attributes = "3p3n2t");
		void Sphere(uint longitude=30, uint latitude=14, const std::string& attributes = "3p3n2t");
		uint numFaces()const {
			return hasIndices() ? numIndices() / 3 : numVertices() / 3;
		}
		uint numIndices()const {
			return (uint)mIndices.size();
		}
		bool hasIndices()const {
			return numIndices();
		}
		const uint* indices()const {
			return &mIndices[0];
		}
		uint* indices() {
			return &mIndices[0];
		}
		int checkFaceID(uint i)const {
			if (i > numFaces()) {
				LOGGER.error("Invalid face index: " + toString(i));
				return 1;
			}
			return 0;
		}
		
		Face face(uint i)const {
			if (checkFaceID(i))return {0, 0, 0};
			if (hasIndices())return { mIndices[i * 3], mIndices[i * 3 + 1], mIndices[i * 3 + 2] };
			return { i * 3,i * 3 + 1,i * 3 + 2 };
		}
		
		void transform(const Transformation& trans) {
			for (uint i = 0; i < numVertices(); i++) {
				position(i) = trans.apply(position(i));
				normal(i) = glm::normalize(trans.applyOrientation(normal(i)));
			}
		}
		Triangle triangle(uint i)const {
			if (checkFaceID(i))return Triangle();
			Face f = face(i);
			return Triangle(position(f[0]), position(f[1]), position(f[2]));
		}
		void operator+=(const Mesh& mesh) {
			assert(hasIndices() == mesh.hasIndices(), "Different indices requirements of 2 meshes combining together");
			Copy(*this + mesh);
			mIndices.insert(mIndices.begin(), mesh.mIndices.begin(), mesh.mIndices.end());
		}
		/*
		Mesh operator+(const Mesh& mesh)const {
			Mesh res = Mesh(*this);
			res += mesh;
			return res;
		}
		VirtualValue<Triangle> triangle(uint i) {
			if (checkFaceID(i))return VirtualValue<Triangle>();
			Face f = face(i);
			vec3* a = &position(f[0]);
			vec3* b = &position(f[1]);
			vec3* c = &position(f[2]);
			return VirtualValue<Triangle>(
				[a, b, c](const Triangle& tri) {*a = tri[0], * b = tri[1], * c = tri[2]; },
				[a, b, c] {return Triangle(*a, *b, *c); });
		}
		void removeIndices() {
			if (!hasIndices())return;
			std::vector<float> newVertices(indices.size() * format.stride);
			uint idx = 0;
			for (uint i = 0; i < indices.size(); i++) {
				for (uint f = 0; f < format.stride; f++)
					newVertices[idx++] = vertices[indices[i] * format.stride + f];
			}
			newVertices.resize(idx);
			vertices = newVertices;
			indices.clear();
		}
		void deleteVertex(uint vid) {
			if (checkVertexID(vid))return;
			if (hasIndices()) {
				vertices.erase(vertices.begin() + vid * format.stride,
					vertices.begin() + (vid + 1) * format.stride);
				std::vector<uint> newIndices; newIndices.resize(indices.size());
				uint idx = 0;
				for (uint fi = 0; fi < facesCount(); fi++) {
					Mesh::Face f = face(fi);
					if (!(f[0] == vid || f[1] == vid || f[2] == vid)) {
						if (f[0] > vid)f[0] -= 1;
						if (f[1] > vid)f[1] -= 1;
						if (f[2] > vid)f[2] -= 1;
						newIndices[idx++] = f[0]; newIndices[idx++] = f[1]; newIndices[idx++] = f[2];
					}
				}
				newIndices.resize(idx);
				indices = newIndices;
			}
			else {
				uint st = vid / 3 * 3;
				vertices.erase(vertices.begin() + st * format.stride,
					vertices.begin() + (st + 3) * format.stride);
			}
		}
		void deleteFace(uint fid) {
			if (checkFaceID(fid))return;
			if (hasIndices()) {
				indices.erase(indices.begin()+fid*3,indices.begin()+fid*3+3);
			}
			else {
				uint st = fid*3;
				vertices.erase(vertices.begin() + st * format.stride,
					vertices.begin() + (st + 3) * format.stride);
			}
		}
		int findFace(const Face& f)const {
			if (checkVertexID(f[0]) || checkVertexID(f[1]) || checkFaceID(f[2]))return -1;
			Face t = f; t.sort();
			for (uint i = 0; i < facesCount(); i++) {
				Face x = face(i); x.sort();
				if (x == t)return i;
			}
			return -1;
		}*/
		void GenerateIndices() {
			if (hasIndices())return;
			mIndices.resize(numVertices());
			for (uint i = 0; i < mIndices.size(); i++)mIndices[i] = i;
		}
		void RegenerateNormal(bool flipNormal = false) {
			//if (checkHasNormal())return;
			if (hasIndices()) {
				for (uint i = 0; i < numVertices(); i++)normal(i) = vec3(0.0f);
				for (uint i = 0; i < numFaces(); i++) {
					Face f = face(i);
					Triangle tri = triangle(i);
					vec3 n = flipNormal ? -tri.normal() : tri.normal();
					normal(f[0]) += n;
					normal(f[1]) += n;
					normal(f[2]) += n;
				}
				for (uint i = 0; i < numVertices(); i++)normal(i) = glm::normalize(normal(i));
			}
			else {
				for (uint i = 0; i < numFaces(); i++) {
					uint v = i * 3;
					Triangle tri = triangle(i);
					vec3 n = flipNormal ? -tri.normal() : tri.normal();
					normal(v) = n;
					normal(v + 1) = n;
					normal(v + 2) = n;
				}
			}
		}
		void RemoveDuplicatedVertices(bool flipNormal = false);
		void findDuplicatedVertices(uint vid,std::list<uint>& dup)const;
		void Save(const Path& file) {
			File f(file, "wb"); if (f.check())return;
			f << int(numVertices()) << int(numIndices());
			mAttributes.Write(f);
			f.writeBinaryData(mData, mSize);
			for (uint i : mIndices)f << i;
			f.close();
		}
		void Load(const Path& file) {
			File f(file, "rb"); if (f.check())return;
			Clear();
			int nV = 0;int nI = 0;f >> nV >> nI;
			mAttributes.Read(f);
			Resize(mAttributes.numBytes()*size_t(nV));
			f.readBinaryData(mData, mSize);
			mIndices.resize(nI);
			for (uint& i : mIndices)f >> i;
			f.close();
		}
	protected:
		//std::vector<uint> mIndices;
	};

	class VAO {
	public:
		VAO() {}
		VAO(const Mesh& mesh) {
			LoadMesh(mesh);
		}
		uint ID()const {
			return id;
		}
		template<typename T>
		void Init(const std::vector<T>& vertices, const VertexAttributes& attributes, uint usage = GL_STATIC_DRAW) {
			Release();
			glGenVertexArrays(1, &id);
			Bind();
			mAttributes = attributes;
			numVertices = vertices.size();
			vbo.generate(vertices.size()*sizeof(T), &vertices[0], usage);
			vbo.bind();
			mAttributes.Enable();
			vbo.unbind();
		}
		void LoadMesh(const Mesh& mesh, uint usage = GL_STATIC_DRAW) {
			Release();
			glGenVertexArrays(1, &id);
			Bind();
			mAttributes = mesh.attributes();
			numVertices = mesh.numVertices();
			vbo.generate(mesh.numBytes(), mesh.pointer(), usage);
			numIndices = mesh.numIndices();
			if(numIndices)ebo.generate(numIndices * sizeof(uint), mesh.indices(), usage);
			vbo.bind();
			mAttributes.Enable();
			vbo.unbind();
		}
		void Release() {
			if (id)glDeleteVertexArrays(1, &id);
			if (vbo)vbo.release();
			if (ebo)ebo.release();
			id = 0;
			numVertices = 0;
			numIndices = 0;
			mAttributes.Clear();
		}
		~VAO() {
			Release();
		}
		void Transfer(VAO& vao) {
			vao.id = id;
			vbo.Transfer(vao.vbo);
			ebo.Transfer(vao.ebo);
			vao.numIndices = numIndices;
			vao.numVertices = numVertices;
			vao.mAttributes = mAttributes;
			id = 0;
			numIndices = 0;
			numVertices = 0;
			mAttributes.Clear();
		}
		static void VertexAttrib(uint index, int size, int stride, uint addr, uint type = GL_FLOAT) {
			glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)ull(addr));
			glEnableVertexAttribArray(index);
		}
		void Bind()const {
			glBindVertexArray(id);
		}
		static void Unbind() {
			glBindVertexArray(0);
		}
		void BindInstanceBuffer(GLArrayBuffer buffer, const VertexAttributes& attributes, uint divisor = 1) {
			Bind();
			buffer.bind();

			size_t _numBytes = attributes.numBytes();
			size_t addr = 0;

			uint numAttrib = mAttributes.numAttributes();
			for (uint i = 0; i < uint(attributes.numAttributes()); i++) {
				glVertexAttribPointer(numAttrib + i, (int)attributes[i].length, attributes[i].type, GL_FALSE, (int)_numBytes, (void*)addr);
				glEnableVertexAttribArray(numAttrib + i);
				glVertexAttribDivisor(numAttrib + i, divisor);
				addr += attributes[i].numBytes();
			}
			mAttributes += attributes;
		}
		void Draw(uint mode = GL_TRIANGLES)const {
			Bind();
			vbo.bind();
			mAttributes.Enable();
			if (hasIndices())glDrawElements(mode, numIndices, GL_UNSIGNED_INT, 0);
			else glDrawArrays(mode, 0, numVertices);
		}
		void DrawInstanced(uint count, uint mode = GL_TRIANGLES)const {
			Bind();
			if (hasIndices())glDrawElementsInstanced(mode, numIndices, GL_UNSIGNED_INT, 0, count);
			else glDrawArraysInstanced(mode, 0, numVertices, count);
		}
		bool hasIndices()const {
			return numIndices;
		}
		operator bool()const {
			return id;
		}
	private:
		GLArrayBuffer vbo;
		GLBuffer<GL_ELEMENT_ARRAY_BUFFER> ebo;
		uint id = 0;
		uint numVertices = 0;
		uint numIndices = 0;
		VertexAttributes mAttributes;
	};

	inline mat4 convertMatrix(const aiMatrix4x4& from){
		mat4 to;
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	class BoneTrans {
	public:
		struct KeyPosition {
			vec3 pos = vec3(0.0f);
			double time = 0.0;
		};
		struct KeyRotation {
			Rotation rot;
			double time = 0.0;
		};
		struct KeyScaling {
			Scaling sca = vec3(0.0f);
			double time = 0.0;
		};
		struct Transform {
			vec3 pos;
			Rotation rot;
			vec3 scaling;
			mat4 toMatrix()const {
				return translate4(pos) * rot.ToMat4() * scale4(scaling);
			}
		};
		std::vector<KeyPosition> poskeys;
		std::vector<KeyRotation> rotkeys;
		std::vector<KeyScaling> scakeys;
		std::string name;
		Transform trans;
		BoneTrans() {
			reset();
		}
		BoneTrans(const aiNodeAnim* anim, const std::string& _name) {
			init(anim, _name);
		}
		BoneTrans(const std::string& _name,const std::vector<KeyPosition>& _poskeys, const std::vector<KeyRotation>& _rotkeys, const std::vector<KeyScaling>& _scakeys) {
			init(_name,_poskeys, _rotkeys, _scakeys);
		}
		void init(const aiNodeAnim* anim, const std::string& _name) {
			name = _name;
			reset();
			poskeys.resize(anim->mNumPositionKeys);
			rotkeys.resize(anim->mNumRotationKeys);
			scakeys.resize(anim->mNumScalingKeys);
			for (uint i = 0; i < anim->mNumPositionKeys; i++) {
				aiVector3D pos = anim->mPositionKeys[i].mValue;
				poskeys[i] = { vec3(pos.x,pos.y,pos.z), anim->mRotationKeys[i].mTime };
			}
			for (uint i = 0; i < anim->mNumRotationKeys; i++) {
				aiQuaternion rot = anim->mRotationKeys[i].mValue;
				rotkeys[i] = { Rotation(quat(rot.w, rot.x, rot.y, rot.z)), anim->mRotationKeys[i].mTime };
			}
			for (uint i = 0; i < anim->mNumScalingKeys; i++) {
				aiVector3D sca = anim->mScalingKeys[i].mValue;
				scakeys[i] = { Scaling(sca.x,sca.y,sca.z), anim->mRotationKeys[i].mTime };
			}
		}
		void init(const std::string& _name,const std::vector<KeyPosition>& _poskeys, const std::vector<KeyRotation>& _rotkeys, const std::vector<KeyScaling>& _scakeys) {
			name = _name; poskeys = _poskeys; rotkeys = _rotkeys; scakeys = _scakeys;
		}
		void reset() {
			posIndex = rotIndex = scaIndex = 0;
		}
		vec3 getPos(uint index) {
			if (index >= poskeys.size())return vec3(0.0f);
			return poskeys[index].pos;
		}
		Rotation getRot(uint index) {
			if (index >= rotkeys.size())return Rotation();
			return rotkeys[index].rot;
		}
		vec3 getSca(uint index) {
			if (index >= scakeys.size())return vec3(1.0f);
			return scakeys[index].sca;
		}

		Transform update(float time) {
			const uint numPos = uint(poskeys.size()), numRot = uint(rotkeys.size()), numSca = uint(scakeys.size());
			while (posIndex < numPos && poskeys[posIndex].time < time)posIndex++;
			while (rotIndex < numRot && rotkeys[rotIndex].time < time)rotIndex++;
			while (scaIndex < numSca && scakeys[scaIndex].time < time)scaIndex++;

			if (posIndex >= numPos)trans.pos = getPos(numPos - 1);
			else if (posIndex == 0)trans.pos = getPos(0);
			else trans.pos = lerpT(getPos(posIndex - 1), getPos(posIndex), getPosDelta(posIndex, time));

			if (rotIndex >= numRot)trans.rot = getRot(numRot - 1);
			else if (rotIndex == 0)trans.rot = getRot(0);
			else trans.rot = slerp(getRot(rotIndex - 1), getRot(rotIndex), getRotDelta(rotIndex, time));

			if (scaIndex >= numSca)trans.scaling = getSca(numRot - 1);
			else if (scaIndex == 0)trans.scaling = getSca(0);
			else trans.scaling = lerpT(getSca(scaIndex - 1), getSca(scaIndex), getScaDelta(scaIndex, time));

			return trans;
		}

	private:
		uint posIndex, rotIndex, scaIndex;
		float getPosDelta(uint index, float time) {
			float pre = float(poskeys[index - 1].time), post = float(poskeys[index].time);
			return (time - pre) / (post - pre);
		}
		float getRotDelta(uint index, float time) {
			float pre = float(rotkeys[index - 1].time), post = float(rotkeys[index].time);
			return (time - pre) / (post - pre);
		}
		float getScaDelta(uint index, float time) {
			float pre = float(scakeys[index - 1].time), post = float(scakeys[index].time);
			return (time - pre) / (post - pre);
		}
	};

	struct BoneData {
		std::string name;
		mat4 offset = mat4(1.0f);
	};

	struct BoneMap {
		struct _BoneData {
			uint id = 0;
			mat4 offset = mat4(1.0f);
		};
		std::unordered_map<std::string, _BoneData> mp;
		bool find(const std::string& boneName) {
			return mp.find(boneName) != mp.end();
		}
		uint addBone(const BoneData& bone) {
			uint boneID = numBones() + 1;
			mp[bone.name] = {boneID,bone.offset };
			return boneID;
		}
		uint numBones()const {
			return (uint)mp.size();
		}
		uint findID(const std::string& boneName) {
			return mp[boneName].id;
		}
	};

	class BoneTree {
	public:
		std::vector<std::list<uint>> tree;
		std::vector<BoneData> boneDatas;
		uint root = 0;
		BoneTree() {}
		void build(const aiScene* scene,BoneMap& boneMap) {
			tree.resize(boneMap.numBones()+1);
			boneDatas.resize(boneMap.numBones());
			root=_build(scene->mRootNode,boneMap);
		}
		uint numBones()const {
			return (uint)boneDatas.size();
		}
		std::list<uint>& children(uint boneID) {
			return tree[boneID];
		}
	private:
		uint _build(const aiNode* node, BoneMap& boneMap) {
			uint boneID = 0;
			std::string name = node->mName.C_Str();
			if (boneMap.find(name)){
				BoneMap::_BoneData& data = boneMap.mp[name];
				boneID = data.id;
				boneDatas[boneID-1] = BoneData{name,data.offset};
			}
			for (uint i = 0; i < node->mNumChildren; i++) {
				uint ch = _build(node->mChildren[i], boneMap);
				if (ch)tree[boneID].push_back(ch);
			}
			return boneID;
		}
	};

	class Animation {
	public:
		std::vector<BoneTrans> boneTrans;
		std::vector<mat4> boneMatrices;
		std::string name;
		BoneTree* boneTree;
		double duration = 0.0, currentTime = 0.0, ticksPerSecond = 0.0;
		Animation() {}
		Animation(const aiScene* scene, aiAnimation* animation, BoneMap& boneMap,BoneTree* tree) {
			boneTree = tree;
			name = animation->mName.C_Str();
			boneTrans.resize(boneMap.numBones());
			boneMatrices.resize(boneMap.numBones(), identity4());
			for (uint i = 0; i < animation->mNumChannels; i++) {
				aiNodeAnim* anim = animation->mChannels[i];
				std::string name = anim->mNodeName.C_Str();
				if (!boneMap.find(name)) {
					boneMap.addBone(BoneData{ name,identity4() });
					boneTrans.emplace_back(BoneTrans());
					boneMatrices.emplace_back(identity4());
				}
				boneTrans[boneMap.findID(name)-1].init(anim, name);
			}
			duration = animation->mDuration;
			ticksPerSecond = animation->mTicksPerSecond;
			reset();
			//root.ptr = buildTree(scene->mRootNode, boneMap);
			//boneTree.build(scene, boneMap);
		}
		~Animation() {

		}
		void finish() {
			currentTime = duration;
		}
		bool isFinished()const {
			return currentTime == duration;
		}
		void reset() {
			currentTime = 0.0;
			for (auto& bone : boneTrans)bone.reset();
			timer.reset();
		}
		void update(double speed) {
			if (isFinished())return;
			currentTime += ticksPerSecond * speed* timer.interval();
			currentTime = std::min(currentTime, duration);
		}
		void pause() {
			timer.reset();
		}
		void setMatrices(const std::string& name = "boneMatrices") {
			if (!GLSLProgram::activatedProgram)return;
			calcBoneTransform(boneTree->root, identity4());
			const std::string prefix = name + "[";
			for (uint i = 0; i < boneMatrices.size(); i++) {
				GLSLProgram::activatedProgram->set(prefix + toString(i + 1) + "]", boneMatrices[i]);
			}
		}
		double getDuration()const {
			return duration / ticksPerSecond;
		}
		double getCurrentTime()const {
			return currentTime / ticksPerSecond;
		}
		void setCurrentTime(double time) {
			for (auto& bone : boneTrans)bone.reset();
			currentTime = time * ticksPerSecond;
			currentTime = std::min(currentTime, duration);
		}
	private:
		FrameTimer timer;
		void calcBoneTransform(uint boneID, const mat4& parentMatrix) {
			mat4 transform, matrix(1.0f);
			if (boneID) {
				transform = boneTrans[boneID - 1].update(float(currentTime)).toMatrix();
				matrix = parentMatrix * transform;
				boneMatrices[boneID - 1] = matrix * boneTree->boneDatas[boneID-1].offset;
			}
			for (uint i:boneTree->children(boneID))
				calcBoneTransform(i, matrix);
		}
	};


	class Meshes {
	public:
		struct TriMesh :public Mesh {
			MaterialData material;
			uint boneid = 0;
			std::string name;
		};
		std::list<TriMesh> meshes;
		std::vector<Animation*> animations;
		BoneTree* boneTree = 0;
		Meshes() {}
		void release() {
			//SafeDelete(boneTree);
			//for (Animation*& anim : animations)SafeDelete(anim);
		}
		bool hasAnimations()const {
			return animations.size();
		}
	};
}




