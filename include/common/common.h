#pragma once

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#define GLEW_STATIC 1
#include "../3rd/GL/glew.h"
#define GLFW_INCLUDE_NONE
#include "../3rd/GL/glfw3.h"
#include "../3rd/GL/GL.h"

#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>

#include <map>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <stack>
#include <queue>
#include <list>
#include <functional>

#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <cmath>

#include "../3rd/glm/glm.hpp"
#include "../3rd/glm/gtc/matrix_transform.hpp"
#include "../3rd/glm/gtc/type_ptr.hpp"
#include "../3rd/glm/gtx/euler_angles.hpp"
#include "../3rd/glm/gtx/quaternion.hpp"
#include "../3rd/glm/gtc/quaternion.hpp"
#include "../3rd/glm/common.hpp"

#include "String.h"


#define identity4() mat4(1.0f)

#define printvec2(v) Festa::printvec<vec2,2>(v)
#define printvec3(v) Festa::printvec<vec3,3>(v)
#define printvec4(v) Festa::printvec<vec4,4>(v)
#define printmat4(m) Festa::printmat<mat4,4>(m)

#define getTime() glfwGetTime()
#define EPS_FLOAT 0.001f
//#define EPS_FLOAT 0.1f

#define VEC3X vec3(1.0f,0.0f,0.0f)
#define VEC3Y vec3(0.0f,1.0f,0.0f)
#define VEC3Z vec3(0.0f,0.0f,1.0f)
#define EQUAL_FLOAT(a,b) (fabsf(a-b)<EPS_FLOAT)

namespace Festa {
	typedef glm::mat4 mat4;
	typedef glm::mat3 mat3;
	typedef glm::mat2 mat2;
	typedef glm::vec4 vec4;
	typedef glm::vec3 vec3;
	typedef glm::vec2 vec2;
	typedef glm::ivec2 ivec2;
	typedef glm::ivec3 ivec3;
	typedef glm::ivec4 ivec4;
	typedef glm::ivec2 vec2i;
	typedef glm::ivec3 vec3i;
	typedef glm::ivec4 vec4i;
	typedef glm::quat quat;
	const float PI = glm::pi<float>();

	
	
	template<typename T1, typename T2>
	T1 fastpow(T1 base, T2 pow) {
		T1 ret = T1(1);
		while (pow) {
			if (pow & T2(1))ret *= base;
			base *= base;
			pow >>= 1;
		}
		return ret;
	}
	inline float radians(float degree) {
		return degree * PI / 180.0f;
	}
	inline float degree(float radians) {
		return radians * 180.0f / PI;
	}

