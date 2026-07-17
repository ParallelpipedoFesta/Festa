#pragma once
#include "common/common.h"

#define DEFAULT_VERTEX_ATTRIBUTES "3p3n2t4b4w"

namespace Festa {

	class FumanEngine {
	public:
		typedef enum : uchar {
			NONE,
			HEAD,

			LEFT_EYE,
			RIGHT_EYE,

			LEFT_PUPIL,
			LEFT_INNER_PUPIL,
			EYELID_LU,
			EYELID_LL,

			RIGHT_PUPIL,
			RIGHT_INNER_PUPIL,
			EYELID_RU,
			EYELID_RL,

			NECK,
			UPPER_BODY,
			MIDDLE_BODY,
			LOWER_BODY,

			ARM_LU,
			ARM_LL,
			ARM_RU,
			ARM_RL,
			LEFT_HAND,
			RIGHT_HAND,

			LEG_LU,
			LEG_LL,
			LEG_RU,
			LEG_RL,
			LEFT_FOOT,
			RIGHT_FOOT,
			HAIR, 
			NUM_PARTS
		}BodyPart;
		struct BodyMesh {
			VAO vao;
			Mesh mesh;
			Material material;
			BodyMesh() {}
			void generate() {
				generateVAO();
			}
			void generateVAO() {
				vao.LoadMesh(mesh);
			}
			void load(const Path& file, const vec3& size, const vec3& off) {
				mesh.Load(file);
				Transformation trans;
				trans.setPosition(off); trans.setScale(size);
				mesh.transform(trans);
				generate();
			}
			void sphere(uint n, const vec3& size, const vec3& off) {

				mesh.Sphere(n, n / 2, DEFAULT_VERTEX_ATTRIBUTES);
				Transformation trans;
				trans.setPosition(off); trans.setScale(size);
				mesh.transform(trans);
				generate();
			}
			void cylinder(uint n, const vec2& size, const vec3& off) {
				mesh.Cylinder(n, false, DEFAULT_VERTEX_ATTRIBUTES);
				mesh.RemoveDuplicatedVertices(true);

				Transformation trans;
				trans.setPosition(off); trans.setScale(vec3(size.x, size.y, size.x));
				mesh.transform(trans);
				generate();
			}
			vec3 eye(uint n, float headRadius, float radius, const vec3& p) {
				mesh.Sphere(n, n / 2, DEFAULT_VERTEX_ATTRIBUTES);
				Transformation trans;
				const float theta = glm::radians(p.x), phi = glm::radians(p.y);
				const float r = headRadius * cosf(phi);
				const vec3 centerOff = vec3(r * sinf(theta), headRadius * sinf(phi), r * cosf(theta) - p.z);
				trans.setPosition(centerOff);
				trans.setScale(vec3(radius));
				mesh.transform(trans);
				material.init(MaterialData(vec3(1.0f), vec3(1.0f), vec3(1.0f), 8.0f));
				generate();
				return centerOff;
			}
			vec3 pupil(uint n, float eyeRadius, float pupilRadius, const vec3& pupilColor) {
				mesh.Circle(n, DEFAULT_VERTEX_ATTRIBUTES);
				Transformation trans;
				const vec3 centerOff = vec3(0.0f, 0.0f, eyeRadius);
				trans.setPosition(centerOff);
				trans.setRotation(Rotation(90.0f, VEC3X));
				trans._scale(vec3(pupilRadius));
				mesh.transform(trans);
				material.init(MaterialData(pupilColor, pupilColor, vec3(1.0f), 16.0f));
				generate();
				return centerOff;
			}
			void eyelid(uint n, float eyeRadius, float radius, float angle, bool fliped) {
				mesh.HalfSphere(n, n / 2, DEFAULT_VERTEX_ATTRIBUTES);
				Transformation trans;
				trans.setPosition(vec3(0.0f, 0.0f, eyeRadius-radius));
				trans.setRotation(Rotation(-angle, VEC3X));
				trans._scale(vec3(radius, fliped?-radius:radius, radius));
				mesh.transform(trans);
				generate();
			}
		};
		class FumanAnimation {
		public:
			struct FrameNode {
				float tick = 0.0f;
				vec3 offset = vec3(0.0f);
				Rotation rot;
				vec3 scaling = vec3(1.0f);
			};
			struct BoneNode {
				std::vector<FrameNode> frames;
				int numFrames()const {
					return (int)frames.size();
				}
				float maxTick()const {
					return frames.size() ? frames.back().tick : 0;
				}
				void GetTransformation(Transformation& trans, int& current, float tick) {
					trans.clear();
					if (!numFrames())return;
					while (current + 1 < numFrames() && frames[current + 1].tick <= tick) {
						current++;
					}
					if (current + 1 >= numFrames()) {
						current = numFrames();
						trans.setPosition(frames.back().offset);
						trans.setRotation(frames.back().rot);
						trans._scale(frames.back().scaling);
						return;
					}
					const float t = std::min((tick - frames[current].tick) / (frames[current + 1].tick - frames[current].tick), 1.0f);
					trans.setPosition(lerpT(frames[current].offset, frames[current + 1].offset, t));
					trans.setRotation(slerp(frames[current].rot, frames[current + 1].rot, t));
					trans._scale(lerpT(frames[current].scaling, frames[current + 1].scaling, t));
					return;
				}
			};
			typedef std::vector<FumanEngine::BodyMesh> FumanMeshes;
			std::vector<BoneNode> mBones;
			FumanAnimation() {
				mBones.resize(NUM_PARTS);
			}
			void CalcTransformation(BodyPart b, Transformation& trans, int& current, float tick) {
				return mBones[b].GetTransformation(trans, current, tick);
			}
			float duration()const {
				return mDuration;
			}
			float& duration() {
				return mDuration;
			}
		private:
			float mDuration = 0.0f;
		};

