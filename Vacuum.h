#pragma once

#include "Festa.hpp"
#include "include/utils/game.h"
#include "include/physics.h"
#include "App.h"
#include "UI.h"
#include "include/utils/gui.h"
#include "quantum_basic.h"

using namespace Festa;

#define CAMERA_Y_MAX 20.0f
#define CAMERA_Y_MIN 4.0f
#define GRID_DIM 1.5f

inline vec3 grid2world(const ivec2& pos) {
	return vec3(float(pos.x) * GRID_DIM, 0.0f, float(pos.y) * GRID_DIM);
}


#define GATESTYLE_BLUE 0b00
#define GATESTYLE_RED 0b01
#define GATESTYLE_GREEN 0b10
#define GATESTYLE_YELLOW 0b11
#define GATESTYLE_SQUARE 0b0000
#define GATESTYLE_DISC 0b0100
#define GATESTYLE_SPHERE 0b1000
#define GATESTYLE_SPHERE 0b1100

#define GATESTYLE_PROCESS 0b000000
#define GATESTYLE_INPUT   0b010000
#define GATESTYLE_MEASURE 0b100000
#define GATESTYLE_OUTPUT  0b110000

#define GATESTYLE_BLOCHSPHERE 0b01000000

typedef enum {
	GATE_0,
	GATE_1,
	GATE_INPUT,
	GATE_OUTPUT,
	GATE_MEASURE,
	GATE_BLOCHSPHERE,
	GATE_X,
	GATE_Y,
	GATE_Z,
	GATE_HADAMARD,
	GATE_CNOT,
	GATE_PERFORMER,
	GATE_ORACLE,
	NUMGATES

}GateID;

struct QuantumGate {
	typedef void(*gate_operate_fn)(QuantumEntanglement::QuantumCalculator&, const std::vector<complex>& parameters);
	typedef enum : uchar {
		PARAM_REAL,
		PARAM_COMPLEX,
		PARAM_BIT,
	}ParameterType;
	struct Parameter {
		std::string name;
		ParameterType type;
		uchar io = 0;// 0:none 1: input 2: output
	};

	int style = 0;
	uint numJoints = 0;
	std::string text;
	std::string name;

	ComplexMatrix matrix;
	gate_operate_fn operate_fn = 0;
	std::vector<Parameter> parameters;
	Qubit value;

	Text3D* label = 0;
	static TextureMap iconMap;
	vec2 off = vec2(0.0f);
	~QuantumGate() {
		if (label) {
			delete label;
			label = 0;
		}
	}
	bool isInput()const {
		return (style & 0b110000) == GATESTYLE_INPUT;
	}
	void generateLabel(bool regenerate = false) {
		if (!text.size())return;
		if (!label)label = new Text3D();
		if (label->texture.empty()||regenerate)label->GenerateTexture(text, vec2(1.0f));
	}
	void createIcon();
	bool renderButton(float x, float y, const vec2& size, bool& hovered);
};

struct GateInstance {
	ivec2 origin;
	std::string text;
	Text3D* label = 0;
	GateID id;
	QuantumGate* gate = 0;
	std::vector<int> joints;
	std::vector<complex> parameters;

	~GateInstance() {
		if (label) {
			delete label;
			label = 0;
		}
	}
	void setClass(GateID ID, QuantumGate* cls) {
		id = ID;
		gate = cls;
		parameters.resize(cls->parameters.size(), 0.0f);
		joints.resize(cls->numJoints, INT_MAX);
	}
	void generateLabel(bool regenerate = false) {
		if (gate)gate->generateLabel(regenerate);
		if (!text.size())return;
		if (!label)label = new Text3D();
		if (label->texture.empty() || regenerate)label->GenerateTexture(text, vec2(1.0f));
	}
	void regenerateLabel(const std::string& newText) {
		if (!label)label = new Text3D();
		label->GenerateTexture(newText, vec2(1.0f));
	}
	void RenderBase();
	void RenderLabel();
	void Render();
	void RenderBlochSphereBase();
	void RenderBlochSphere();
	bool completed()const {
		if (!gate)return false;
		return gate->numJoints == joints.size();
	}
	void Operate(QuantumEntanglement::QuantumCalculator& calculator) {
		if (!gate)return;
		if (gate->operate_fn) {
			gate->operate_fn(calculator, parameters);
			return;
		}
		calculator.Transform(gate->matrix);
	}
	void Save(File& f) {
		f << origin;
		f << id;
		f << int(joints.size());
		for (int i = 0; i < joints.size(); i++)f << joints[i];
		f << int(parameters.size());
		for (int i = 0; i < parameters.size(); i++)f << parameters[i];

	}
	void Load(File& f) {
		f >> origin;
		f >> id;
		int length = 0; f >> length;
		joints.resize(length);
		for (int i = 0; i < length; i++)f >> joints[i];
		f >> length;
		parameters.resize(length);
		for (int i = 0; i < length; i++)f >> parameters[i];
	}
};

