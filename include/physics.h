#pragma once

#include "Festa.hpp"
#include "resources.h"
#include "utils/game.h"
#include <list>

#include "3rd/bullet3/src/btBulletCollisionCommon.h"
#include "3rd/bullet3/src/btBulletDynamicsCommon.h"

namespace Festa {
	template<typename T>
	class Quadtree {
	public:
		typedef std::pair<AABB, T> pair_t;
		typedef typename std::list<pair_t>::const_iterator iterator;
		struct Node {
			std::list<pair_t> models;
			Node* ch[4] = { 0,0,0,0 };
			Node() {}
			void Release() {
				models.clear();
				for (int i = 0; i < 4; i++) {
					if (ch[i]) {
						ch[i]->Release();
						delete ch[i];
						ch[i] = 0;
					}
				}
			}
			void Pop() {
				models.pop_back();
			}
			
		};
		Node root;
		Quadtree() {}
		~Quadtree() {
			Release();
		}
		void Release() {
			Clear();
		}
		void Clear() {
			root.Release();
			box.clear();
		}
		void SetWorldSize(const vec3& size) {
			box.set(vec3(0.0f), size);
		}
		AABB GetBoundingBox()const {
			return box;
		}
		Node* Insert(const AABB& aabb, const T& data, iterator& iter) {
			return Insert_tree(root, iter, box, aabb, data);
		}
		Node* Insert(const AABB& aabb, const T& data) {
			iterator iter;
			return Insert_tree(root, iter, box, aabb, data);
		}
		void GetSubAABB(int i, AABB& ch)const {
			const vec3 center = ch.center();
			switch (i) {
			case 0:
				ch.min.x = center.x, ch.min.z = center.z;
				return;
			case 1:
				ch.min.x = center.x, ch.max.z = center.z;
				return;
			case 2:
				ch.max.x = center.x, ch.min.z = center.z;
				return;
			case 3:
				ch.max.x = center.x, ch.max.z = center.z;
				return;
			}
		}
		void Push(const AABB& aabb, const T& data) {
			root.models.emplace_back(std::make_pair(aabb, data));
		}
		void Build() {
			std::queue<pair_t> q;
			if (box.empty()) {
				for (const auto& i : root.models) {
					q.push(i);
					box.update(i.first);
					//cout << "aabb "; printvec3(i.first.center()); printvec3(i.first.size());
				}
			}
			else {
				for (const auto& i : root.models)
					q.push(i);
			}
			root.models.clear();
			while (q.size()) {
				Insert(q.front().first, q.front().second); q.pop();
			}
		}
		pair_t* FindPlatform(const vec3& pos) {
			pair_t* ret = 0;
			FindPlatform_tree(root, box, pos, ret);
			return ret;
		}
		pair_t* FindPlatform(const AABB& m) {
			pair_t* ret = 0;
			FindPlatform_tree(root, box, m, ret);
			return ret;
		}
		float ylimit(const AABB& m)const {
			float ret = INFINITY;
			_ylimit(root, box, m, ret);
			return ret;
		}
		static bool intersect(const AABB& x, const vec3& pos) {
			return (x.min.x <= pos.x && pos.x <= x.max.x) &&
				(x.min.z <= pos.z && pos.z <= x.max.z);
		}
		static bool intersect(const AABB& x, const AABB& y) {
			return (x.max.x >= y.min.x && y.max.x >= x.min.x) &&
				(x.max.z >= y.min.z && y.max.z >= x.min.z);
		}
	private:
		AABB box;
		Node* Insert_tree(Node& node, iterator& iter, const AABB& bbox, const AABB& model, const T& data) {
			int i = -1;
			const vec3 center = bbox.center();
			AABB ch = bbox;
			if (center.x <= model.min.x) {
				if (center.z <= model.min.z)i = 0, ch.min.x = center.x, ch.min.z = center.z;
				else if (model.max.z <= center.z)i = 1, ch.min.x = center.x, ch.max.z = center.z;
			}
			if (model.max.x <= center.x) {
				if (center.z <= model.min.z)i = 2, ch.max.x = center.x, ch.min.z = center.z;
				else if (model.max.z <= center.z)i = 3, ch.max.x = center.x, ch.max.z = center.z;
			}
			if (i == -1) {
				node.models.emplace_back(std::make_pair(model, data));
				iter = --node.models.end();
				return &node;
			}
			if (!node.ch[i])node.ch[i] = new Node();
			Insert_tree(*node.ch[i], iter, ch, model, data);
		}
		void FindPlatform_tree(Node& node, const AABB& bbox, const vec3& pos, pair_t*& ret) {
			if (!intersect(bbox, pos))return;
			for (pair_t& i : node.models) {
				if (intersect(i.first, pos)) {
					if (!ret || ret->first.max.y < i.first.max.y)ret = &i;
				}
			}
			for (int i = 0; i < 4; i++) {
				if (!node.ch[i])continue;
				AABB sub = bbox; GetSubAABB(i, sub);
				FindPlatform_tree(*node.ch[i], sub, pos, ret);
			}
		}

