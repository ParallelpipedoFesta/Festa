#pragma once

#include "transformation.h"
#include "mesh.h"
#include <cmath>

namespace Festa {
	/*class TriangleMesh {
	public:
		std::vector<Triangle> mesh;
		TriangleMesh() {}
		TriangleMesh(const Mesh& m) {
			init(m);
		}
		TriangleMesh(const Meshes& meshes) {
			for (Meshes::TriMesh m : meshes.meshes)init(m);
		}
		uint numVertices()const {
			return uint(mesh.size() * 3);
		}
		uint numTriangles()const {
			return uint(mesh.size());
		}
		const void* ptr()const {
			return &mesh[0];
		}
		Mesh toMesh()const {
			return Mesh(std::vector<float>((float*)ptr(), (float*)ptr() + numVertices() * 3), "3");
		}
		void operator+=(const TriangleMesh& other) {
			mesh.insert(mesh.end(), other.mesh.begin(), other.mesh.end());
		}
		TriangleMesh operator+(const TriangleMesh& other)const {
			TriangleMesh res = TriangleMesh(*this);
			res += other;
			return res;
		}
	private:
		void init(const Mesh& m) {
			uint pre = uint(mesh.size()), stride = m.format.stride;
			if (m.indices.size()) {
				uint vcount = uint(m.indices.size());
				mesh.resize(pre + vcount / 3);
				for (uint i = 0; i < vcount / 3; i++) {
					for (uint j = 0; j < 3; j++) {
						uint index = m.indices[i * 3 + j] * stride;
						mesh[pre + i].tri[j] = vec3(m.vertices[index], m.vertices[index + 1], m.vertices[index + 2]);
					}
				}
			}
			else {
				uint vcount = uint(m.vertices.size()) / stride;
				mesh.resize(pre + vcount / 3);
				for (uint i = 0; i < vcount / 3; i++) {
					for (uint j = 0; j < 3; j++) {
						uint index = (i * 3 + j) * stride;
						mesh[pre + i].tri[j] = vec3(m.vertices[index], m.vertices[index + 1], m.vertices[index + 2]);
					}
				}
			}
		}
	};*/

	struct Ray {
		vec3 ori=vec3(0.0f);
		vec3 dir=vec3(0.0f);
		Ray() {}
		Ray(const vec3& _ori, const vec3& _dir) {
			ori = _ori;
			dir = glm::normalize(_dir);
		}
		void fromTo(const vec3& from, const vec3& to) {
			ori = from;
			dir = glm::normalize(to - from);
		}
		vec3 operator()(float t)const {
			return ori + dir * t;
		}
	};