		class Animation {
		public:
			typedef std::vector<FumanEngine::BodyMesh> FumanMeshes;
			Animation() {

			}
			Animation(FumanAnimation* _anim) {
				anim = _anim;
				mCurrentFrames.resize(NUM_PARTS, 0);
			}
			int GetCurrentFrame(BodyPart b)const {
				return mCurrentFrames[b];
			}
			int& GetCurrentFrame(BodyPart b) {
				return mCurrentFrames[b];
			}
			float time()const {
				return mTime;
			}
			float& time() {
				return mTime;
			}
			float duration()const {
				return anim->duration();
			}
			bool isFinished()const {
				return mTime >= duration();
			}
			bool isLooped()const {
				return mLooped;
			}
			void SetIsLooped(bool looped) {
				mLooped = looped;
			}
			bool isPaused()const {
				return mPaused;
			}
			void SetIsPaused(bool paused) {
				mPaused = paused;
			}
			bool isActivated()const {
				return mActivated;
			}
			void Activate() {
				mActivated = true;
			}
			void Deactivate() {
				mActivated = false;
			}
			void Reset() {
				mTime = 0.0f;
				for (int& c : mCurrentFrames)c = 0;
			}
			void SetTime(float time) {
				mTime = time;
				if (mTime > duration()) {
					if (mLooped)Reset();
					else mTime = duration();
				}
			}
			void Update(float deltaTime) {
				if (!mActivated)return;
				if(!mPaused)SetTime(mTime + deltaTime);
			}
			void CalcTransformation(BodyPart b, Transformation& trans) {
				anim->CalcTransformation(b, trans, mCurrentFrames[b], mTime);
			}
			FumanAnimation* baseAnimation()const {
				return anim;
			}
		private:
			std::vector<int> mCurrentFrames;
			float mTime = 0.0f;
			bool mPaused = false;
			bool mLooped = false;
			bool mActivated = false;
			FumanAnimation* anim = 0;
		};