		void FindPlatform_tree(Node& node, const AABB& bbox, const AABB& m, pair_t*& ret) {
			if (!intersect(bbox, m))return;
			for (pair_t& i : node.models) {
				if (intersect(i.first, m) && (!ret || ret->first.max.y < i.first.max.y))
					ret = &i;
			}
			for (int i = 0; i < 4; i++) {
				if (!node.ch[i])continue;
				AABB sub = bbox; GetSubAABB(i, sub);
				FindPlatform_tree(*node.ch[i], sub, m, ret);
			}
		}

		void _ylimit(const Node& node, const AABB& bbox, const AABB& m, float& ret)const {
			if (!intersect(bbox, m))return;
			for (const pair_t& i : node.models) {
				if (intersect(i.first, m) && i.first.min.y >= m.max.y)
					ret = std::min(ret, i.first.min.y - m.max.y);
			}
			for (int i = 0; i < 4; i++) {
				if (!node.ch[i])continue;
				AABB sub = bbox; GetSubAABB(i, sub);
				_ylimit(*node.ch[i], sub, m, ret);
			}
		}
	};

	inline void Vector_fromBT(const btVector3& from, vec3& to) {
		to.x = from.x();
		to.y = from.y();
		to.z = from.z();
	}
	inline void Vector_toBT(const vec3& from, btVector3& to) {
		to.setX(from.x);
		to.setY(from.y);
		to.setZ(from.z);
	}
	inline void Transformation_fromBT(const btTransform& from, Transformation& to) {
		/*Vector_fromBT(from.getBasis()[0], to.x);
		Vector_fromBT(from.getBasis()[1], to.y);
		Vector_fromBT(from.getBasis()[2], to.z);*/
		/*Vector_fromBT(from.getBasis().getColumn(0), to.x);
		Vector_fromBT(from.getBasis().getColumn(1), to.y);
		Vector_fromBT(from.getBasis().getColumn(2), to.z);
		Vector_fromBT(from.getOrigin(), to.w);*/
		mat4 mat;
		from.getOpenGLMatrix(&mat[0][0]);
		to.loadFrom(mat);
	}
	inline void Transformation_toBT(const Transformation& from, btTransform& to) {
		/*btMatrix3x3 mat;
		Vector_toBT(from.x, mat[0]);
		Vector_toBT(from.y, mat[1]);
		Vector_toBT(from.z, mat[2]);*/
		/*for (int i = 0; i < 3; i++)mat[i][0] = from.x[i];
		for (int i = 0; i < 3; i++)mat[i][1] = from.y[i];
		for (int i = 0; i < 3; i++)mat[i][2] = from.z[i];
		to.setBasis(mat);
		btVector3 ori; Vector_toBT(from.w, ori);
		to.setOrigin(ori);*/
		mat4 mat = from.toMatrix();
		to.setFromOpenGLMatrix(&mat[0][0]);
	}
	struct RigidBody {
		//btCollisionShape* mCollisionShape = 0;
		//btDefaultMotionState* mMotionState = 0;
		btRigidBody* mRigidBody = 0;
		RigidBody() {}
		~RigidBody() {
			//Release();
		}
		void Release() {
			if (!mRigidBody)return;
			btCollisionShape* mCollisionShape = mRigidBody->getCollisionShape();
			btMotionState* mMotionState = mRigidBody->getMotionState();
			if (mCollisionShape) {
				//delete mCollisionShape;
				mCollisionShape = 0;
			}
			if (mMotionState) {
				//delete mMotionState;
				mMotionState = 0;
			}
			delete mRigidBody;
			mRigidBody = 0;
		}
		operator bool()const {
			return mRigidBody;
		}
		void SetTransformation(const Transformation& trans) {
			btTransform t; Transformation_toBT(trans, t);
			mRigidBody->getMotionState()->setWorldTransform(t);
		}
		Transformation GetTransformation()const {
			Transformation ret;
			if (!mRigidBody)return ret;
			btTransform trans; 
			mRigidBody->getMotionState()->getWorldTransform(trans);
			Transformation_fromBT(trans, ret);
			return ret;
		}
		vec3 GetPosition()const {
			vec3 ret; Vector_fromBT(mRigidBody->getWorldTransform().getOrigin(), ret);
			return ret;
		}
		void SetPosition(const vec3& pos) {
			btVector3 ori; Vector_toBT(pos, ori);
			mRigidBody->getWorldTransform().setOrigin(ori);
		}
		vec3 GetVelocity()const {
			vec3 ret; Vector_fromBT(mRigidBody->getLinearVelocity(), ret);
			return ret;
		}
		void SetVelocity(const vec3& vel) {
			btVector3 btVel; Vector_toBT(vel, btVel);
			mRigidBody->setLinearVelocity(btVel);
		}
		void Reshape(btCollisionShape* shape) {
			if (!mRigidBody)return;
			btCollisionShape* old = mRigidBody->getCollisionShape();
			delete old;
			mRigidBody->setCollisionShape(shape);
		}
		void Create(float mass, btCollisionShape* shape, const Transformation& trans) {
			Release();
			btTransform t; Transformation_toBT(trans, t);
			btDefaultMotionState* motionState = new btDefaultMotionState(t);
			btVector3 inertia(0, 0, 0);
			shape->calculateLocalInertia(mass, inertia);
			btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
				mass, motionState, shape, inertia);
			mRigidBody = new btRigidBody(rigidBodyCI);
		}
		void StaticPlane(const vec3& normal, float constant, const Transformation& trans) {
			btVector3 n; Vector_toBT(normal, n);
			Create(0.0f, new btStaticPlaneShape(n, constant), trans);
		}
		void Box(float mass, const vec3& size, const Transformation& trans) {
			btVector3 s; Vector_toBT(size*0.5f, s);
			Create(mass, new btBoxShape(s), trans);
		}
		void Cylinder(float mass, const vec3& size, const Transformation& trans) {
			btVector3 s; Vector_toBT(size * 0.5f, s);
			Create(mass, new btCylinderShape(s), trans);
		}
		void Capsule(float mass, const vec2& size, const Transformation& trans) {
			Create(mass, new btCapsuleShape(size.x, size.y), trans);
		}
		void BvhTriangleMesh(const Mesh& mesh, const Transformation& trans) {
			Create(0.0f, new btBvhTriangleMeshShape(GetTriangleMesh(mesh, trans), true), Transformation());
		}
		static btTriangleMesh* GetTriangleMesh(const Mesh& mesh, const Transformation& trans) {
			btTriangleMesh* ret = new btTriangleMesh();
			for (uint i = 0; i < mesh.numVertices(); i++) {
				btVector3 v; Vector_toBT(trans.apply(mesh.position(i)), v);
				ret->findOrAddVertex(v, false);
			}
			for (uint i = 0; i < mesh.numFaces(); i++) {
				const auto& f = mesh.face(i);
				ret->addTriangleIndices(int(f[0]), int(f[1]), int(f[2]));
			}
			return ret;
		}
		inline void SetRestitution(float restitution) {
			mRigidBody->setRestitution(restitution);
		}
		inline void SetFriction(float friction) {
			mRigidBody->setFriction(friction);
		}
		inline void SetRollingFriction(float friction) {
			mRigidBody->setRollingFriction(friction);
		}
		inline void SetAngularFactor(float f) {
			mRigidBody->setAngularFactor(f);
		}
		inline void SetLinearFactor(const vec3& factor) {
			btVector3 f; Vector_toBT(factor, f);
			mRigidBody->setLinearFactor(f);
		}
		inline void DisableRotation() {
			mRigidBody->setAngularFactor(0.0f);
		}
		inline void SetDamping(float linearDamping, float angularDamping) {
			mRigidBody->setDamping(linearDamping, angularDamping);
		}
		inline void Activate(bool forceActivation) {
			mRigidBody->activate(forceActivation);
		}
		inline void Disable() {
			mRigidBody->setCollisionFlags(mRigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		}
		inline void Enable() {
			mRigidBody->setCollisionFlags(mRigidBody->getCollisionFlags() & ~int(btCollisionObject::CF_NO_CONTACT_RESPONSE));
			mRigidBody->activate(true);
		}
	};

