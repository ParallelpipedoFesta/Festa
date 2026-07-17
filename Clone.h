#pragma once

#include "include/physics.h"
#include "include/fuman.h"
#include "UI.h"

using namespace Festa;

struct PlayerControl {
	vec3 firstPerson = vec3(0.0f);
	vec3 thirdPerson = vec3(0.0f);
	bool usingFirstPerson = true;
	float vel_normal = 0.0f;
	float vel_sprint = 0.0f;
	float vel_crouch = 0.0f;
	float jumpVel0 = 0.0f;
	float jumpVel1 = 0.0f;
	float holdToJumpDuration = 0.0f;
	Timer holdingSpace;

	float throwingVel0 = 0.0f;
	float throwingVel1 = 0.0f;
	float holdToThrowDuration = 0.0f;
	Timer throwing;
	void UpdateJumpingTimer(bool firstFrame, bool pressed) {
		if (firstFrame || !pressed)holdingSpace.reset();
	}
	float GetJumpingVelocity()const {
		return lerpT(jumpVel0, jumpVel1, std::min(float(holdingSpace.interval()) / holdToJumpDuration, 1.0f));
	}
	void UpdateThrowingTimer(bool firstFrame, bool pressed) {
		if (firstFrame || !pressed)throwing.reset();
	}
	float GetThrowingVelocity()const {
		return lerpT(throwingVel0, throwingVel1, std::min(float(throwing.interval()) / holdToThrowDuration, 1.0f));
	}

};



class Clone {
public:
	typedef FumanEngine::Animation Animation;
	RigidBody mBody;
	Clone() {

	}
	void Init(PhysicalWorld& world, const Path& file, float mass) {
		mFuman.Load(file); LoadAnimations();

		AABB aabb = CalcAABB();
		mOffset = vec3(0.0f, -aabb.center().y, 0.0f);
		const vec3 size = aabb.size();
		float radius = std::max(size.x, size.z) * 0.5f;
		float height = std::max(0.0f, size.y - radius * 2.0f);
		mBody.Capsule(mass, vec2(radius, height), Transformation());
		mBody.SetRestitution(0.5f);
		mBody.SetFriction(1.0f);
		mBody.DisableRotation();
		world.AddRigidBody(mBody);
	}
	void Render() {
		Transformation t = GetModelTransformation();
		mFuman.Render(t);
	}
	void Update(float deltaTime) {
		mFuman.Update(deltaTime);
		AABB aabb = CalcAABB();
		mOffset = vec3(0.0f, -aabb.center().y, 0.0f);
	}
	vec3 GetPosition()const {
		return mBody.GetPosition();
	}
	void SetPosition(const vec3& pos) {
		mBody.SetPosition(pos);
	}
	vec3 GetVelocity()const {
		return mBody.GetVelocity();
	}
	void SetVelocity(const vec3& vel) {
		mBody.SetVelocity(vel);
	}
	FumanEngine::Animation* GetAnimation(const std::string& name) {
		return mFuman.GetAnimation(name);
	}
	const FumanEngine::Animation* GetAnimation(const std::string& name)const {
		return mFuman.GetAnimation(name);
	}
	void StandStill() {
		if (mAnimations[0])mAnimations[0]->Deactivate();
		if (mAnimations[1])mAnimations[1]->Deactivate();
	}

	void UpdatePlayer(Window& window, PhysicalWorld& world, GameControl& gameControl, Camera& camera, PlayerControl& movement, bool firstFrame) {
		Update(window.interval());
		bool moved = MoveUsingKeyboard(window, world, gameControl, camera, movement, firstFrame);
		SetView(window, camera, gameControl, movement, moved, firstFrame);
	}