	class AABB {
	public:
		vec3 min = vec3(INFINITY), max = vec3(-INFINITY);
		AABB() {}
		AABB(const vec3& size) {
			max = size * 0.5f;
			min = -max;
		}
		AABB(const vec3& min, const vec3& max)
			:min(min), max(max) {}
		AABB(const std::vector<vec3>& vertices) {
			update(vertices);
		}
		AABB(const Mesh& mesh) {
			update(mesh);
		}
		AABB(const Meshes& meshes) {
			update(meshes);
		}
		void set(const vec3& center, const vec3& size) {
			min = center - size * 0.5f;
			max = center + size * 0.5f;
		}
		void clear() {
			min = vec3(INFINITY), max = vec3(-INFINITY);
		}
		bool empty()const {
			return min.x > max.x || min.y > max.y || min.z > max.z;
		}
		operator bool()const {
			return min.x <= max.x && min.y <= max.y && min.z <= max.z;
		}
		void updateX(float x) {
			min.x = std::min(min.x, x);
			max.x = std::max(max.x, x);
		}
		void updateY(float y) {
			min.y = std::min(min.y, y);
			max.y = std::max(max.y, y);
		}
		void updateZ(float z) {
			min.z = std::min(min.z, z);
			max.z = std::max(max.z, z);
		}
		void update(float x, float y, float z) {
			updateX(x); updateY(y); updateZ(z);
		}
		void update(const vec3& v) {
			update(v.x, v.y, v.z);
		}
		void update(const std::vector<vec3>& vertices) {
			for (uint i = 0; i < vertices.size(); i++) update(vertices[i]);
		}
		void update(const Mesh& mesh) {
			for (uint i = 0; i < mesh.numVertices(); i++)
				update(mesh.position(i));
		}
		void update(const Meshes& meshes) {
			for (const Meshes::TriMesh& mesh:meshes.meshes)update(mesh);
		}
		void update(const AABB& aabb) {
			update(aabb.min); update(aabb.max);
		}
		void translate(const vec3& pos) {
			min += pos; max += pos;
		}
		void rotate(const Rotation& rot) {
			*this *= rot.ToMat4();
		}
		void scale(const Scaling& scaling) {
			min *= scaling.toVec3(); max *= scaling.toVec3();
		}
		vec3 size()const {
			return max - min;
		}
		vec3 center()const {
			return (min + max) / 2.0f;
		}
		std::vector<vec3> vertices()const {
			return {
				vec3(min.x,min.y,min.z),vec3(min.x,min.y,max.z),
				vec3(min.x,max.y,min.z),vec3(min.x,max.y,max.z),
				vec3(max.x,min.y,min.z),vec3(max.x,min.y,max.z),
				vec3(max.x,max.y,min.z),vec3(max.x,max.y,max.z),
			};
		}
		void operator*=(const mat4& trans) {
			const std::vector<vec3> v = vertices();
			clear();
			for (const vec3& x : v) {
				vec3 y = trans * vec4(x, 1.0f);
				update(y.x, y.y, y.z);
			}
		}
		void operator*=(const Transformation& trans) {
			const std::vector<vec3> v = vertices();
			clear();
			for (const vec3& x : v) {
				const vec3 y = trans.apply(x);
				update(y.x, y.y, y.z);
			}
		}
		AABB operator+(const vec3& t)const {
			return AABB(min + t, max + t);
		}
		AABB operator-(const vec3& t)const {
			return AABB(min - t, max - t);
		}
		AABB operator*(const mat4& trans)const {
			AABB res;
			std::vector<vec3> v = vertices();
			for (const vec3& x : v) {
				vec3 y = trans * vec4(x, 1.0f);
				res.update(y.x, y.y, y.z);
			}
			return res;
		}
		AABB operator*(const Transformation& trans)const {
			AABB res;
			std::vector<vec3> v = vertices();
			for (const vec3& x : v) {
				vec3 y = trans.apply(x);
				res.update(y.x, y.y, y.z);
			}
			return res;
		}
		Transformation cubeTransform()const {
			Transformation trans;
			trans.setPosition(center());
			trans.setScale(size());
			return trans;
		}
		void print()const {
			printf("max: "); printvec3(max);
			printf("min: "); printvec3(min);
			printf("center: "); printvec3(center());
			printf("size: "); printvec3(size());
		}
		bool isContaining(const vec3& p)const {
			return (min.x <= p.x && p.x <= max.x) &&
				(min.y <= p.y && p.y <= max.y) &&
				(min.z <= p.z && p.z <= max.z);
		}
		bool isContaining(const AABB& aabb)const {
			return (min.x <= aabb.min.x && aabb.max.x <= max.x) &&
				(min.y <= aabb.min.y && aabb.max.y <= max.y) &&
				(min.z <= aabb.min.z && aabb.max.z <= max.z);
		}
	};

	class Sphere {
	public:
		vec3 center = vec3(0.0f); float radius = 0.0f;
		Sphere() {}
		Sphere(const vec3& center, float radius) :center(center), radius(radius) {}
		Sphere(const vec3& c, const vec3& p) {
			init(c, p);
		}
		Sphere(const AABB& aabb) {
			init(aabb.center(), aabb.min);
		}
		void translate(const vec3& pos) {
			center += pos;
		}
		void scale(float scaling) {
			radius *= scaling;
		}

		void scale(const Scaling& scaling) {
			scale(std::max(std::max(scaling.x, scaling.y), scaling.z));
		}
		void scale_ori(const Scaling& scaling) {
			float s = std::max(std::max(scaling.x, scaling.y), scaling.z);
			radius *= s; center *= s;
		}
		void init(const vec3& c, const vec3& p) {
			center = c;
			radius = length(center - p);
		}
		void operator*=(const mat4& mat) {
			init(vec4(center, 1.0f) * mat, vec4(center.x + radius, center.y, center.z, 1.0f) * mat);
		}
		Sphere operator*(const mat4& mat)const {
			Sphere res;
			res.init(vec4(center, 1.0f) * mat, vec4(center.x + radius, center.y, center.z, 1.0f) * mat);
			return res;
		}
	};

	