	inline mat4 translate4(const vec3& pos) {
		return glm::translate(mat4(1.0f), pos);
	}
	inline mat4 rotate4(float angle, const vec3& axis) {
		return glm::rotate(mat4(1.0f), angle, axis);
	}
	inline mat4 scale4(const vec3& scaling) {
		return glm::scale(mat4(1.0f), scaling);
	}
	template<typename T>
	inline T abs(const T& x) {
		return x >= 0 ? x : -x;
	}
	template<typename T>
	inline int sgn(const T& x) {
		const T z(0);
		return (abs(x)<=T(EPS_FLOAT)) ? 0 : ((x < T(0)) ? -1 : 1);
	}
	template<typename T>
	inline T clampT(const T& x, const T& a, const T& b) {
		return std::min(std::max(x, a), b);
	}
	template<typename T>
	inline T lerpT(const T& a, const T& b, float t) {
		return a * (1.0f - t) + b * t;
	}
	inline double roundf(double x, int prec) {
		int tmp = prec;
		while (tmp--)x *= 10.0;
		x = double(int(x + 0.5));
		tmp = prec;
		while (tmp--)x /= 10.0;
		return x;
	}
	inline double randf() {
		return double(rand()) / double(RAND_MAX);
	}
	inline double randf(double a, double b) {
		return a + randf() * (b - a);
	}
	inline double normalf(double u,double sigma) {
		double u1 = randf(), u2 = randf();
		return u+sigma*sqrt(-2.0*log(u1))*cos(2.0*PI*u2);
	}
	inline int randint() {
		return rand();
	}
	inline int randint(int a, int b) {
		return a + int(randf() * double(b - a));
	}
	template<typename T>
	T randT(const T& a, const T& b) {
		return a + T(randf()) * (b - a);
	}
	template<typename T>
	T normalT(const T& u, const T& sigma) {
		double u1 = randf(), u2 = randf();
		return u + sigma * T(sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2));
	}

	template<typename T, int size>
	inline void printvec(const T& vec) {
		for (int i = 0; i < size; i++)std::cout << vec[i] << " ";
		std::cout << std::endl;
	}

	template<typename T, int size>
	inline void printmat(const T& mat) {
		// rx,ry,rz
		// +  +  +
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++)std::cout << mat[i][j] << " ";
			std::cout << std::endl;
		}
	}

	template<typename T>
	class BezierCurve {
	public:
		BezierCurve() {}
		BezierCurve(int _levels) {
			SetLevels(_levels);
		}
		BezierCurve(const std::vector<T>& points) {
			mPoints = points;
		}
		void Insert(const T& p) {
			mPoints.push_back(p);
		}
		void SetLevels(int _levels) {
			mPoints.resize(_levels);
		}
		int levels()const {
			return int(mPoints.size());
		}
		const T& point(int i)const {
			return mPoints[i];
		}
		T& point(int i) {
			return mPoints[i];
		}
		const T& operator[](int i)const {
			return mPoints[i];
		}
		T& operator[](int i) {
			return mPoints[i];
		}
		T interpolate(float t)const {
			if (!levels())return T();
			std::vector<T> v = mPoints;
			int count = levels() - 1;
			while(count){
				for (int j = 0; j < count; j++) {
					v[j] = v[j] * (1.0f - t) + v[j + 1] * t;
				}
				count--;
			}
			return v[0];
		}
		T operator()(float t)const {
			return interpolate(t);
		}
	private:
		std::vector<T> mPoints;
	};

	template<typename T>
	void SafeDelete(T*& ptr) {
		if (ptr!=nullptr) { 
			try {
				delete ptr;
				//free(ptr);
				ptr = nullptr;
			}
			catch (...) {}
			 
		}
	}

	template<typename T>
	class amr_ptr {
	public:
		T* ptr = nullptr;
		amr_ptr() {}
		amr_ptr(const T& val) :ptr(new T(val)) {}
		amr_ptr(const amr_ptr<T>& p) {
			if (p.ptr != nullptr)ptr = new T(*p.ptr);
		}
		void operator=(const amr_ptr<T>& p) {
			SafeDelete(ptr);
			if (p.ptr != nullptr)ptr = new T(*p.ptr);
		}
		~amr_ptr() {
			release();
		}
		void release() {
			//cout << "amr " << ptr << endl;
			SafeDelete(ptr);
		}
		amr_ptr operator+(int x)const {
			return amr_ptr(ptr + x);
		}
		void operator+=(int x) {
			ptr += x;
		}
		T& operator*()const {
			return *ptr;
		}
		T& operator[](int x)const {
			return *(ptr + x);
		}
		T* operator->()const {
			return ptr;
		}
		operator bool()const {
			return ptr;
		}

	};

	typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALFARPROC)(int);
	extern PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT;

	template<uint Target>
	class GLBuffer {
	public:
		GLBuffer() {}
		GLBuffer(uint _id):id(_id) {

		}
		GLBuffer(size_t size, const void* data, uint usage) {
			generate(size, data, usage);
		}
		template<typename T>
		GLBuffer(const std::vector<T>& data, uint usage) {
			generate(data, usage);
		}
		~GLBuffer() {
			release();
		}
		void create() {
			release();
			glGenBuffers(1, &id);
		}
		void generate(size_t size, const void* data, uint usage) {
			create();
			bindData(size, data, usage);
		}
		template<typename T>
		void generate(const std::vector<T>& data, uint usage) {
			create();
			bindData(data, usage);
		}
		void bindData(size_t size, const void* data, uint usage) {
			bind();
			glBufferData(Target, size, data, usage);
			//unbind();
		}
		template<typename T>
		void bindData(const std::vector<T>& data, uint usage) {
			bind();
			glBufferData(Target, data.size()*sizeof(T), &data[0], usage);
			//unbind();
		}
		void release() {
			if(id)glDeleteBuffers(1, &id);
			id = 0;
		}
		void bind()const {
			glBindBuffer(Target, id);
		}
		void bindBufferBase(uint target, uint index)const {
			glBindBufferBase(target, index, id);
		}
		static void unbind() {
			glBindBuffer(Target, 0);
		}
		operator bool()const {
			return id;
		}
		uint ID()const {
			return id;
		}
		uint& _ID() {
			return id;
		}
		operator uint()const {
			return id;
		}
		void Transfer(GLBuffer<Target>& buf) {
			buf.id = id;
			id = 0;
		}
	private:
		uint id = 0;
	};

	typedef GLBuffer<GL_ARRAY_BUFFER> GLArrayBuffer;

	template<typename T, typename ID_t = uint>
	class BufferGenerator {
	public:
		BufferGenerator() {}
		void gen(ID_t& id) {
			if (q.size()) {
				id = q.front(); q.pop();
				buffers[q - 1] = T();
			}
			else {
				id = ID_t(buffers.size() + 1);
				buffers.push_back(T());
			}
		}
		void del(ID_t& id) {
			q.push(id);
			id = 0;
		}
		T& buffer(ID_t id)const {
			return buffers[id - 1];
		}
		uint numBuffers()const {
			return buffers.size() - q.size();
		}
	private:
		std::list<T> buffers;
		std::queue<ID_t> q;
	};

	struct Version2 {
		int major = 0, minor = 0;
		Version2() {}
		Version2(double version) {
			init(version);
		}
		Version2(const std::string& version) {
			init(stringTo<double>(version));
		}
		void init(double version) {
			major = int(version);
			minor = int(version * 10.0) % 10;
		}
		double toDouble()const {
			return double(major) + double(minor) / 10.0;
		}
		std::string toStr()const {
			return toString(toDouble());
		}
		std::string glFormat(const std::string& post="")const {
			if (post.size())return "#version " + toString(major * 100 + minor * 10) + " "+post;
			else return "#version " + toString(major * 100 + minor * 10);
		}
		friend std::ostream& operator<< (std::ostream& out, const Version2& version) {
			out << version.toStr();
			return out;
		}
		bool operator==(const Version2& x)const {
			return x.major == major && x.minor == minor;
		}
		bool operator!=(const Version2& x)const {
			return x.major != major || x.minor != minor;
		}
	};
	template<typename T>
	struct VirtualValue {
		typedef std::function<void(const T&)> Setf;
		typedef std::function<T()> Getf;
		Setf set; Getf get;
		VirtualValue(){}
		VirtualValue(Setf setf,Getf getf):set(setf),get(getf) {
			
		}
		void operator=(const T& value) {
			set(value);
		}
		operator T()const {
			return get();
		}
	};

	template<typename Size>
	class _BoolList {
	public:
		struct iterator {
			_BoolList* obj = 0;
			Size i = 0;
			iterator() {}
			iterator(_BoolList* obj,Size i) :obj(obj),i(i) {

			}
			iterator operator++() {
				i++;
				return *this;
			}
			VirtualValue<bool> operator*() {
				return (*obj)[i];
			}
			bool operator!=(const iterator& it)const {
				return i < obj->size();
			}
		};
		_BoolList() {}
		_BoolList(Size s) {
			resize(s);
		}
		void set(Size i, bool t) {
			if (t)data[i / 8] |= (1 << (i % 8));
			else data[i / 8] = ~((~data[i / 8]) | (1 << (i % 8)));
		}
		bool get(Size i)const {
			return data[i / 8] & (1 << (i % 8));
		}
		VirtualValue<bool> operator[](Size i) {
			return VirtualValue<bool>(
				[=](bool t) {set(i, t); },
				[=]() {return get(i); }
			);
		}
		VirtualValue<bool> back() {
			return (*this)[msize - 1];
		}
		iterator begin() {
			return iterator(this, 0);
		}
		iterator end()const {
			return iterator();
		}
		void expand(Size s) {
			resize(size()+s);
		}
		void resize(Size s) {
			msize = s;
			if (s > capacity())
				data.resize(data.size() + Size(ceil(double(s) / 8.0)));
		}
		Size capacity()const {
			return Size(data.size())*Size(8);
		}
		Size size()const {
			return msize;
		}
		void append(bool x) {
			expand(1);
			back() = x;
		}
		_BoolList operator+(const _BoolList& x)const {
			_BoolList ret(msize+x.size());
			Size j = 0;
			for (Size i = 0; i < msize; i++)ret[j++] = get(i);
			for (Size i = 0; i < x.size(); i++)ret[j++] = x.get(i);
			return ret;
		}
		void operator+=(const _BoolList& x) {
			Size j = msize;
			expand(x.size());
			for (Size i = 0; i < x.size(); i++)set(j + i, x.get(i));
		}
		bool all()const {
			for (Size i = 0; i < msize; i++)
				if (!(*this)[i])return false;
			return true;
		}
		bool any()const{
			for (Size i = 0; i < msize; i++)
				if ((*this)[i])return true;
			return false;
		}
		_BoolList operator!()const {
			_BoolList ret = *this;
			for (uchar& b : data)
				b = ~b;
			return ret;
		}
	private:
		std::vector<uchar> data;
		Size msize = 0;
	};
	typedef _BoolList<uint> BoolList;


	template<typename T>
	//typedef int T;
	class OrderedDict {
	public:
		typedef std::map<std::string, uint> map_t;
		typedef std::pair<const std::string&, T&> pair_t;
		struct iterator {
			OrderedDict* obj=0;
			uint i = 0;
			iterator() {}
			iterator(const OrderedDict* _obj,uint _i) :obj((OrderedDict*)((void*)_obj)) {
				i = _i;
			}
			bool operator!=(const iterator& it)const {
				return i!=it.i;
			}
			pair_t operator*()const {
				if (obj->c[i].str)return { *obj->c[i].str,obj->c[i].data };
				else return  {"",obj->c[i].data };
			}
			amr_ptr<pair_t> operator->()const {
				return amr_ptr<pair_t>(**this);
			}
			iterator operator++() {
				i++;
				return *this;
			}
			iterator operator--() {
				i--;
				return *this;
			}
		};
		OrderedDict() {}
		bool find(const std::string& key)const {
			return m.find(key) != m.end();
		}
		T& insert(const std::string& key, const T& value) {
			if (find(key)) {
				return (*this)[key]=value;
			}
			m[key] = uint(c.size());
			c.push_back({ &(m.find(key)->first),value });
			return c.back().data;
		}
		void erase(const std::string& key) {
			map_t::iterator it = m.find(key);
			if (it == m.end())return;
			for (uint i = it->second + 1; i < size(); i++) {
				m[*c[i].str]--;
			}
			c.erase(c.begin() + it->second);
			m.erase(key);
		}
		void erase(const iterator& it) {
			erase((*it).first);
		}
		void rename(const std::string& oldName, const std::string& newName) {
			if (oldName == newName)return;
			insert(newName, (*this)[oldName]);
			erase(oldName);
		}
		T& operator[](const std::string& key) {
			if (!find(key))insert(key,T());
			return c[m[key]].data;
		}
		T operator[](const std::string& key)const {
			if (!find(key)) { LOGGER.error("Key Error: " + key); return T(); }
			return c[((OrderedDict*)((void*)this))->m[key]].data;
		}
		T& operator[](uint i) {
			return c[i].data;
		}
		T operator[](uint i)const {
			return c[i].data;
		}
		const std::string& key(uint i)const {
			return *c[i].str;
		}
		uint index(const std::string& key)const {
			return ((OrderedDict*)((void*)this))->m[key];
		}
		iterator iter(uint i)const {
			return iterator(this, i);
		}
		iterator iter(const std::string& key)const {
			return iterator(this,m[key]);
		}
		iterator begin()const {
			return iterator(this,0);
		}
		iterator end()const {
			return iterator(this, uint(c.size()));
		}
		uint size()const {
			return uint(c.size());
		}
		void clear() {
			c.clear();
			m.clear();
		}
		OrderedDict copy()const {
			return *this;
		}
		OrderedDict operator+(const OrderedDict& x)const {
			OrderedDict res = copy();
			for (auto i : x)res[i.first] = i.second;
		}
		OrderedDict operator+=(const OrderedDict& x) {
			for (auto i : x)insert(i.first, i.second);
		}
	private:
		struct Node {
			const std::string* str;
			T data;
		};
		std::vector<Node> c;
		map_t m;
	};