	bool MoveUsingKeyboard(Window& window, PhysicalWorld& world, GameControl& gameControl, Camera& camera, PlayerControl& movement, bool firstFrame) {
		Transformation model = GetModelTransformation();
		const vec2 left(model.x.x, model.x.z);
		const vec2 front(model.z.x, model.z.z);
		bool moved = false;
		vec2 direction = gameControl.InteractToMove(front, left, moved);
		StandStill();

		const ButtonStatus& sprint = gameControl.GetKey(gameControl.mKeysToMove[4]);
		const ButtonStatus& space = gameControl.GetKey(gameControl.mKeysToMove[5]);

		int anim = -1;
		vec3 vel = GetVelocity();
		if (moved) {
			//can be removed
			float v = 0.0f;
			direction = glm::normalize(direction);
			if (sprint) {
				v = movement.vel_sprint;
				anim = 1;
			}
			else {
				v = movement.vel_normal;
				anim = 0;
			}
			if (space) {
				v = movement.vel_crouch;
				//if (anim == 1)anim = 0;
			}
			direction *= v;
			vel.x = direction.x;
			vel.z = direction.y;
		}
		if (space.pressed()) {
			if (mAnimations[2]) {
				mAnimations[2]->Activate();
				if (space.firstPressed())mAnimations[2]->Reset();
			}
		}
		else if (mAnimations[2]) {
			mAnimations[2]->Deactivate();
		}
		bool grounded = world.CheckIsGrounded(mBody);
		if (space.clicked() && grounded) {
			vel.y = movement.GetJumpingVelocity();
			SetJumping();
		}
		movement.UpdateJumpingTimer(firstFrame, space.pressed());

		if (glm::length(vel) > EPS_FLOAT) {
			mBody.Activate(true);
			SetVelocity(vel);
		}
		if (grounded && anim >= 0 && mAnimations[anim])mAnimations[anim]->Activate();

		SetView(window, camera, gameControl, movement, moved, firstFrame);
		return moved;
	}
	void SetView(Window& window, Camera& camera, GameControl& gameControl, PlayerControl& movement, bool moved, bool firstFrame) {
		vec2 delta(0.0f);
		if (!firstFrame) {
			//vec2 delta = (vec2(window.mouse.pos) - vec2(window.size()) * 0.5f) * gameControl.mMouseSensitivity;
			delta = vec2(window.mouse.dir()) * gameControl.mMouseSensitivity;
			if (glm::length(delta) > 0.05f) {
				camera.rot.pitch += delta.y;
				camera.rot.yaw -= delta.x;
			}
		}

		if (gameControl.GetKey(gameControl.mToggleCamera).firstPressed()) {
			movement.usingFirstPerson = !movement.usingFirstPerson;
		}

		ivec2 cursor = window.mouse.pos;
		window.mouse.moveTo(window.width() / 2, window.height() / 2);
		//if (cursor.x<0 || cursor.x>window.width() || cursor.y<0 || cursor.y>window.height())//window.mouse.moveTo(window.width() / 2, window.height() / 2);
		Rotation verticalRotation = Rotation(camera.rot.yaw, vec3(0.0f, 1.0f, 0.0f));
		Transformation trans = Transformation::rotation(Rotation(camera.rot.pitch, vec3(1.0f, 0.0f, 0.0f)) + verticalRotation);
		trans.translate(GetPosition());
		auto headTrans = GetBoneTransformation(FumanEngine::HEAD);
		trans.translate(headTrans.w);
		trans.translate(verticalRotation * (movement.usingFirstPerson ? movement.firstPerson : movement.thirdPerson));
		camera.position() = trans.w;
		camera.right() = -trans.x;
		camera.up() = trans.y;
		camera.back() = -trans.z;

		Transformation bodyTrans = mBody.GetTransformation();
		if (moved) {
			mRotation = verticalRotation;
		}
		bodyTrans.clearOrientation();
	}
	Transformation GetModelTransformation()const {
		//_translate(-mOffset);
		Transformation trans = mBody.GetTransformation();
		trans.setRotation(mRotation);
		trans._translate(mOffset);
		return trans;
	}
	const Transformation& GetBoneTransformation(FumanEngine::BodyPart b)const {
		return mFuman.GetBoneTransformation(b);
	}
	AABB CalcAABB() {
		return mFuman.CalcAABB();
	}
	void SetJumping() {
		if (mAnimations[3]) {
			mAnimations[3]->Activate();
			mAnimations[3]->Reset();
		}
	}
	void SetHolding() {
		if (mAnimations[4] && !mAnimations[4]->isActivated()) {
			mAnimations[4]->Activate();
			mAnimations[4]->Reset();
		}
	}
	void SetNotHolding() {
		if (mAnimations[4])
			mAnimations[4]->Deactivate();
	}
private:
	vec3 mOffset = vec3(0.0f);
	Fuman mFuman;
	//mat3 mBasis = mat3(1.0f);
	Rotation mRotation;
	std::vector<Animation*> mAnimations;

	void LoadAnimations() {
		mAnimations.resize(5);
		mAnimations[0] = GetAnimation("walking");
		mAnimations[1] = GetAnimation("running");
		mAnimations[2] = GetAnimation("crouch");
		mAnimations[3] = GetAnimation("jumping");
		mAnimations[4] = GetAnimation("holding");

		if (mAnimations[0])mAnimations[0]->SetIsLooped(true);
		if (mAnimations[1])mAnimations[1]->SetIsLooped(true);
	}
};