	struct Plane {
		vec3 ori, normal;
		Plane() { ori = normal = vec3(0.0f); }
		Plane(const vec3& ori, const vec3& normal) 
			:ori(ori), normal(normal) {}
		float distance(const vec3& pos)const {
			return glm::dot(ori - pos,-normal);
		}
		vec3 perpendicular(const vec3& pos)const {
			return -normal * glm::dot(ori - pos, -normal);
		}
	};

	struct Viewport {
		int x, y, w, h;
		Viewport() { x = y = w = h = 0; }
		template<typename T>
		Viewport(T _w, T _h) :x(0), y(0), w(int(_w)), h(int(_h)) {}
		template<typename T>
		Viewport(T _x, T _y, T _w, T _h) 
			:x(int(_x)), y(int(_y)), w(int(_w)), h(int(_h)) {}
		void apply()const {
			glViewport(x, y, w, h);
		}
		mat4 toMatrix()const {
			float w2 = float(w) / 2.0f, h2 = float(h) / 2.0f;
			return mat4(
				w2, 0.0f, 0.0f, 0.0f,
				0.0f, h2, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				float(x) + w2, float(y) + h2, 0.5f, 1.0f
			);
		}
		vec3 ndc2screen(const vec3& pos)const {
			float w2 = float(w) / 2.0f, h2 = float(h) / 2.0f;
			return vec3(pos.x * w2 + float(x) + w2, pos.y * h2 + float(y) + h2, pos.z * 0.5f + 0.5f);
		}
		vec3 screen2ndc(const vec3& pos)const {
			float w2 = float(w) / 2.0f, h2 = float(h) / 2.0f;
			return vec3((pos.x - float(x) - w2) / w2, (pos.y - float(y) - h2) / h2, (pos.z - 0.5f) * 2.0f);
		}
		vec2 size()const {
			return vec2(float(w), float(h));
		}
		vec2 pos()const {
			return vec2(float(x), float(y));
		}
		void load() {
			glGetIntegerv(GL_VIEWPORT, &x);
		}
	};

	struct Projection {
		float zNear = 0.0f;
		float zFar = 0.0f;
		float left = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;
		float top = 0.0f;
		bool frustum = true;
		Projection() {}
		Projection(float _left, float _right, float _bottom, float _top, float _zNear, float _zFar, bool _frustum = true) :
			left(_left), right(_right), bottom(_bottom), top(_top), zNear(_zNear), zFar(_zFar), frustum(_frustum) {}
		Projection(int w, int h, float _zNear, float _zFar, bool _frustum = true) :zNear(_zNear), zFar(_zFar), frustum(_frustum) {
			float w2 = float(w) * 0.5f, h2 = float(h) * 0.5f;
			left = -w2; right = w2;
			bottom = -h2; top = h2;
		}
		Projection(float fovy, float aspect, float _zNear, float _zFar) {
			perspective(fovy, aspect, _zNear, _zFar);
		}
		Projection(const Viewport& viewport, float fovy, float _zNear, float _zFar) {
			perspective(fovy, float(viewport.w) / float(viewport.h), _zNear, _zFar);
		}
		Projection(const Viewport& viewport, float _zNear, float _zFar) :zNear(_zNear), zFar(_zFar) {
			frustum = false;
			float w2 = float(viewport.w) * 0.5f, h2 = float(viewport.h) * 0.5f;
			left = -w2; right = w2;
			bottom = -h2; top = h2;
		}
		void perspective(float fovy, float aspect, float _zNear, float _zFar) {
			frustum = true;
			zNear = _zNear; 
			zFar = _zFar;
			float h2 = zNear * tanf(glm::radians(fovy * 0.5f)), w2 = h2 * aspect;
			left = -w2; right = w2;
			bottom = -h2; top = h2;
		}
		mat4 toMatrix()const {
			return frustum? glm::frustum(left, right, bottom, top, zNear, zFar) : glm::ortho(left, right, bottom, top, zNear, zFar);
		}
		mat4 operator()()const {
			return toMatrix();
		}
		vec4 view2clip(const vec3& v)const {
			return toMatrix() * vec4(v, 1.0f);
		}
		vec3 view2ndc(const vec3& v)const {
			vec4 t = view2clip(v);
			return vec3(t) / t.w;
		}
		vec3 ndc2view(const vec3& v)const {
			if (frustum) {
				float ze = 2.0f * zFar * zNear / (zFar - zNear) / (v.z - (zFar + zNear) / (zFar - zNear));
				float xe = (-v.x - (right + left) / (right - left)) * ze / (2 * zNear / (right - left));
				float ye = (-v.y - (top + bottom) / (top - bottom)) * ze / (2 * zNear / (top - bottom));
				return vec3(xe, ye, ze);
			}
			return v;
		}

	};