struct OrderedGate {
	GateInstance* ptr = 0;
	OrderedGate() {}
	OrderedGate(GateInstance* ins) :ptr(ins) {}
	bool operator<(const OrderedGate& o)const {
		return ptr->origin.x == o.ptr->origin.x ? ptr->origin.y > o.ptr->origin.y : ptr->origin.x > o.ptr->origin.x;
	}
};

struct QuantumCircuit {
	QuantumEntanglement tensor;
	std::priority_queue<OrderedGate> q;
	std::map<int, uchar> qubitMap;

	uchar registeredQubits()const {
		return (uchar)qubitMap.size();
	}
	uchar getQubitID(int y)const {
		auto w = qubitMap.find(y);
		if (w == qubitMap.end())return -1;
		return (*w).second;
	}
	bool RegisterQubit(int y) {
		if (registeredQubits() == MAX_QUBITS)return false;
		qubitMap[y] = registeredQubits();
		return true;
	}
	void AllocateQubits() {
		tensor.Allocate(registeredQubits());
	}
	bool RegisterGate(GateInstance* gate) {
		if (!gate->completed())return false;
		q.push(gate);
		return true;
	}
	void Simulate() {
		while (q.size()) {
			GateInstance* gate = q.top().ptr; q.pop();
			int fn = gate->gate->style & 0b110000;
			if (fn == GATESTYLE_INPUT) {
				//init qubit
				uchar q = getQubitID(gate->origin.y);
				tensor.InitializeQubit(q, gate->gate->value);
				continue;
			}
			if (fn == GATESTYLE_MEASURE) {
				uchar q = getQubitID(gate->origin.y);
				//std::cout << "measure\n";
				bool v = tensor.Measure(q) & (1<<q);
				gate->regenerateLabel(v?"|1>":"|0>");
				if (gate->parameters.size()==2) {
					tensor.GetSuperposition(q, gate->parameters[0], gate->parameters[1]);
				}
				continue;
			}
			if (fn == GATESTYLE_OUTPUT) {
				uchar q = getQubitID(gate->origin.y);
				if (gate->parameters.size() == 2) {
					tensor.GetSuperposition(q, gate->parameters[0], gate->parameters[1]);
				}
				continue;
			}
			std::vector<uchar> qubits(gate->joints.size()+1);
			bool completed = true;
			for (uint i = 0; i < gate->joints.size(); i++) {
				qubits[i] = getQubitID(gate->joints[i]);
				if (qubits[i] >= tensor.numQubits()) {
					completed = false;
					break;
				}
			}
			if (!completed) continue;
			qubits.back() = getQubitID(gate->origin.y);
			QuantumEntanglement::QuantumCalculator calculator;
			tensor.GetCalculator(calculator, qubits);
			gate->Operate(calculator);
		}
	}
	void Clear() {
		tensor.Release();
		while (q.size())q.pop();
		qubitMap.clear();
	}
};