#define CHECK_STACK if(!st.size()){formatError("empty stack");return;}
	class JsonData {
	public:
		typedef std::map<std::string, uint> map_t;
		typedef std::pair<const std::string&, JsonData&> pair_t;
		struct iterator {
			JsonData* obj = 0;
			uint i = 0;
			iterator() {}
			iterator(const JsonData* _obj, uint _i) :obj((JsonData*)((void*)_obj)) {
				i = _i;
			}

			bool operator!=(const iterator& it)const {
				return i != it.i;
			}
			pair_t operator*()const {
				return { obj->c[i].str,*obj->c[i].x };
			}
			amr_ptr<pair_t> operator->()const {
				return **this;
			}
			iterator operator++() {
				i++;
				return *this;
			}
			iterator operator--() {
				i--;
				return *this;
			}
		};
		struct Node {
			amr_ptr<JsonData> x;
			std::string str;
		};
		std::string data;
		std::vector<Node> c;
		map_t m;
		char type = 0;
		JsonData() {}
		JsonData(const char* str) {
			data = std::string(str); type = '\'';
		}
		JsonData(const std::string& str) {
			data = str; type = '\'';
		}
		JsonData(char x) { init(x); type = '\''; }
		JsonData(uchar x) { init(x); }
		JsonData(int x) { init(x); }
		JsonData(uint x) { init(x); }
		JsonData(ll x) { init(x); }
		JsonData(ull x) { init(x); }
		JsonData(float x) { init(x); }
		JsonData(double x) { init(x); }
		static JsonData Dict() {
			JsonData res;
			res.type = '{';
			return res;
		}
		static JsonData List() {
			JsonData res;
			res.type = '[';
			return res;
		}
		template<typename T>
		static JsonData List(const std::vector<T>& vec) {
			JsonData res;
			res.type = '[';
			c.resize(vec.size());
			for (size_t i = 0; i < vec.size(); i++) {
				c[i].x = JsonData(vec[i]);
			}
			return res;
		}
		template<typename T>
		void init(const T& x) {
			data = toString(x);
		}
		template<typename T>
		VirtualValue<T> to() {
			return VirtualValue<T>(
				[this](const T& value) {data = toString(value); },
				[this] {return stringTo<T>(data); });
		}
		operator std::string()const {
			return data;
		}
		std::string& ToString() {
			return data;
		}
		std::vector<std::string> keys()const {
			return {};
		}
		bool operator==(const JsonData& x)const {
			//create class Vector and String
			return false;//imcom
		}
		bool operator==(const std::string& x)const {
			if (isString())return data == x;
			return false;
		}
		bool operator==(const char* x)const {
			if (isString())return data == x;
			return false;
		}
		bool operator==(ull x)const {
			if (isValue())return data == toString(x);
			return false;
		}
		bool operator==(ll x)const {
			if (isValue())return data == toString(x);
			return false;
		}
		iterator begin()const {
			return iterator(this, 0);
		}
		iterator end()const {
			return iterator(this, size());
		}
		JsonData& operator[](const std::string& key) {
			/*if (!isDict())throw "Only a dict can be indexed by string";
			else if (!find(key))throw "KeyError: " + key;
			return *c[m[key]].x;*/
			if (!isDict())LOGGER.error("Only a dict can be indexed by string");
			else if (!find(key))LOGGER.error("KeyErrosr: " + key);
			return *c[m[key]].x;
		}
		JsonData operator[](const std::string& key)const {
			if (!isDict())throw "Only a dict can be indexed by string";
			else if (!find(key))throw "KeyError: " + key;
			return *c[((JsonData*)(void*)this)->m[key]].x;
		}
		JsonData& operator[](uint index) {
			if (index >= c.size())throw "IndexError: " + toString(index);
			return *c[index].x;
		}
		JsonData operator[](uint index)const {
			if (index >= c.size())throw "IndexError: " + toString(index);
			return *c[index].x;
		}
		void append(const JsonData& x) {
			c.emplace_back(Node{ x,"" });
		}
		JsonData& insert(const std::string& key, const JsonData& value) {
			if (find(key))
				return (*this)[key] = value;
			m[key] = uint(c.size());
			c.emplace_back(Node{ value,(m.find(key)->first) });
			return *c.back().x;
		}
		void erase(const std::string& key) {
			map_t::iterator it = m.find(key);
			if (it == m.end())return;
			for (uint i = it->second + 1; i < size(); i++) {
				m[c[i].str]--;
			}
			c.erase(c.begin() + it->second);
			m.erase(key);
		}
		void erase(const iterator& it) {
			erase((*it).first);
		}
		void rename(const std::string& oldName, const std::string& newName) {
			if (oldName == newName)return;
			insert(newName, (*this)[oldName]);
			erase(oldName);
		}
		uint size()const {
			return uint(c.size());
		}
		std::string serialize()const {
			if (isString())return "\"" + data + "\"";
			else if (isValue())return data;
			if (isList()) {
				std::string ret = "[";
				for (uint i = 0; i < c.size(); i++) {
					ret += c[i].x->serialize() + ",";
				}
				if (size())ret.back() = ']';
				else ret += "]";
				return ret;
			}

			std::string ret = "{";
			for (auto& i : m) {
				ret += "\"" + i.first + "\":" + c[i.second].x->serialize() + ",";
			}
			if (size())ret.back() = '}';
			else ret += "}";
			return ret;
		}
		friend std::ostream& operator<< (std::ostream& out, const JsonData& data) {
			out << data.serialize();
			return out;
		}
		bool isDict()const {
			return type == '{';
		}
		bool isList()const {
			return type == '[';
		}
		bool isValue()const {
			return type != '{' && type != '[';
		}
		bool isString()const {
			return type == '\'' || type == '\"';
		}

		void deserialize(const std::string& str) {
			JsonLoader loader(str);
			if (loader.st.size() && loader.st.top())
				*this = *loader.st.top();
		}
		void reset() {
			data.clear();
			m.clear();
			c.clear();
			type = 0;
		}
		void clear() {
			m.clear();
			c.clear();
		}
		bool find(const std::string& key)const {
			return m.find(key) != m.end();
		}

		JsonData copy()const {
			return *this;
		}
		JsonData operator+(const JsonData& x)const {
			JsonData ret = copy();
			for (auto i : x) {
				if (find(i.first))throw "Duplicated key: " + i.first;
				ret.insert(i.first, i.second);
			}
			return ret;
		}
		void operator+=(const JsonData& x) {
			for (auto i : x) {
				if (find(i.first))throw "Duplicated key: " + i.first;
				insert(i.first, i.second);
			}
		}
		bool empty()const {
			return !type && !c.size();
		}
		void load(const Path& path) {
			if (!empty())clear();
			File file(path);
			std::string str;
			file.readLines(str);
			deserialize(str);
		}
	private:
		struct JsonLoader {
			std::stack<JsonData*> st;
			std::stack<std::string> keys;
			bool topDone = true;
			bool error = false;
			const std::string* strp = 0;
			std::string data;
			JsonLoader() {}
			JsonLoader(const std::string& str) :strp(&str) {
				for (char s : str) {
					if (s == '\"' || s == '\'') {
						CHECK_STACK;
						if (st.top()->type == s) {
							st.top()->data = data;
							data.clear();
							topDone = true;
							continue;
						}
						else if (!st.top()->isString()) {
							st.push(new JsonData()); st.top()->type = s;
							topDone = false;
							continue;
						}
					}
					if (st.size() && st.top()->isString() && !topDone) {
						data.push_back(s);
						continue;
					}
					else if (s == '\n' || s == ' ' || s == '\t')continue;
					else if (s == '{' || s == '[') {
						st.push(new JsonData()); st.top()->type = s;
						topDone = false;
					}
					else if (s == ':') {
						CHECK_STACK;
						if (!st.top()->isString()) { formatError("key must be str"); return; }
						JsonData* tmp = st.top(); st.pop();
						keys.push(tmp->data);
						if (!st.top()->isDict()) { formatError("key:value must be in a dict"); return; }
					}
					else if (s == ',') {
						if (end())return;
					}
					else if (s == '}' || s == ']') {
						if (end())return;
						if (!pairOp(st.top()->type, s)) {
							formatError(std::string("unmatched op: ") + st.top()->type + " and " + s);
							return;
						}
						topDone = true;
					}
					else data.push_back(s);
				}
			}
			bool end() {
				JsonData* tmp;
				if (data.size()) {
					tmp = new JsonData();
					tmp->data = data;
					data.clear();
				}
				else {
					if (!topDone)return false;
					tmp = st.top(); st.pop();
				}
				if (!st.size()) {
					formatError("empty stack");
					return true;
				}
				JsonData& top = *st.top();
				top.c.push_back({ *tmp,"" });

				SafeDelete(tmp);
				if (top.isList()) return false;
				else if (!keys.size()) {
					formatError("missing key in dict");
					return true;
				}
				top.m[keys.top()] = uint(st.top()->c.size()) - 1;
				top.c.back().str = (top.m.find(keys.top())->first);
				keys.pop();
				return false;
			}
			void formatError(const std::string& info) {
				throw "JsonData::FormatError: ";
				error = true;
			}
			static bool pairOp(char a, char b) {
				switch (a) {
				case '{':
					return b == '}';
				case '[':
					return b == ']';
				default:
					return false;
				}
			}
		};
	};