		static FumanEngine instance;
		typedef std::vector<std::vector<BodyPart>> FumanTree;
		Path mFolder;
		std::vector<std::string> mBodyPartNames;
		FumanTree mFumanTree;
		BodyPart mRoot;
		std::vector<BodyPart> mFather;
		std::map<std::string, FumanAnimation> mAnimations;
		JsonData mTypesConfig;
		FumanEngine() {
			//std::cout << "Fuman engine\n";
			mBodyPartNames.resize(NUM_PARTS);
			mBodyPartNames[MIDDLE_BODY] = "middle body";
			mBodyPartNames[UPPER_BODY] = "upper body";
			mBodyPartNames[LOWER_BODY] = "lower body";
			mBodyPartNames[HEAD] = "head";
			mBodyPartNames[LEFT_EYE] = "left eye";
			mBodyPartNames[LEFT_PUPIL] = "left pupil";
			mBodyPartNames[LEFT_INNER_PUPIL] = "left inner pupil";
			mBodyPartNames[RIGHT_EYE] = "right eye";
			mBodyPartNames[RIGHT_PUPIL] = "right pupil";
			mBodyPartNames[RIGHT_INNER_PUPIL] = "right inner pupil";
			mBodyPartNames[NECK] = "neck";

			mBodyPartNames[EYELID_LU] = "eyelid left-upper";
			mBodyPartNames[EYELID_LL] = "eyelid left-lower";
			mBodyPartNames[EYELID_RU] = "eyelid right-upper";
			mBodyPartNames[EYELID_RL] = "eyelid right-lower";

			mBodyPartNames[ARM_LU] = "arm left-upper";
			mBodyPartNames[ARM_LL] = "arm left-lower";
			mBodyPartNames[ARM_RU] = "arm right-upper";
			mBodyPartNames[ARM_RL] = "arm right-lower";
			mBodyPartNames[LEFT_HAND] = "left hand";
			mBodyPartNames[RIGHT_HAND] = "right hand";

			mBodyPartNames[LEG_LU] = "leg left-upper";
			mBodyPartNames[LEG_LL] = "leg left-lower";
			mBodyPartNames[LEG_RU] = "leg right-upper";
			mBodyPartNames[LEG_RL] = "leg right-lower";
			mBodyPartNames[LEFT_FOOT] = "left foot";
			mBodyPartNames[RIGHT_FOOT] = "right foot";

			mBodyPartNames[HAIR] = "hair";

			mRoot = LOWER_BODY;
			mFather.resize(NUM_PARTS, NUM_PARTS);
			mFather[LEFT_PUPIL] = LEFT_EYE;
			mFather[LEFT_INNER_PUPIL] = LEFT_PUPIL;
			mFather[EYELID_LU] = LEFT_EYE;
			mFather[EYELID_LL] = LEFT_EYE;

			mFather[RIGHT_PUPIL] = RIGHT_EYE;
			mFather[RIGHT_INNER_PUPIL] = RIGHT_PUPIL;
			mFather[EYELID_RU] = RIGHT_EYE;
			mFather[EYELID_RL] = RIGHT_EYE;

			mFather[LEFT_EYE] = HEAD;
			mFather[RIGHT_EYE] = HEAD;
			mFather[HEAD] = NECK;
			mFather[NECK] = UPPER_BODY;
			mFather[UPPER_BODY] = MIDDLE_BODY;
			mFather[MIDDLE_BODY] = LOWER_BODY;

			mFather[ARM_LU] = UPPER_BODY;
			mFather[ARM_LL] = ARM_LU;
			mFather[ARM_RU] = UPPER_BODY;
			mFather[ARM_RL] = ARM_RU;
			mFather[LEFT_HAND] = ARM_LL;
			mFather[RIGHT_HAND] = ARM_RL;

			mFather[LEG_LU] = LOWER_BODY;
			mFather[LEG_LL] = LEG_LU;
			mFather[LEG_RU] = LOWER_BODY;
			mFather[LEG_RL] = LEG_RU;
			mFather[LEFT_FOOT] = LEG_LL;
			mFather[RIGHT_FOOT] = LEG_RL;

			mFather[HAIR] = HEAD;

			mFumanTree.resize(NUM_PARTS);
			for (uint i = 0; i < NUM_PARTS; i++) {
				if(mFather[i] < NUM_PARTS)mFumanTree[mFather[i]].push_back((BodyPart)i);
			}

			/*for (uint i = 0; i < NUM_PARTS; i++) {
				std::cout << "father " << mBodyPartNames[i] <<" "<<i << '\n';
				for(BodyPart b:mFumanTree[i])std::cout << "ch " << mBodyPartNames[b] <<" "<<int(b) << '\n';
			}*/
			//exit(0);
			//std::cout << "Fuman engine\n";
		}
		void LoadAnimations(const Path& file) {
			mAnimations.clear();
			File f(file, "rb"); if (f.check())return;
			int num = 0; f >> num;
			while (num--) {
				std::string name; readString(f, name);
				FumanAnimation& animation = mAnimations[name];
				f >> animation.duration();
				for (uint i = 0; i < NUM_PARTS; i++) {
					int numFrames = 0; f >> numFrames;
					auto& bone = animation.mBones[i]; 
					bone.frames.resize(numFrames);
					for (int frame = 0; frame < numFrames; frame++) {
						f >> bone.frames[frame];
					}
				}
			}
		}
		void SaveAnimations(const Path& file) {
			File f(file, "wb"); if (f.check())return;
			f << (int)mAnimations.size();
			for (auto& p : mAnimations) {
				writeString(f, p.first);
				f << p.second.duration();
				for (uint i = 0; i < NUM_PARTS; i++) {
					auto& bone = p.second.mBones[i];
					f << bone.numFrames();
					for (int frame = 0; frame < bone.numFrames(); frame++) {
						f.writeBinaryData(bone.frames[frame]);
					}
				}
			}
		}
		void LoadTypesConfig(const Path& file) {
			mTypesConfig.load(file);
		}
		void Init(const Path& animations, const Path& types) {
			LoadAnimations(animations);
			LoadTypesConfig(types);
		}
		static void Init(const Path& folder) {
			instance.LoadAnimations(folder + Path("animations.fanim"));
			instance.LoadTypesConfig(folder + Path("types.json"));
			instance.mFolder = folder;
		}
		static Path GetPath(const std::string& name) {
			return instance.mFolder + Path(name);
		}
	private:
		static void readString(File& f, std::string& str) {
			int size = 0; f >> size;
			f.read(str, size);
		}
		static void writeString(File& f, const std::string& str) {
			f << (int)str.size();
			f.write(str);
		}
	};

	struct FumanGenerator {
		typedef FumanEngine::BodyPart BodyPart;
		struct BoneNode {
			int boneID = 0;
			float weight = 0.0f;
			bool operator<(const BoneNode& n) {
				return weight > n.weight;
			}
		};
		struct Eye {
			float radius = 0.175f;
			float theta = 24.0f;
			float phi = 11.0f;
			float depth = 0.025f;
			vec2 pupilRadius = vec2(0.087f, 0.057f);
			vec3 pupilColor = vec3(0.2f, 1.0f, 0.98f);
			vec2 eyelid = vec2(35.0f, 40.0f);
			float eyelidScaling = 1.0857f;
		};
		struct Limb {
			vec3 position = vec3(0.0f);
			vec2 size = vec2(0.0f);
			float offset = 0.0f;
			float upperRatio = 0.0f;
			float upperLength()const {
				return size.y * upperRatio;
			}
			float lowerLength()const {
				return size.y * (1.0f - upperRatio);
			}
		};
		std::string mName = "Not Festa";
		int mGender = 0;//f0 m1 n2
		int mSampleCount = 30;