struct Thread {
	typedef std::list<GateInstance*>::const_iterator iter;
	std::list<GateInstance*> gates;
	float length = 0.0f;
	int x0 = 0;
	int y = 0;
	bool isEmpty()const {
		return !gates.size() || (gates.size()==1&&!gates.front());
	}
	bool noInput()const {
		return gates.size() && !gates.front();
	}
	int position(iter it)const {
		return ((*it) ? (*it)->origin.x : x0);
	}
	iter getRight(int pos)const {
		iter it = gates.begin();
		for (; it != gates.end(); it++) {
			if (position(it) >= pos)return it;
		}
		return it;
	}
	bool isOccupied(GateInstance& ins) {
		if (!ins.gate)return true;
		if (!gates.size())return false;
		if (ins.gate->isInput()) {
			if (gates.front())return true;
			return false;
		}
		iter it = getRight(ins.origin.x);
		if (it == gates.end())return false;
		if (position(it) - ins.origin.x < 1)return true;
		if (it == gates.begin())return false;
		iter left = it; left--;
		return (ins.origin.x - position(left) < 1);
	}
	void insert(GateInstance* ins) {
		const int correction = 1;
		if (isEmpty()) {
			if (ins->gate && !ins->gate->isInput())gates.push_back(0);
			gates.push_back(ins);
			length = 5.0f;
			x0 = ins->origin.x;
			return;
		}
		if (ins->gate && ins->gate->isInput() && !gates.front()) {
			gates.front() = ins;
			//x0 -= correction;
			//ins->origin.x = x0;
			x0 = ins->origin.x;
			return;
		}
		iter it = getRight(ins->origin.x);
		if (it == gates.begin()) {
			if (!gates.front()) {
				gates.insert(++it, ins);
				length += x0 - ins->origin.x;
				x0 = ins->origin.x;
				return;
			}
			gates.front()->origin.x = ins->origin.x - correction;
			x0 = ins->origin.x - correction;
			gates.insert(++it, ins);
			length += correction;
			return;
		}
		if (it == gates.end()) {
			length = ins->origin.x - x0 + 5.0f;
		}
		gates.insert(it, ins);
	}
	void Erase(GateInstance* ins) {
		if (gates.front() == ins) {
			gates.front() = 0;
			return;
		}
		gates.remove(ins);
	}
};

// to quadtree-asset class
class CircuitEntity {
public:
	typedef Quadtree<GateInstance*> quadtree_t;
	typedef quadtree_t::iterator iterator;
	typedef quadtree_t::Node Node;

	static std::vector<QuantumGate> mGateClasses;

	std::string mDescription;

	QuantumCircuit mCircuit;
	std::set<GateInstance*> mGates;
	Quadtree<GateInstance*> mQuadtree;
	std::map<int, Thread> mThreads;

	CircuitEntity() {
		
	}
	~CircuitEntity() {
		Release();
	}
	void Release() {
		for (GateInstance* gate : mGates) {
			delete gate;
		}
		mGates.clear();
		mQuadtree.Clear();
		mThreads.clear();
	}
	void Init(const vec3& worldSize) {
		mQuadtree.SetWorldSize(worldSize);
		InitGates();
		
	}
	GateInstance* Pickup(const Ray& ray, float& dist) {
		quadtree_t::Node* node = 0;
		iterator iter;
		return Pickup(ray, dist, node, iter);
	}
	GateInstance* Pickup(const Ray& ray, float& dist, Node*& node, iterator& iter) {
		return Pickup_tree(mQuadtree.root, mQuadtree.GetBoundingBox(), ray, dist, node, iter);
	}
	void EraseItem(Node* node, iterator& iter) {
		if (!node)return;
		GateInstance* item = iter->second;
		node->models.erase(iter);
		mGates.erase(item);
	}
	bool Render(const Camera& camera) {
		phong.bind();
		for (auto i : mThreads) {
			DrawThread(camera, ivec2(i.second.x0, i.first), i.second.length);
		}
		RenderGates_tree(mQuadtree.root, mQuadtree.GetBoundingBox(), camera);
	}
	static bool coveredVertex(const Camera& camera, ivec2& coord) {
		Ray ray = camera.cursor2world(window.mouse.pos, window.viewport);
		float t = -ray.ori.y / ray.dir.y;
		if (t < 0.0)return false;
		vec3 pos = (ray.ori + ray.dir * t) / GRID_DIM;
		coord.x = int(pos.x + sgn(pos.x) * 0.5f);
		coord.y = int(pos.z + sgn(pos.z) * 0.5f);
		return true;
	}
	
