#pragma once
#include "common.h"
#include <cmath>



namespace Festa {
	struct vec2cmp {
		vec2 v = vec2(0.0f);
		vec2cmp() {}
		vec2cmp(const vec2& vec) :v(vec) {}
		bool operator==(const vec2cmp& a)const {
			return EQUAL_FLOAT(v.x, a.v.x) && EQUAL_FLOAT(v.y, a.v.y);
		}
		bool operator<(const vec2cmp& a)const {
			if (!EQUAL_FLOAT(v.x, a.v.x))return v.x < a.v.x;
			if (!EQUAL_FLOAT(v.y, a.v.y))return v.y < a.v.y;
			return false;
		}
	};
	struct vec3cmp {
		vec3 v = vec3(0.0f);
		vec3cmp() {}
		vec3cmp(const vec3& vec) :v(vec) {}
		bool operator==(const vec3cmp& a)const {
			return EQUAL_FLOAT(v.x, a.v.x) && EQUAL_FLOAT(v.y, a.v.y) && EQUAL_FLOAT(v.z, a.v.z);
		}
		bool operator<(const vec3cmp& a)const {
			if (!EQUAL_FLOAT(v.x, a.v.x))return v.x < a.v.x;
			if (!EQUAL_FLOAT(v.y, a.v.y))return v.y < a.v.y;
			if (!EQUAL_FLOAT(v.z, a.v.z))return v.z < a.v.z;
			return false;
		}
	};

	inline quat rotation(float angle, const vec3& axis) {
		const float a = angle / 2.0f, s = sinf(a);
		return quat(cosf(a), axis.x * s, axis.y * s, axis.z * s);
	}