		vec4 mSkinColor = vec4(1.0f, 0.9f, 0.7f, 8.0f);
		vec4 mHairColor = vec4(0.3f, 0.2f, 0.1f, 8.0f);

		int mBodyType = 0;
		int mSuitType = 0;
		int mHairType = 0;

		vec4 mHair = vec4(0.0f);
		float mHeadRadius = 0.6f;
		vec3 mNeck = vec3(0.2f, 0.6f, 0.1f);

		Eye mLeftEye;
		Eye mRightEye;

		vec3 mBodySize = vec3(0.45f, 1.0f, 0.4f);
		float mUpperBodyPosition = 0.5f;
		float mMiddleBodyPosition = 0.25f;

		Limb mArms = {
			vec3(0.4f, 1.4f, 0.0f),
			vec2(0.15f, 0.9f),
			0.0f,
			0.5f
		};
		Limb mLegs = {
			vec3(0.2f, 0.33f, 0.0f),
			vec2(0.2f, 1.0f),
			0.0f,
			0.5f
		};
		vec2 mHands = vec2(0.15f, 0.1f);

		Material skin;
		Material hair;

		std::vector<vec3> mCenters;
		std::vector<vec3> mOffsets;
		std::vector<FumanEngine::BodyMesh> mMeshes;

		FumanGenerator() {
			mMeshes.resize(FumanEngine::NUM_PARTS);
			mOffsets.resize(FumanEngine::NUM_PARTS);
			mCenters.resize(FumanEngine::NUM_PARTS);
		}
		void Load(const Path& path) {
			File f(path, "rb"); if (f.check())return;
			int size = 0; f >> size; mName.resize(size); f.read(mName, size);
			f >> mGender >> mSampleCount >>mSkinColor>>mHairColor;
			f >> mBodyType >> mSuitType >> mHairType;
			f >> mHair >> mHeadRadius >> mNeck;
			f >> mLeftEye >> mRightEye;
			f >> mBodySize >> mUpperBodyPosition >> mMiddleBodyPosition;
			f.close();
		}
		void Save(const Path& path) {
			File f(path, "wb"); if (f.check())return;
			f << int(mName.size()); f.write(mName);
			f << mGender << mSampleCount <<mSkinColor<<mHairColor;
			f << mBodyType << mSuitType << mHairType;
			f << mHair << mHeadRadius << mNeck;
			f << mLeftEye << mRightEye;
			f << mBodySize << mUpperBodyPosition << mMiddleBodyPosition;
			f << mArms << mLegs << mHands;
			f.close();
		}
		void Generate() {
			InitMaterials();
			if(mBodyType>=0)LoadBody(FumanEngine::instance.mTypesConfig["body"][mGender][mBodyType], true);
			else {

			}
			ConstructLimbs();
			ConstructHead();
			ConstructEyes();
			if(mSuitType>=0)LoadSuit(FumanEngine::instance.mTypesConfig["suits"][mSuitType]);
			else {
				Naked();
			}
			if(mHairType>=0)LoadHair(FumanEngine::instance.mTypesConfig["hair"][mHairType], true);
			else {

			}
		}
		void LoadBody(const JsonData& j, bool loadParams) {
			if (loadParams) {
				const JsonData& bodyParams = j["body"];
				mBodySize.x = bodyParams[0].to<float>();
				mBodySize.y = bodyParams[1].to<float>();
				mBodySize.z = bodyParams[2].to<float>();
				mUpperBodyPosition = bodyParams[3].to<float>();
				mMiddleBodyPosition = bodyParams[4].to<float>();
				mNeck.z = bodyParams[5].to<float>();
				const JsonData& armParams = j["arms"];
				mArms.position.x = armParams[0].to<float>();
				mArms.position.y = armParams[1].to<float>();
				mArms.position.z = armParams[2].to<float>();
				mArms.offset = armParams[3].to<float>();
				mArms.size.x = armParams[4].to<float>();
				mArms.size.y = armParams[5].to<float>();
				const JsonData& legParams = j["legs"];
				mLegs.position.x = legParams[0].to<float>();
				mLegs.position.y = legParams[1].to<float>();
				mLegs.position.z = legParams[2].to<float>();
				mLegs.offset = legParams[3].to<float>();
				mLegs.size.x = legParams[4].to<float>();
				mLegs.size.y = legParams[5].to<float>();
			}

			mMeshes[FumanEngine::LOWER_BODY].load(FumanEngine::GetPath("meshes") + Path(j["file"]), mBodySize, vec3(0.0f, 0.0f, 0.0f));
			mOffsets[FumanEngine::MIDDLE_BODY] = vec3(0.0f, mMiddleBodyPosition, 0.0f);
			mOffsets[FumanEngine::UPPER_BODY] = vec3(0.0f, mUpperBodyPosition, 0.0f);

			AssignWeights(FumanEngine::LOWER_BODY, { FumanEngine::UPPER_BODY,  FumanEngine::MIDDLE_BODY });
		}
		void LoadHair(const JsonData& j, bool loadParams) {
			if (loadParams) {
				const JsonData& p = j["params"];
				mHair.x = p[0].to<float>();
				mHair.y = p[1].to<float>();
				mHair.z = p[2].to<float>();
				mHair.w = p[3].to<float>();
			}
			mMeshes[FumanEngine::HAIR].material = hair;
			const vec3 hairSize = mHair * mHeadRadius;
			mMeshes[FumanEngine::HAIR].load(FumanEngine::GetPath("meshes") + Path(j["file"]), hairSize, vec3(0.0f, -hairSize.y * 0.5f, 0.0f));
			mOffsets[FumanEngine::HAIR] = GetCenter(FumanEngine::HEAD) + vec3(0.0f, mHeadRadius * (-mHair.w - 1.0f), 0.0f);
			InitWeights(FumanEngine::HAIR);
		}
		void ConstructLimbs() {
			vec3 legTop = mLegs.position;
			mMeshes[FumanEngine::LEG_RU].load(FumanEngine::GetPath("meshes/leg1.fmesh"), vec3(mLegs.size.x, mLegs.size.y, mLegs.size.x), vec3(0.0f, mLegs.offset - mLegs.size.y, 0.0f));
			mOffsets[FumanEngine::LEG_RU] = legTop;
			mOffsets[FumanEngine::LEG_RL] = legTop + vec3(0.0f, -mLegs.upperLength(), 0.0f);
			legTop.x = -legTop.x;
			mMeshes[FumanEngine::LEG_LU].load(FumanEngine::GetPath("meshes/leg1.fmesh"), vec3(mLegs.size.x, mLegs.size.y, mLegs.size.x), vec3(0.0f, mLegs.offset - mLegs.size.y, 0.0f));
			mOffsets[FumanEngine::LEG_LU] = legTop;
			mOffsets[FumanEngine::LEG_LL] = legTop + vec3(0.0f, -mLegs.upperLength(), 0.0f);

			vec3 armTop = mArms.position;
			mMeshes[FumanEngine::ARM_RU].load(FumanEngine::GetPath("meshes/arm0.fmesh"), vec3(mArms.size.x, mArms.size.y, mArms.size.x), vec3(0.0f, mArms.offset - mArms.size.y, 0.0f));
			mCenters[FumanEngine::ARM_RU] = vec3(0.0f, -mArms.upperLength() * 0.5f, 0.0f);
			mOffsets[FumanEngine::ARM_RU] = armTop;
			mCenters[FumanEngine::ARM_RL] = vec3(0.0f, -mArms.lowerLength() * 0.5f, 0.0f);
			mOffsets[FumanEngine::ARM_RL] = armTop + vec3(0.0f, -mArms.upperLength() * 0.5f, 0.0f);
			mMeshes[FumanEngine::RIGHT_HAND].sphere(mSampleCount, vec3(mHands.x), vec3(0.0f));
			mOffsets[FumanEngine::RIGHT_HAND] = armTop + vec3(0.0f, -mArms.size.y - mHands.y, 0.0f);

			armTop.x = -armTop.x;
			mMeshes[FumanEngine::ARM_LU].load(FumanEngine::GetPath("meshes/arm0.fmesh"), vec3(mArms.size.x, mArms.size.y, mArms.size.x), vec3(0.0f, mArms.offset - mArms.size.y, 0.0f));
			mCenters[FumanEngine::ARM_LU] = vec3(0.0f, -mArms.upperLength() * 0.5f, 0.0f);
			mOffsets[FumanEngine::ARM_LU] = armTop;
			mCenters[FumanEngine::ARM_LL] = vec3(0.0f, -mArms.lowerLength() * 0.5f, 0.0f);
			mOffsets[FumanEngine::ARM_LL] = armTop + vec3(0.0f, -mArms.upperLength() * 0.5f, 0.0f);
			mMeshes[FumanEngine::LEFT_HAND].sphere(mSampleCount, vec3(mHands.x), vec3(0.0f));
			mOffsets[FumanEngine::LEFT_HAND] = armTop + vec3(0.0f, -mArms.size.y - mHands.y, 0.0f);

			AssignWeights(FumanEngine::ARM_LU, { FumanEngine::ARM_LL });
			AssignWeights(FumanEngine::ARM_RU, { FumanEngine::ARM_RL });
			AssignWeights(FumanEngine::LEG_LU, { FumanEngine::LEG_LL });
			AssignWeights(FumanEngine::LEG_RU, { FumanEngine::LEG_RL });

			InitWeights(FumanEngine::LEFT_HAND);
			InitWeights(FumanEngine::RIGHT_HAND);
		}
		void ConstructHead() {
			const vec3 neckBottom = vec3(0.0f, mBodySize.y + mNeck.z, 0.0f);
			mMeshes[FumanEngine::NECK].cylinder(mSampleCount, mNeck, vec3(0.0f));
			mOffsets[FumanEngine::NECK] = neckBottom;

			const vec3 headBottom = neckBottom + vec3(0.0f, mNeck.y * 0.5f, 0.0f);
			mMeshes[FumanEngine::HEAD].sphere(mSampleCount, vec3(mHeadRadius), vec3(0.0f, mHeadRadius, 0.0f));
			mOffsets[FumanEngine::HEAD] = headBottom;
			mCenters[FumanEngine::HEAD] = vec3(0.0f, mHeadRadius, 0.0f);

			InitWeights(FumanEngine::HEAD);
			InitWeights(FumanEngine::NECK);
		}
		void ConstructEyes() {
			const vec3 headCenter = GetCenter(FumanEngine::HEAD);
			const float eyelidOff = 0.021f;
			mCenters[FumanEngine::LEFT_EYE] = mMeshes[FumanEngine::LEFT_EYE].eye(mSampleCount, mHeadRadius,
				mLeftEye.radius, vec3(-mLeftEye.theta, mLeftEye.phi, mLeftEye.depth));
			mOffsets[FumanEngine::LEFT_EYE] = headCenter;
			vec3 eyeCenter = GetCenter(FumanEngine::LEFT_EYE);
			mMeshes[FumanEngine::LEFT_PUPIL].pupil(mSampleCount, mLeftEye.radius - 0.002f, mLeftEye.pupilRadius.x, mLeftEye.pupilColor);
			mMeshes[FumanEngine::LEFT_INNER_PUPIL].pupil(mSampleCount, mLeftEye.radius, mLeftEye.pupilRadius.y, vec3(0.0f));
			float eyelidRadius = mLeftEye.radius * mLeftEye.eyelidScaling;
			mMeshes[FumanEngine::EYELID_LU].eyelid(mSampleCount, mLeftEye.radius + eyelidOff, eyelidRadius, mLeftEye.eyelid.x, false);
			mMeshes[FumanEngine::EYELID_LL].eyelid(mSampleCount, mLeftEye.radius + eyelidOff, eyelidRadius, mLeftEye.eyelid.y, true);
			for (uint i = FumanEngine::LEFT_PUPIL; i <= FumanEngine::EYELID_LL; i++)mOffsets[i] = eyeCenter;

			mCenters[FumanEngine::RIGHT_EYE] = mMeshes[FumanEngine::RIGHT_EYE].eye(mSampleCount, mHeadRadius,
				mRightEye.radius, vec3(mRightEye.theta, mRightEye.phi, mRightEye.depth));
			mOffsets[FumanEngine::RIGHT_EYE] = headCenter;
			eyeCenter = GetCenter(FumanEngine::RIGHT_EYE);
			mMeshes[FumanEngine::RIGHT_PUPIL].pupil(mSampleCount, mRightEye.radius - 0.002f, mRightEye.pupilRadius.x, mRightEye.pupilColor);
			mMeshes[FumanEngine::RIGHT_INNER_PUPIL].pupil(mSampleCount, mRightEye.radius, mRightEye.pupilRadius.y, vec3(0.0f));
			eyelidRadius = mRightEye.radius * mRightEye.eyelidScaling;
			mMeshes[FumanEngine::EYELID_RU].eyelid(mSampleCount, mRightEye.radius + eyelidOff, eyelidRadius, mRightEye.eyelid.x, false);
			mMeshes[FumanEngine::EYELID_RL].eyelid(mSampleCount, mRightEye.radius + eyelidOff, eyelidRadius, mRightEye.eyelid.y, true);
			for (uint i = FumanEngine::RIGHT_PUPIL; i <= FumanEngine::EYELID_RL; i++)mOffsets[i] = eyeCenter;

			for (uint i = FumanEngine::LEFT_EYE; i <= FumanEngine::EYELID_RL; i++)InitWeights((BodyPart)i);
		}
		void Naked() {
			mSuitType = -1;
			mMeshes[FumanEngine::LOWER_BODY].material = skin;
			mMeshes[FumanEngine::ARM_LU].material = skin;
			mMeshes[FumanEngine::ARM_RU].material = skin;
			mMeshes[FumanEngine::LEG_LU].material = skin;
			mMeshes[FumanEngine::LEG_RU].material = skin;
		}
		void Bald() {
			mHairType = -1;
			mMeshes[FumanEngine::HAIR].vao.Release();
		}
		void LoadSuit(const JsonData& j) {
			const Path folder = FumanEngine::GetPath("suits") + Path(j["folder"]);
			const float shininess = j["shininess"].to<float>();
			std::vector<std::pair<BodyPart, std::string>> parts = {
				{FumanEngine::LOWER_BODY,"body.jpg"},
				{ FumanEngine::ARM_LU,"arm-left.jpg" },
				{ FumanEngine::ARM_RU,"arm-right.jpg" },
				{ FumanEngine::LEG_LU,"leg-left.jpg" } ,
				{ FumanEngine::LEG_RU,"leg-right.jpg" }
			};
			for (const auto& p : parts) {
				BodyPart i = p.first;
				mMeshes[i].material.specular = vec3(1.0f);
				mMeshes[i].material.shininess = shininess;
				mMeshes[i].material.useDiffuseMap();
				Image img(folder + Path(p.second));
				ProcessImage(img);
				mMeshes[i].material.diffuseMap.Generate(img);
			}
		}