	Thread* GetThread(const ivec2& pos) {
		if (mThreads.find(pos.y) == mThreads.end())return 0;
		return &mThreads[pos.y];
	}
	void InsertGate(Thread*& th, GateInstance* ins) {
		if (!th) {
			th = &mThreads[ins->origin.y];
			th->y = ins->origin.y;
		}
		mGates.insert(ins);
		th->insert(ins);
		AABB aabb; aabb.set(grid2world(ins->origin), vec3(1.0f));
		mQuadtree.Insert(aabb, ins);
	}
	void InsertGate(Thread*& th, GateInstance* ins, Node*& node, iterator& iter) {
		if (!th) {
			th = &mThreads[ins->origin.y];
			th->y = ins->origin.y;
		}
		mGates.insert(ins);
		th->insert(ins);
		AABB aabb; aabb.set(grid2world(ins->origin), vec3(1.0f));
		node = mQuadtree.Insert(aabb, ins, iter);
	}
	void EraseThread(int y) {
		mThreads.erase(y);
	}
	bool Simulate() {
		mCircuit.Clear();

		for (const auto& it : mThreads) {
			if (!mCircuit.RegisterQubit(it.first)) {
				return false;
			}
		}
		mCircuit.AllocateQubits();

		for (const auto& it : mThreads) {
			for (GateInstance* ins : it.second.gates) {
				if(ins&&!mCircuit.RegisterGate(ins))return false;
			}
		}
		mCircuit.Simulate();
		return true;
	}
	void Clear() {
		mCircuit.Clear();
		Release();
	}
	void Load(const Path& path) {
		Clear();
		File f(path, "rb");
		int numThreads = 0; f >> numThreads;
		while(numThreads--) {
			int y = 0; f >> y;
			Thread& th = mThreads[y];
			int numGates; f >> numGates;
			while (numGates--) {
				bool real = false; f >> real;
				if (!real)continue;
				GateInstance* ins = new GateInstance();
				ins->Load(f);
				ins->setClass(ins->id, &mGateClasses[ins->id]);
				Thread* ptr = &th;
				InsertGate(ptr, ins);
			}
		}
		int length = 0; f >> length;
		f.read(mDescription, length);
		f.close();
	}
	void Save(const Path& path) {
		File f(path, "wb");
		f << int(mThreads.size());
		for (const auto& i : mThreads) {
			f << i.first;
			const Thread& th = i.second;
			f << int(th.gates.size());
			for (GateInstance* g : th.gates) {
				f << bool(g);
				if (g) {
					g->Save(f);
				}
			}
		}
		f << int(mDescription.size());
		f.write(mDescription);
		f.close();
	}
private:

	void InitGates();
	void RenderGates_tree(quadtree_t::Node& node, const AABB& aabb, const Camera& camera) {
		if (intersect(aabb, Ray(camera.position(), camera.front())) < 0.0f)return;
		for (auto& i : node.models) {
			i.second->Render();
		}
		for (int i = 0; i < 4; i++) {
			if (!node.ch[i])continue;
			AABB subAABB = aabb; mQuadtree.GetSubAABB(i, subAABB);
			RenderGates_tree(*node.ch[i], aabb, camera);
		}
	}
	GateInstance* Pickup_tree(quadtree_t::Node& node, const AABB& aabb, const Ray& ray, float& dist, quadtree_t::Node*& node_ret, iterator& iter) {
		float _dist = intersect(aabb, ray);
		if (_dist < 0.0f || _dist>dist)return 0;
		GateInstance* ret = 0;
		for (auto it = node.models.begin(); it != node.models.end(); it++) {
			_dist = intersect(it->first, ray);
			//std::cout << "_dist " << _dist << "\n";
			if (_dist >= 0.0f && _dist < dist) {
				dist = _dist;
				ret = it->second;
				node_ret = &node;
				iter = it;
				//std::cout << "dist " << dist << "\n";
			}
		}
		for (int i = 0; i < 4; i++) {
			if (!node.ch[i])continue;
			AABB subAABB = aabb;
			mQuadtree.GetSubAABB(i, subAABB);
			_dist = INFINITY;
			iterator _iter; quadtree_t::Node* _node = 0;
			GateInstance* item = Pickup_tree(*node.ch[i], subAABB, ray, _dist, _node, _iter);
			if (_dist < dist) {
				dist = _dist;
				ret = item;
				iter = _iter;
				node_ret = _node;
			}
		}
		return ret;
	}
	static void DrawThread(const Camera& camera, const ivec2& pos, float length) {
		activateHighlighted(camera, Color(0, 255, 0, 128));
		Transformation trans;
		trans.setScale(vec3(length, 0.2f, 0.2f));
		trans.setPosition(grid2world(pos)+vec3(length*0.5f, 0.0f, 0.0f));
		GLSLProgram::activatedProgram->set("model", trans.toMatrix());
		CUBE332.get().Draw();
		phong.bind();
	}
};