	inline quat RotationBetweenVectors(vec3 start, vec3 dest) {
		start = normalize(start);
		dest = normalize(dest);

		float cosTheta = dot(start, dest);
		vec3 rotationAxis;

		if (cosTheta < -1.0f + EPS_FLOAT) {
			rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
			if (length(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
				rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);

			rotationAxis = normalize(rotationAxis);
			return rotation(PI, rotationAxis);
		}

		rotationAxis = cross(start, dest);

		float s = sqrtf((1 + cosTheta) * 2);
		float invs = 1 / s;

		return quat(
			s * 0.5f,
			rotationAxis.x * invs,
			rotationAxis.y * invs,
			rotationAxis.z * invs
		);

	}

	

	struct Rotation {
		struct EulerAngles {
			float pitch = 0.0f, yaw = 0.0f, roll = 0.0f;
			EulerAngles() {}
			EulerAngles(float _pitch, float _yaw,float _roll) :pitch(_pitch), yaw(_yaw), roll(_roll) {}
			EulerAngles(const Rotation& rot) {
				*this = rot.toEulerAngles();
			}
			void clear() {
				pitch = 0.0f;
				yaw = 0.0f;
				roll = 0.0f;
			}
			mat4 toMatrix()const {
				return glm::eulerAngleXYZ(glm::radians(pitch),glm::radians(yaw),glm::radians(roll));
			}
			vec3 operator*(const vec3& v)const {
				return toMatrix() * vec4(v, 1.0f);
			}
			EulerAngles operator-()const {
				return EulerAngles(-pitch, -yaw,-roll);
			}
			EulerAngles operator+(const EulerAngles& e)const {
				return EulerAngles(pitch + e.pitch, yaw + e.yaw, roll + e.roll);
			}
			void operator+=(const EulerAngles& e) {
				roll += e.roll;
				pitch += e.pitch;
				yaw += e.yaw;
			}
			EulerAngles operator*(float t)const {
				return EulerAngles(t * pitch, t * yaw, t * roll);
			}
		};
		quat rot = quat(1.0f, 0.0f, 0.0f, 0.0f);
		Rotation() {}
		Rotation(const quat& rot) :rot(rot) {}
		Rotation(float angle, const vec3& axis) {
			const float a = glm::radians(angle) * 0.5f, s = sinf(a);
			rot = quat(cosf(a), axis.x * s, axis.y * s, axis.z * s);
		}
		static Rotation radians(float angle, const vec3& axis) {
			const float a = angle * 0.5f, s = sinf(a);
			return quat(cosf(a), axis.x * s, axis.y * s, axis.z * s);
		}
		Rotation(const vec3& from, const vec3& to) {
			rot = RotationBetweenVectors(from, to);
		}
		Rotation(float pitch, float yaw,float roll) {
			rot = (Rotation(roll, VEC3Z) + Rotation(yaw, VEC3Y) + Rotation(pitch, VEC3X)).rot;
		}
		Rotation(const EulerAngles& e) {
			rot = (Rotation(e.roll, VEC3Z) + Rotation(e.yaw, VEC3Y) + Rotation(e.pitch, VEC3X)).rot;
		}
		void clear() {
			rot = quat(1.0f, 0.0f, 0.0f, 0.0f);
		}
		EulerAngles toEulerAngles()const {
			EulerAngles euler;

			// pitch (x-axis rotation)
			float sinr_cosp = 2 * (rot.w * rot.x + rot.y * rot.z);
			float cosr_cosp = 1 - 2 * (rot.x * rot.x + rot.y * rot.y);
			euler.pitch = glm::degrees(atan2f(sinr_cosp, cosr_cosp));

			// yaw (y-axis rotation)
			float sinp = 2 * (rot.w * rot.y - rot.z * rot.x);
			//if (std::abs(sinp) >= 1)
				//euler.y = std::copysignf(PI / 2, sinp); // use 90 degrees if out of range
			//else
			euler.yaw = glm::degrees(asinf(sinp));

			// roll (z-axis rotation)
			float siny_cosp = 2 * (rot.w * rot.z + rot.x * rot.y);
			float cosy_cosp = 1 - 2 * (rot.y * rot.y + rot.z * rot.z);
			euler.roll = glm::degrees(atan2f(siny_cosp, cosy_cosp));

			return euler;
		}
		mat3 ToMat3()const {
			return glm::toMat3(rot);
		}
		mat4 ToMat4()const {
			return glm::toMat4(rot);
		}
		void rotate(const Rotation& other) {
			rot = other.rot * rot;
		}
		Rotation operator+(const Rotation& other)const {
			return Rotation(other.rot * rot);
		}
		void operator+=(const Rotation& other) {
			rotate(other);
		}
		bool operator==(const Rotation& other)const {
			return abs(glm::dot(rot, other.rot) - 1.0f) < EPS_FLOAT;
		}
		Rotation operator-()const {
			return quat(rot.w, -rot.x, -rot.y, -rot.z);
		}
		vec3 operator*(const vec3& v)const {
			return rot * v;
		}
		Rotation operator*(float v)const {
			return Rotation(glm::angle(rot) * v, glm::axis(rot));
		}
		void operator*=(float v) {
			*this = Rotation(glm::angle(rot) * v, glm::axis(rot));
		}
		float getAngle()const {
			return glm::angle(rot);
		}
		vec3 getAxis()const {
			return glm::axis(rot);
		}
	};

	typedef Rotation::EulerAngles EulerAngles;

	struct Scaling {
		float x=1.0f, y=1.0f, z=1.0f;
		Scaling() {}
		Scaling(const vec3& scaling) {
			x = scaling.x; y = scaling.y; z = scaling.z;
		}
		Scaling(float _x, float _y, float _z){
			x = _x; y = _y; z = _z;
		}
		Scaling(float scaling) {
			x = scaling; y = scaling; z = scaling;
		}
		vec3 toVec3()const {
			return vec3(x,y,z);
		}
		operator vec3()const {
			return vec3(x, y, z);
		}
		mat4 toMatrix()const {
			return scale4(vec3(x,y,z));
		}
		void scale(const Scaling& other) {
			x *= other.x; y *= other.y; z *= other.z;
		}
		Scaling operator+(const Scaling& other)const {
			return Scaling(x*other.x,y*other.y,z*other.z);
		}
		void operator+=(const Scaling& other) {
			scale(other);
		}
		bool operator==(const Scaling& other)const {
			return EQUAL_FLOAT(x, other.x) && EQUAL_FLOAT(y, other.y) && EQUAL_FLOAT(z, other.z);
		}
		Scaling operator-()const {
			return vec3(1.0f / x, 1.0f / y, 1.0f / z);
		}
		friend vec3 operator*(const Scaling& s,const vec3& v) {
			return v * s.toVec3();
		}
	};

	struct RigidTransformation {
		vec3 pos=vec3(0.0f);
		Rotation rot;
		RigidTransformation() {}
		RigidTransformation(const vec3& pos, const Rotation& rot) :pos(pos), rot(rot) {}
		void clear() {
			pos = vec3(0.0f);
			rot.clear();
		}
		void translate(const vec3& other) {
			pos += other;
		}
		void rotate(const Rotation& other) {
			rot += other;
		}
		mat4 toMatrix()const {
			return translate4(pos) * rot.ToMat4();
		}
		mat4 inversedMatrix()const {
			return (-rot).ToMat4() * translate4(-pos);
		}
		void operator=(const RigidTransformation& other) {
			pos = other.pos; rot = other.rot;
		}
		void operator+=(const RigidTransformation& other) {
			pos += other.pos; rot += other.rot;
		}
		RigidTransformation operator+(const RigidTransformation& other)const {
			return RigidTransformation(pos + other.pos, rot + other.rot);
		}
		vec3 apply(const vec3& v)const {
			return pos + (rot * v);
		}
		vec3 operator*(const vec3& v)const {
			return apply(v);
		}
		vec3 inverse(const vec3& v)const {
			return (-rot) * (v - pos);
		}
		
	};

	struct Transformation {
		vec3 x = vec3(1.0f, 0.0f, 0.0f);
		vec3 y = vec3(0.0f, 1.0f, 0.0f);
		vec3 z = vec3(0.0f, 0.0f, 1.0f);
		vec3 w = vec3(0.0f, 0.0f, 0.0f);
		Transformation() {}
		Transformation(const mat4& mat) {
			loadFrom(mat);
		}
		Transformation(const Rotation& rot) {
			setRotation(rot);
		}
		Transformation(const RigidTransformation& trans) {
			setPosition(trans.pos);
			setRotation(trans.rot);
		}
		Transformation(const vec3& pos, const Rotation& rot, const vec3& scaling) {
			loadFrom(pos, rot, scaling);
		}
		Transformation(const vec3& _x, const vec3& _y, const vec3& _z, const vec3& _w = vec3(0.0f))
			:x(_x), y(_y), z(_z), w(_w) {}
		void clear() {
			x = vec3(1.0f, 0.0f, 0.0f);
			y = vec3(0.0f, 1.0f, 0.0f);
			z = vec3(0.0f, 0.0f, 1.0f);
			w = vec3(0.0f, 0.0f, 0.0f);
		}
		void clearOrientation() {
			x = vec3(1.0f, 0.0f, 0.0f);
			y = vec3(0.0f, 1.0f, 0.0f);
			z = vec3(0.0f, 0.0f, 1.0f);
		}
		void loadFrom(const Transformation& trans) {
			x = trans.x, y = trans.y, z = trans.z, w = trans.w;
		}
		void loadFrom(const mat4& mat) {
			x = mat[0];
			y = mat[1];
			z = mat[2];
			w = mat[3];
		}
		void loadFrom(const vec3& pos, const Rotation& rot, const vec3& scaling) {
			setPosition(pos);
			setRotation(rot);
			_scale(scaling);
		}
		void normalizeAxes() {
			x = glm::normalize(x);
			y = glm::normalize(y);
			z = glm::normalize(z);
		}
		void print() {
			printf("x "); printvec3(x);
			printf("y "); printvec3(y);
			printf("z "); printvec3(z);
			printf("w "); printvec3(w);
		}
		void setPosition(const vec3& pos) {
			w = pos;
		}
		void _translate(const vec3& t) {
			w = apply(t);
		}
		void translate(const vec3& t) {
			w += t;
		}
		const vec3& getPosition()const {
			return w;
		}
		vec3& getPosition() {
			return w;
		}
		void setRotation(const Rotation& rot) {
			const float qxx(rot.rot.x * rot.rot.x);
			const float qyy(rot.rot.y * rot.rot.y);
			const float qzz(rot.rot.z * rot.rot.z);
			const float qxz(rot.rot.x * rot.rot.z);
			const float qxy(rot.rot.x * rot.rot.y);
			const float qyz(rot.rot.y * rot.rot.z);
			const float qwx(rot.rot.w * rot.rot.x);
			const float qwy(rot.rot.w * rot.rot.y);
			const float qwz(rot.rot.w * rot.rot.z);

			x = vec3(1.0f - 2.0f * (qyy + qzz), 2.0f * (qxy + qwz), 2.0f * (qxz - qwy));
			y = vec3(2.0f * (qxy - qwz), 1.0f - 2.0f * (qxx + qzz), 2.0f * (qyz + qwx));
			z = vec3(2.0f * (qxz + qwy), 2.0f * (qyz - qwx), 1.0f - 2.0f * (qxx + qyy));
		}
		void _rotate(const Rotation& rot) {
			Transformation trans; trans.setRotation(rot);
			_transform(trans);
		}
		void rotate(const Rotation& rot) {
			Transformation trans; trans.setRotation(rot);
			transform(trans);
		}
		void setScale(const vec3& scale) {
			x = vec3(scale.x, 0.0f, 0.0f);
			y = vec3(0.0f, scale.y, 0.0f);
			z = vec3(0.0f, 0.0f, scale.z);
		}
		void _scale(const vec3& scaling) {
			x *= scaling;
			y *= scaling;
			z *= scaling;
		}
		void scale(const vec3& scaling) {
			x *= scaling.x;
			y *= scaling.y;
			z *= scaling.z;
			w *= scaling;
		}
		void scaleAxes(const vec3& scaling) {
			x *= scaling.x;
			y *= scaling.y;
			z *= scaling.z;
		}
		void _transform(const Transformation& trans) {
			x = applyOrientation(trans.x);
			y = applyOrientation(trans.y);
			z = applyOrientation(trans.z);
			w = apply(trans.w);
		}
		void transform(const Transformation& trans) {
			x = trans.applyOrientation(x);
			y = trans.applyOrientation(y);
			z = trans.applyOrientation(z);
			w = trans.apply(w);
		}
		vec3 applyOrientation(const vec3& v)const {
			return v.x * x + v.y * y + v.z * z;
		}
		vec3 apply(const vec3& v)const {
			return v.x * x + v.y * y + v.z * z + w;
		}
		vec3 operator*(const vec3& v)const {
			return apply(v);
		}
		Transformation transpose()const {
			return Transformation(vec3(x.x, y.x, z.x), vec3(x.y, y.y, z.y), vec3(x.z, y.z, z.z));
		}
		Transformation operator*(const Transformation& trans)const {
			return Transformation(
				trans.applyOrientation(x),
				trans.applyOrientation(y),
				trans.applyOrientation(z),
				trans.apply(w)
				);
		}
		void operator*=(const Transformation& trans) {
			transform(trans);
		}
		mat4 toMatrix()const {
			return mat4(vec4(x,0.0f), vec4(y,0.0f), vec4(z,0.0f), vec4(w,1.0f));
		}
		static Transformation translation(const vec3& t) {
			Transformation trans; trans.setPosition(t);
			return trans;
		}
		static Transformation rotation(const Rotation& t) {
			Transformation trans; trans.setRotation(t);
			return trans;
		}
		static Transformation scaling(const vec3& t) {
			Transformation trans; trans.setScale(t);
			return trans;
		}
	};

	

	inline EulerAngles EulerAnglesDegrees(float roll, float pitch, float yaw) {
		return EulerAngles(radians(roll), radians(pitch), radians(yaw));
	}


	inline Rotation slerp(const Rotation& a, const Rotation& b, float t) {
		return glm::slerp(a.rot, b.rot, t);
	}

	inline EulerAngles lerp(const EulerAngles& a, const EulerAngles& b,float t) {
		return EulerAngles(lerpT(a.roll, b.roll, t), lerpT(a.pitch, b.pitch, t), lerpT(a.yaw, b.yaw, t));
	}

	inline vec3 rotateOri(const vec3& x,const vec3& ori,const Rotation& rot) {
		vec3 ret = x - ori;
		ret = rot*ret;
		return ret + ori;
	}
}