		void InitMaterials() {
			skin.init(MaterialData(mSkinColor, mSkinColor, vec3(1.0f), mSkinColor.w));
			hair.init(MaterialData(mHairColor, mHairColor, vec3(1.0f), mHairColor.w));
			mMeshes[FumanEngine::NECK].material = skin;
			mMeshes[FumanEngine::HEAD].material = skin;
			mMeshes[FumanEngine::EYELID_LU].material = skin;
			mMeshes[FumanEngine::EYELID_LL].material = skin;
			mMeshes[FumanEngine::EYELID_RU].material = skin;
			mMeshes[FumanEngine::EYELID_RL].material = skin;
			mMeshes[FumanEngine::LEFT_HAND].material = skin;
			mMeshes[FumanEngine::RIGHT_HAND].material = skin;

			mMeshes[FumanEngine::HAIR].material = hair;
		}

		void Construct() {
			InitMaterials();
			mMeshes[FumanEngine::LOWER_BODY].sphere(mSampleCount, mBodySize, vec3(0.0f, mBodySize.y, 0.0f));
			mOffsets[FumanEngine::MIDDLE_BODY] = vec3(0.0f, mMiddleBodyPosition, 0.0f);
			mOffsets[FumanEngine::UPPER_BODY] = vec3(0.0f, mUpperBodyPosition, 0.0f);
			AssignWeights(FumanEngine::LOWER_BODY, { FumanEngine::UPPER_BODY,  FumanEngine::MIDDLE_BODY });
			ConstructLimbs();
			ConstructHead();
			ConstructEyes();
			Naked();
		}