#define FESTA_LEFT_TOP 4
#define FESTA_ORIGIN 1
#define FESTA_CENTER 2
#define FESTA_LEFT_BOTTOM 3

	struct WindowPosition {
		vec2 pos = vec2(0.0f);
		int type = -1;
		WindowPosition() {}
		WindowPosition(const vec2& pos, int type = FESTA_LEFT_TOP)
			:pos(pos), type(type) {}
		WindowPosition tolt()const {
			switch (type) {
			case FESTA_LEFT_TOP:
				return *this;
			case FESTA_CENTER:
				return WindowPosition();
			case FESTA_LEFT_BOTTOM:
				return WindowPosition();
			}
		}
		WindowPosition to(int type_)const {
			return WindowPosition();
		}
	};

	template<typename T>
	struct UpdatingValue {
		T prev, now;
		UpdatingValue() {}
		UpdatingValue(const T& x) {
			now = x;
		}
		UpdatingValue(const T& _prev, const T& _now)
			:prev(_prev), now(_now) {}
		void operator=(const UpdatingValue<T>& x) {
			prev = x.prev; now = x.now;
		}
		template<typename t>
		void operator=(const t& x) = delete;
		void update(const T& x) {
			prev = now;
			now = x;
		}
	};

	struct Timer {
		std::chrono::steady_clock::time_point clock;
		Timer() {
			reset();
		}
		void reset() {
			clock = std::chrono::high_resolution_clock::now();
		}
		double interval()const {
			return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - clock).count();
		}
		double fps()const {
			double i = interval();
			return i == 0.0 ? 0.0 : 1.0 / i;
		}
	};

	/*class Timer {
	public:
		Timer() { reset(); }
		void reset() {
			st = getTime();
		}
		double interval()const {
			return getTime() - st;
		}
		double fps()const {
			double i = interval();
			return i == 0.0 ? 0.0 : 1.0 / i;
		}
	private:
		double st = 0.0f;
	};
	*/

	struct FrameTimer {
		std::chrono::steady_clock::time_point clock;
		bool first = true;
		FrameTimer() {}
		void reset() {
			first = true;
		}
		bool firstFrame()const {
			return first;
		}
		void update() {
			first = false;
			clock = std::chrono::high_resolution_clock::now();
		}
		double get()const {
			return firstFrame() ? 0.0 : std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - clock).count();
		}
		double interval() {
			if (firstFrame()) {
				clock = std::chrono::high_resolution_clock::now();
				first = false;
				return 0.0;
			}
			double ret = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - clock).count();
			clock = std::chrono::high_resolution_clock::now();
			return ret;
		}
	};

	/*class FrameTimer {
	public:
		FrameTimer() {  }
		void reset() {
			st = 0.0;
		}
		double interval() {
			if (firstFrame()) {
				st = getTime();
				return 0.0;
			}
			double ret = getTime() - st;
			st = getTime();
			return ret;
		}
		double get() {
			if (firstFrame())return 0.0;
			return getTime() - st;
		}
		void update() {
			st=getTime();
		}
		bool firstFrame()const {
			return st == 0.0;
		}
		static double fps(double inter) {
			return inter == 0.0 ? -1.0 : 1.0 / inter;
		}
	private:
		double st = 0.0;
	};
	*/
}