class MyWindow {
public:
	MyWindow() {
		mWindow.Init(&window, PROJECT_NAME, DOCKING_LEFT);
	}
	void BindCircuit(CircuitEntity* circuit) {
		mCircuit = circuit;
	}
	void Begin() {
		mWindow.Activate();
		mWindow.Begin();
	}
	void End() {
		mWindow.End();
	}
	void EditCircuit() {
		if (!mCircuit) return;
		Begin();
		ImGui::Text("Quantum Circuit Overview");
		// description
		if (mEditingDraft) {
			ImGui::InputText("Description", mDraft, 1024);
			if (ImGui::Button("Done")) {
				mCircuit->mDescription = mDraft;
				mEditingDraft = false;
			}
		}
		else {
			ImGui::TextWrapped(mCircuit->mDescription.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Edit Description")) {
				mEditingDraft = true;
				std::strcpy(mDraft, mCircuit->mDescription.c_str());
			}
		}
		if (ImGui::Button("Save")) {
			Path path = askSaveFileName(window.hwnd, L"All(*.*)");
			if (path)mCircuit->Save(path);
		}
		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			Path path = askOpenFileName(window.hwnd, L"All(*.*)");
			if(path)mCircuit->Load(path);
		}

		ImGui::Text((std::string("Number of qubits: ") + toString(mCircuit->mThreads.size())).c_str());