	private:
		void ProcessImage(Image& im) {
			for (int x = 0; x < im.width(); x++) {
				for (int y = 0; y < im.height(); y++) {
					Color col = im.GetColor(x, y);
					if (col.r > 250 && col.g > 250 && col.b > 250)
						im.SetColor(x, y, mSkinColor);
				}
			}
		}
		vec3 GetCenter(BodyPart b) {
			return mOffsets[b] + mCenters[b];
		}
		void InitWeights(BodyPart i) {
			std::vector<BoneNode> nodes(1 + FumanEngine::instance.mFumanTree[i].size());
			nodes[0].boneID = FumanEngine::instance.mFather[i]; uint idx = 1;
			for (BodyPart b : FumanEngine::instance.mFumanTree[i])nodes[idx++].boneID = b;

			for (uint v = 0; v < mMeshes[i].mesh.numVertices(); v++) {
				mMeshes[i].mesh.boneIDs(v) = vec4(float(i), -1.0f, -1.0f, -1.0f);
				mMeshes[i].mesh.boneWeights(v) = vec4(1.0f, 0.0f, 0.0f, 0.0f);
			}
			mMeshes[i].generateVAO();
		}
		void AssignWeights(BodyPart b, const std::vector<BodyPart>& ch) {
			Mesh& mesh = mMeshes[b].mesh;
			const float h0 = GetCenter(b).y;
			std::vector<BoneNode> nodes(ch.size() + 1);
			for (uint i = 0; i < ch.size(); i++)nodes[i + 1].boneID = ch[i];
			nodes[0].boneID = b;

			//cout << "positions "<<FumanEngine::instance.mBodyPartNames[b]<<endl;
			for (uint j = 0; j < std::min(uint(nodes.size()), 4u); j++) {
				//printvec3(mMeshes[nodes[j].boneID].center());

			}

			for (uint v = 0; v < mesh.numVertices(); v++) {
				vec4 boneIDs(float(b), -1.0f, -1.0f, -1.0f);
				vec4 boneWeights(1.0f, 0.0f, 0.0f, 0.0f);
				for (uint j = 0; j < std::min(uint(nodes.size()), 4u); j++) {
					boneIDs[j] = float(nodes[j].boneID);
					const float dist = glm::length(mesh.position(v) + mOffsets[b] - GetCenter((BodyPart)nodes[j].boneID));
					boneWeights[j] = SmoothedWeight(dist);
				}
				boneWeights = glm::normalize(boneWeights);
				float mx = std::max(std::max(boneWeights.x, boneWeights.y), std::max(boneWeights.z, boneWeights.w));
				if (mx > 0.5f)
					for (uint j = 0; j < 4; j++) {
						if (boneWeights[j] < mx || mx < 0.0f)boneWeights[j] = 0.0f;
						else boneWeights[j] = 1.0f, mx = -1.0f;
					}
				mesh.boneIDs(v) = boneIDs;
				mesh.boneWeights(v) = boneWeights;
			}
			mMeshes[b].generateVAO();
		}