	class PhysicalWorld {
	public:
		PhysicalWorld() {
			Init();
		} 
		~PhysicalWorld() {
			Release();
		}
		void StepSimulation(float deltaTime, int maxSubSteps) {
			mDynamicsWorld->stepSimulation(deltaTime, maxSubSteps);
		}
		void StepSimulation(float deltaTime) {
			mDynamicsWorld->stepSimulation(deltaTime, 1, deltaTime);
		}
		void AddRigidBody(const RigidBody& rigidBody) {
			mDynamicsWorld->addRigidBody(rigidBody.mRigidBody);
		}
		void RemoveRigidBody(const RigidBody& rigidBody) {
			mDynamicsWorld->removeRigidBody(rigidBody.mRigidBody);
		}
		btDiscreteDynamicsWorld* world() {
			return mDynamicsWorld;
		}
		bool CheckIsGrounded(const RigidBody& character, btScalar upThreshold = 0.7f) {
			if (!character)return false;
			const btCollisionObject* body = character.mRigidBody; 
			btVector3 up = btVector3(0, 1, 0);
			int numManifolds = mDynamicsWorld->getDispatcher()->getNumManifolds();
			for (int i = 0; i < numManifolds; i++) {
				btPersistentManifold* contactManifold = mDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
				const btCollisionObject* objA = contactManifold->getBody0();
				const btCollisionObject* objB = contactManifold->getBody1();
				if (objA != body && objB != body)continue;
				int numContacts = contactManifold->getNumContacts();
				for (int j = 0; j < numContacts; j++) {
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					btVector3 normal = pt.m_normalWorldOnB;
					if (objB == body)normal = -normal;
					if (normal.dot(up) > upThreshold) {
						//printf("grounded\n");
						return true;
					}
				}
			}
			return false;
		}
	private:
		btBroadphaseInterface* mBroadphase = 0;
		btDefaultCollisionConfiguration* mCollisionConfiguration = 0;
		btCollisionDispatcher* mDispatcher = 0;
		btSequentialImpulseConstraintSolver* mSolver = 0;
		btDiscreteDynamicsWorld* mDynamicsWorld = 0;
		void Init() {
			mBroadphase = new btDbvtBroadphase();
			mCollisionConfiguration = new btDefaultCollisionConfiguration();
			mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
			mSolver = new btSequentialImpulseConstraintSolver();
			mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
			mDynamicsWorld->setGravity(btVector3(0, -9.81, 0));

			//mDynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawAabb);
		}
		void Release() {
			//delete mBroadphase;
			//delete mCollisionConfiguration;
			//delete mDispatcher;
			//delete mSolver;
			delete mDynamicsWorld;
		}
	};


}