	class Camera {
	public:
		Transformation view2world;//view2world
		EulerAngles rot;
		Projection projection;
		Camera() {}
		Camera(const Projection& _projection): projection(_projection) {}
		mat4 viewMatrix()const {
			return glm::inverse(view2world.toMatrix());
		}
		/*mat4 inversedView()const {
			return translate4(pos) * rot.toMatrix();
		}*/
		mat4 projectionMatrix()const {
			return projection.toMatrix();
		}
		void setProjection(const Projection& _projection) {
			projection = _projection;
		}
		void translate(const vec3& t) {
			view2world.w += t;
		}
		void CalcViewAxes() {
			//view2world.y = rot.toMatrix() * vec4(0.0f, 1.0f, 0.0f, 1.0f);
			float yaw = glm::radians(rot.yaw + 90.0f), pitch = glm::radians(rot.pitch);
			const vec3 front(cosf(yaw) * cosf(pitch), sinf(pitch), -sinf(yaw) * cosf(pitch));
			back() = -front;
			right() = glm::normalize(glm::cross(front, vec3(0.0f, 1.0f, 0.0f)));
			up() = glm::normalize(glm::cross(back(), right()));
		}
		void ApplyRotation() {
			right() = rot * right();
			up() = rot * up();
			back() = rot * back();
		}
		const vec3& position()const {
			return view2world.w;
		}
		vec3& position() {
			return view2world.w;
		}
		vec3 direction(const vec3& dir)const {
			return view2world.applyOrientation(dir);
		}
		const vec3& right()const {
			return view2world.x;
		}
		const vec3& up()const {
			return view2world.y;
		}
		const vec3& front()const {
			return -view2world.z;
		}
		vec3& right() {
			return view2world.x;
		}
		vec3& up() {
			return view2world.y;
		}
		vec3& back() {
			return view2world.z;
		}
		vec3 world2view(const vec3& v)const {
			return viewMatrix() * vec4(v, 1.0f);
		}
		vec4 world2clip(const vec3& v)const {
			return projection.view2clip(world2view(v));
		}
		vec3 view2ndc(const vec3& v)const {
			return projection.view2ndc(v);
		}
		vec3 world2ndc(const vec3& v)const {
			return view2ndc(world2view(v));
		}
		vec3 world2screen(const vec3& v, const Viewport& viewport)const {
			return viewport.ndc2screen(world2ndc(v));
		}
		vec3 ndc2world(const vec3& v)const {
			return glm::inverse(viewMatrix()) * vec4(projection.ndc2view(v),1.0f);
		}
		vec3 screen2world(const vec3& v, const Viewport& viewport)const {
			return ndc2world(viewport.screen2ndc(v));
		}
		Ray cursor2world(const ivec2& cursor, const Viewport& viewport)const {
			vec3 p(cursor.x, viewport.h-cursor.y, 0.0f);
			return Ray(position(), screen2world(p, viewport) - position());
		}
		bool isVisible(const vec3& t)const {
			vec3 v = world2ndc(t);
			return (-1.0f <= v.x && v.x <= 1.0f) &&
				(-1.0f <= v.y && v.y <= 1.0f) &&
				(-1.0f <= v.z && v.z <= 1.0f);
		}
		bool IsVisible(const AABB& aabb)const {
			std::vector<vec3> vertices = aabb.vertices();
			AABB ndcBox; 
			float zmin = INFINITY;
			float zmax = -INFINITY;
			for (const vec3& v : vertices) {
				const vec3 view = world2view(v);
				ndcBox.update(view2ndc(view));
				zmin = std::min(zmin, view.z);
				zmax = std::max(zmax, view.z);
			}
			/*std::cout << "vis " << zmin << " " << zmax << ":"; printvec3(ndcBox.max); printvec3(ndcBox.min);
			if (!(
				(ndcBox.max.x < -1.0f || ndcBox.min.x>1.0f) ||
				(ndcBox.max.y < -1.0f || ndcBox.min.y>1.0f) ||
				(zmax < -projection.zFar || zmin>-projection.zNear)))std::cout << "YES\n";
			else std::cout << "NO\n";
			system("pause");*/
			return !(
				(ndcBox.max.x < -1.0f || ndcBox.min.x>1.0f) ||
				(ndcBox.max.y < -1.0f || ndcBox.min.y>1.0f) ||
				(zmax < -projection.zFar || zmin>-projection.zNear));
		
		}
		void rotate(const vec3& ori,const EulerAngles& eulerAngle) {
			rot += eulerAngle;
			position() = rotateOri(position(), ori, Rotation(eulerAngle));
		}
	};