		static float SmoothedWeight(float dist) {
			return std::min(expf(-dist), 1.0f);
		}
		static float CalcDistance(const vec3& pos, const AABB& aabb) {
			vec3 a = aabb.max - pos;
			vec3 b = pos - aabb.min;
			vec3 c(0.0f);
			if (a.x < 0.0f || b.x < 0.0f)c.x = std::min(fabsf(a.x), fabsf(b.x));
			if (a.y < 0.0f || b.y < 0.0f)c.y = std::min(fabsf(a.y), fabsf(b.y));
			if (a.z < 0.0f || b.z < 0.0f)c.z = std::min(fabsf(a.z), fabsf(b.z));
			return glm::length(c);
		}
	};

	class Fuman {
	public:
		typedef FumanEngine::BodyPart BodyPart;
		Fuman() {}
		Fuman(const Path& file) {
			Load(file);
		}
		void Load(const Path& file) {
			FumanGenerator g; 
			g.Load(file); g.Generate();
			LoadFromGenerator(g);
			LoadAnimations();
		}
		void LoadFromGenerator(FumanGenerator& g) {
			mBones.resize(FumanEngine::NUM_PARTS);
			for (int i = 0; i < g.mOffsets.size(); i++) {
				mBones[i].offset = g.mOffsets[i];// -g.mOffsets[FumanEngine::instance.mFather[i]];
			}
			mMeshes.resize(g.mMeshes.size());
			for (int i = 0; i < g.mMeshes.size(); i++) {
				g.mMeshes[i].material.Transfer(mMeshes[i].material);
				g.mMeshes[i].vao.Transfer(mMeshes[i].vao);
				PrecomputeAABB(g.mMeshes[i].mesh);
			}
		}
		void Update(float deltaTime) {
			UpdateAnimations(deltaTime);
			UpdateBones();
		}
		void Render(const Transformation& trans) {
			GLSLProgram::activatedProgram->set("model", trans.toMatrix());
			for (uint i = 0; i < FumanEngine::NUM_PARTS; i++) {
				GLSLProgram::activatedProgram->set("boneMatrices[" + toString(i) + "]", mBones[i].trans.toMatrix());
			}
			for (auto& mesh : mMeshes) {
				mesh.material.bind(*GLSLProgram::activatedProgram, "material");
				mesh.vao.Draw();
			}
		}
		void LoadAnimations() {
			mAnimations.clear();
			for (auto& p : FumanEngine::instance.mAnimations) {
				mAnimations[p.first] = FumanEngine::Animation(&p.second);
			}
		}
		FumanEngine::Animation* GetAnimation(const std::string& name) {
			return mAnimations.find(name) == mAnimations.end() ? 0 : &mAnimations[name];
		}
		const FumanEngine::Animation* GetAnimation(const std::string& name)const {
			return mAnimations.find(name) == mAnimations.end() ? 0 : &(((Fuman*)this)->mAnimations)[name];
		}
		void DeactivateAllAnimations() {
			for (auto& it : mAnimations) {
				it.second.Deactivate();
			}
		}
		AABB CalcAABB()const {
			AABB ret;
			for (const _Bone& bone : mBones)
				if(bone.aabb)
				ret.update(bone.GetAABB());
			return ret;
		}
		const Transformation& GetBoneTransformation(FumanEngine::BodyPart b)const {
			return mBones[b].trans;
		}
		Transformation& GetBoneTransformation(FumanEngine::BodyPart b) {
			return mBones[b].trans;
		}