		ImGui::Checkbox("Simulate Frequently", &mAutoSimulation);
		if (mAutoSimulation && mTimer.interval() > 1.0) {
			mTimer.reset();
			mCircuit->Simulate();
		}
		if (ImGui::Button("Run")) {
			mCircuit->Simulate();
		}
		End();
	}

	void EditGate(const Camera& camera) {
		if (!editingGate)return;

		if (!pickedThread) {
			ivec2 coord;
			if (mCircuit->coveredVertex(camera, coord))editingGate->origin = coord;
			Thread* th = mCircuit->GetThread(editingGate->origin);
			bool occupied = th && th->isOccupied(*editingGate);
			phong.bind();
			editingGate->RenderBase();
			if (occupied) {
				activateHighlighted(camera, Color(255, 0, 0, 128));
				editingGate->Render();
			}
			else {
				activateHighlighted(camera, Color(0, 255, 0, 128));
				editingGate->Render();
			}
			if (!occupied && window.mouse.left.firstPressed()) {
				mCircuit->InsertGate(th, editingGate, pickedNode, pickedIter);
				pickedThread = th;
			}
		}
		else {
			phong.bind();
			editingGate->RenderBase();
			activateHighlighted(camera, Color(255, 255, 255, 128));
			editingGate->Render();
		}

		if (attachingJoint != -1 && window.mouse.left.firstPressed()) {
			ivec2 coord;
			if (mCircuit->coveredVertex(camera, coord)) {
				editingGate->joints[attachingJoint] = coord.y;
				attachingJoint = -1;
			}
		}

		mWindow.Activate();
		mWindow.Begin();
		ImGui::Text(editingGate->gate->name.c_str());
		const vec2 stride = QuantumGate::iconMap.textureCoordStride();
		const vec2 uv0 = editingGate->gate->off;
		const vec2 uv1 = uv0 + stride;
		ImGui::Image((void*)QuantumGate::iconMap.GetTexture().ID(), ImVec2(100, 100), ImVec2(uv0.x, uv0.y), ImVec2(uv1.x, uv1.y));
		if (ImGui::Button("Move") || (pickedThread && window.GetKey(GLFW_KEY_M) == GLFW_PRESS)) {
			pickedThread->Erase(editingGate);
			if (pickedThread->isEmpty()) {
				mCircuit->EraseThread(pickedThread->y);
			}
			pickedThread = 0;
			attachingJoint = -1;
			mCircuit->EraseItem(pickedNode, pickedIter);
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete") || window.GetKey(GLFW_KEY_DELETE) == GLFW_PRESS) {
			pickedThread->Erase(editingGate);
			if (pickedThread->isEmpty()) {
				mCircuit->EraseThread(pickedThread->y);
			}
			pickedThread = 0;
			attachingJoint = -1;
			mCircuit->EraseItem(pickedNode, pickedIter);
			editingGate = 0;
			return;
		}
		const std::vector<QuantumGate::Parameter>& params = editingGate->gate->parameters;
		for (uint i = 0; i < std::min(params.size(), editingGate->parameters.size()); i++) {
			const QuantumGate::Parameter& p = params[i];
			complex& value = editingGate->parameters[i];
			if (p.io) {
				if (p.io == 1) {
					ImGui::Text("Input:");
				}
				else {
					ImGui::Text("Output:");
					ImGui::Text((p.name + " = " + value.serialize()).c_str());
					continue;
				}
			}
			switch (p.type) {
			case QuantumGate::PARAM_BIT: {
				int bit = value.real > 0.1f ? 1 : 0;
				ImGui::InputInt(p.name.c_str(), &bit);
				value.real = bit ? 1.0f : 0.0f;
				break;
			}
			case QuantumGate::PARAM_REAL:
				ImGui::InputFloat(p.name.c_str(), &value.real);
				break;
			case QuantumGate::PARAM_COMPLEX:
				ImGui::Text((p.name + ":").c_str());
				ImGui::InputFloat("real", &value.real);
				ImGui::InputFloat("imaginary", &value.imaginary);
				break;
			};

		}
		for (int i = 0; i < editingGate->joints.size(); i++) {
			ImGui::Text(("Joint " + toString(i) + ": " + (editingGate->joints[i] == INT_MAX ? "Unattached" : toString(editingGate->joints[i]))).c_str());
			ImGui::SameLine();
			if (ImGui::Button(("Attach##" + toString(i)).c_str())) {
				attachingJoint = i;
			}
		}
		mWindow.End();
		if (!mWindow.IsActivated()) {
			editingGate = 0;
			pickedThread = 0;
			attachingJoint = -1;
		}
	}

	void Interact(const Camera& camera, bool able2place) {
		if (selectedGate == -1 && window.mouse.right.firstPressed()) {
			float dist = INFINITY;
			editingGate = mCircuit->Pickup(camera.cursor2world(window.mouse.pos, window.viewport), dist, pickedNode, pickedIter);
			ivec2 coord;
			if (editingGate) {
				pickedThread = mCircuit->GetThread(editingGate->origin);
			}
		}
		if (selectedGate != -1 && able2place) {
			GateInstance ins;
			ins.setClass((GateID)selectedGate, &CircuitEntity::mGateClasses[selectedGate]);
			if (CircuitEntity::coveredVertex(camera, ins.origin)) {
				Thread* th = mCircuit->GetThread(ins.origin);
				bool occupied = th && th->isOccupied(ins);
				if (occupied) {
					activateHighlighted(camera, Color(255, 0, 0, 128));
					ins.Render();
				}
				else {
					phong.bind();
					ins.RenderBase();
					activateHighlighted(camera, Color(0, 255, 0, 128));
					ins.Render();
				}


				if (!occupied && window.mouse.left.firstPressed()) {
					mCircuit->InsertGate(th, new GateInstance(ins));
				}
			}
			if (window.mouse.right.firstPressed() || window.GetKey(GLFW_KEY_C))selectedGate = -1;
		}
	}
	void Main(const Camera& camera, bool able2place) {
		Interact(camera, able2place);
		if (editingGate) {
			EditGate(camera);
			return;
		}
		EditCircuit();
	}
	void SelectGate(int id) {
		selectedGate = id;
	}
private:

	DockingWindow mWindow;
	CircuitEntity* mCircuit = 0;

	char mDraft[1024] = { 0 };
	bool mEditingDraft = false;

	Timer mTimer;

	bool mAutoSimulation = false;

	int selectedGate = -1;

	GateInstance* editingGate = 0;
	CircuitEntity::Node* pickedNode = 0;
	CircuitEntity::iterator pickedIter;
	Thread* pickedThread = 0;
	int attachingJoint = -1;
};