	inline bool intersect(const Ray& ray, const vec3& v, float* t=0,float eps=0.005f) {
		const vec3 tmp = v - ray.ori;
		float tx = tmp.x / ray.dir.x, ty = tmp.y / ray.dir.y, tz = tmp.z / ray.dir.z;
		if (fabsf(tx - ty) < eps && fabsf(tx - tz) < eps && fabsf(ty-tz)<eps) {
			if (t)*t = (tx+ty+tz)/3.0f;
			return true;
		}
		return false;
	}

	inline bool intersect(const vec3& v, const Triangle& tri) {
		return false;
	}

	inline bool intersect(const vec3& v, const AABB& aabb) {
		return aabb.min.x <= v.x && v.x <= aabb.max.x &&
			aabb.min.y <= v.y && v.y <= aabb.max.y &&
			aabb.min.z <= v.z && v.z <= aabb.max.z;
	}

	inline bool intersect(const AABB& box1,const AABB& box2) {
		return (box1.max.x >= box2.min.x && box2.max.x >= box1.min.x) &&
			(box1.max.y >= box2.min.y && box2.max.y >= box1.min.y) &&
			(box1.max.z >= box2.min.z && box2.max.z >= box1.min.z);
	}

	inline bool intersect(const Ray& ray, const Triangle& tri, float* t=0) {
		const vec3 e1 = tri[1] - tri[0], e2 = tri[2] - tri[0],s=ray.ori-tri[0];
		const vec3 s1 = glm::cross(ray.dir, e2), s2 = glm::cross(s, e1);
		const float c = 1.0f / glm::dot(s1, e1);
		const float _t = c * glm::dot(s2, e2), b1 = c * glm::dot(s1, s), b2 = c * glm::dot(s2, ray.dir);
		if (_t >= 0 && b1 >= 0 && b2 >= 0 && (1 - b1 - b2) >= 0) {
			if (t)*t = _t;
			return true;
		}
		return false;
	}
	inline float intersect(const AABB& aabb, const Ray& ray, ivec3* face = 0) {
		if ((aabb.min.x <= ray.ori.x && ray.ori.x <= aabb.max.x) &&
			(aabb.min.y <= ray.ori.y && ray.ori.y <= aabb.max.y) &&
			(aabb.min.z <= ray.ori.z && ray.ori.z <= aabb.max.z))return 0.0f;
		vec3 _lower = (aabb.min - ray.ori) / ray.dir;
		vec3 _upper = (aabb.max - ray.ori) / ray.dir;
		vec3 lower = min(_lower, _upper);
		vec3 upper = max(_lower, _upper);

		float tmin = 0.0f;
		for (int i = 0; i < 3; i++) {
			if (lower[i] > tmin) {
				tmin = lower[i];
				if (face) {
					*face = ivec3(0);
					(*face)[i] = -sgn(ray.dir[i]);
				}
			}
		}
		float tmax = std::min(upper.x, std::min(upper.y, upper.z));
		if (tmin > tmax)return -1.0f;
		return tmin;
	}

}