		void UpdateAnimations(float deltaTime) {
			for (auto& p : mAnimations) {
				p.second.Update(deltaTime);
			}
		}
		void UpdateBones() {
			CalculateBones();
			ApplyBones();
		}
		void CalculateBones() {
			ResetBones();
			for (uint i = 0; i < FumanEngine::NUM_PARTS; i++) {
				for (auto& p : mAnimations) {
					if (p.second.isActivated()) {
						Transformation t;
						p.second.CalcTransformation((BodyPart)i, t);
						mBones[i].trans.transform(t);
					}
				}
			}
		}
		void ResetBones() {
			for (uint i = 0; i < FumanEngine::NUM_PARTS; i++) {
				mBones[i].trans.clear();
			}
		}
		void ApplyBones() {
			CalcTransformations(FumanEngine::instance.mRoot, Transformation(), vec3(0.0f), vec3(0.0f));
		}
	protected:
		struct _Bone {
			vec3 offset = vec3(0.0f);
			Transformation trans;
			AABB aabb;
			AABB GetAABB()const {
				return (aabb - offset) * trans;
			}
		};
		struct _Mesh {
			VAO vao;
			Material material;
		};
		//std::vector<vec3> mOffsets;
		std::vector<_Mesh> mMeshes;
		std::vector<_Bone> mBones;
		std::map<std::string, FumanEngine::Animation> mAnimations;

		void PrecomputeAABB(const Mesh& mesh, float threshold = 0.8f) {
			for (uint i = 0; i < mesh.numVertices(); i++) {
				const vec3& pos = mesh.position(i);
				const vec4& boneIDs = mesh.boneIDs(i);
				const vec4& boneWeights = mesh.boneWeights(i);
				for (int j = 0; j < 4; j++) {
					if (boneWeights[i] > threshold && 0 <= boneIDs[i] && boneIDs[i] < mBones.size())
						mBones[boneIDs[i]].aabb.update(pos);
				}
			}
		}

		void CalcTransformations(FumanEngine::BodyPart b, const Transformation& father, const vec3& fatherOff, const vec3& fatherPos) {
			Transformation global = mBones[b].trans;
			global.transform(father);
			mBones[b].trans = global;
			const vec3 off = fatherOff + father.applyOrientation(mBones[b].offset-fatherPos);
			mBones[b].trans.translate(off);
			for (FumanEngine::BodyPart ch : FumanEngine::instance.mFumanTree[b]) {
				CalcTransformations(ch, global, off, mBones[b].offset);
			}
		}
	};

}