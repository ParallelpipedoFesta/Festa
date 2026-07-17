#pragma once

#include "game.h"
#include "gui.h"

namespace Festa {
	class Scene {
	public:
		enum ModelType {
			MT_STILL,
			MT_PLAYER,
			MT_ITEM,
			MT_FRIEND,
			MT_ENEMY
		};
		struct Object {
			std::string model;
			vec3 pos = vec3(0.0f);
			EulerAngles rot;
			vec3 scaling = vec3(1.0f);
			mat4 getMatrix()const {
				return translate4(pos) * rot.toMatrix() * scale4(scaling);
			}
		};
		Resources* resources;
		Object player;
		std::list<Object> still, friends, enemies, items;

		Scene() {}
		Scene(Resources* _resources) :resources(_resources) {}
		void init(Resources* _resources) {
			resources = _resources;
		}
		void load(const Path& file) {
			clear();
			File f;
			f.open(file, "rb");
			if (f.check())return;
			readObject(f, player);
			uint s; 
			f >> s; 
			still.resize(s); for (Object& obj : still)readObject(f, obj);
			f >> s;
			friends.resize(s); for (Object& obj : friends)readObject(f, obj);
			f >> s;
			enemies.resize(s); for (Object& obj : enemies)readObject(f, obj);
			f >> s;
			items.resize(s); for (Object& obj : items)readObject(f, obj);
		}
		void save(const Path& file) {
			File f;
			f.open(file, "wb");
			if (f.check())return;
			writeObject(f, player);
			f << uint(still.size()); for (const Object& obj: still)writeObject(f, obj);
			f << uint(friends.size()); for (const Object& obj : friends)writeObject(f, obj);
			f << uint(enemies.size()); for (const Object& obj : enemies)writeObject(f, obj);
			f << uint(items.size()); for (const Object& obj : items)writeObject(f, obj);
		}
		void clear() {
			player = Object();
			still.clear();
			friends.clear();
			enemies.clear();
			items.clear();
		}
		bool empty()const {
			return !player.model.size() && !still.size() && !friends.size() && !enemies.size() && !items.size();
		}
		void renderObject(const Object& obj) {
			if (!obj.model.size())return;
			Model& model = (*resources)["model"][obj.model].to<Model>();
			model.setTransformation(obj.getMatrix());
			model.render();
		}
		void render() {
			if(player.model.size())renderObject(player);
			for (const Object& obj : still)renderObject(obj);
			for (const Object& obj : friends)renderObject(obj);
			for (const Object& obj : enemies)renderObject(obj);
			for (const Object& obj : items)renderObject(obj);
		}
	private:
		void readString(File& f, std::string& str) {
			uint s; f >> s; str.resize(s);
			f.read(str, s);
		}
		void readObject(File& f, Object& obj) {
			readString(f, obj.model);
			f >> obj.pos >> obj.rot >> obj.scaling;
		}
		void writeString(File& f, const std::string& str) {
			f << (uint)str.size();
			f.write(str);
		}
		void writeObject(File& f, const Object& obj) {
			writeString(f, obj.model);
			f.writeBinaryData(obj.pos); f.writeBinaryData(obj.rot); f.writeBinaryData(obj.scaling);
		}
	};

	struct SEDesktop:public DockingWindow {
		Scene scene;
		Camera camera;
		CameraController1 cc;
		Scene::Object* obj;
		bool player = false;
		std::string name;
		ImGuiInputBox<128> inputBox;
		GeometricModel axes;
		bool renderFloor = true;

		SEDesktop() {}
		void initialize(GLWindow& _father, const string& _title, Resources& resources) {
			init(&_father, _title, DOCKING_LEFT);
			scene.init(&resources);
			camera.pos = vec3(0.0f, 2.0f, 5.0f);
			camera.rot.pitch = -5.0f;
			cc = CameraController1(father, &camera);
			inputBox.init("model");
			axes.axes();
		}
		void render() {
			if (!isActivated())return;
			begin();
			if (obj) renderTable();
			else renderDesktop();
			ImGui::Checkbox("Render Ground", &renderFloor);
			if (ImGui::Button("Reset Camera")) {
				camera.pos = vec3(0.0f, 2.0f, 5.0f);
				camera.rot = EulerAngles(-5.0f, 0.0f, 0.0f);
			}
			end();
		}
		void setObject(const std::string& _name, Scene::Object& _obj) {
			name = _name;
			obj = &_obj;
			player = false;
		}
		void setPlayer(const std::string& _name, Scene::Object& _obj) {
			name = _name;
			obj = &_obj;
			player = true;
		}
		void renderDesktop() {
			if (ImGui::Button("Open")) {
				if (!scene.empty() && father->messagebox("Save?", title)) {
					askSave();
				}
				Path file = askOpenFileName(father->hwnd, L"All(*.*)\0*.*\0\0");
				if (file) {
					std::cout << "load " << file << '\n';
					scene.load(file);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {
				askSave();
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear")) {
				if (father->messagebox("Are u sure?", title))
					scene.clear();
			}
			if (ImGui::Selectable("Player##")) {
				setPlayer("player", scene.player);
			}
			objectTable("Scene", scene.still);
			objectTable("Friends", scene.friends);
			objectTable("Enemies", scene.enemies);
			objectTable("Items", scene.items);
		}
		void objectTable(const std::string& label, std::list<Scene::Object>& objects) {
			if (ImGui::TreeNode((label+"##").c_str())) {
				ImGui::SameLine();
				if (ImGui::Button(("Add##" + label).c_str())) {
					objects.emplace_back(Scene::Object());
					setObject(label+"_" + toString(objects.size()), objects.back());
					return;
				}
				uint i = 1;
				for (auto it = objects.begin(); it != objects.end();it++) {
					if (ImGui::Selectable((label +"_" + toString(i) + "_" + it->model).c_str())) {
						setObject(label+"_" + toString(i), *it);
						return;
					}
					i++;
				}
				ImGui::TreePop();
			}
		}
		bool eraseCurrent(std::list<Scene::Object>& objects) {
			for (auto it = objects.begin(); it != objects.end(); it++) {
				if (&*it == obj) {
					objects.erase(it);
					return true;
				}
			}
			return false;
		}
		
		bool eraseCurrent() {
			return eraseCurrent(scene.still) || eraseCurrent(scene.friends) || eraseCurrent(scene.enemies) || eraseCurrent(scene.items);
		}
		void renderTable() {
			if (ImGui::Button("<")) {
				obj = 0;
				return;
			}
			ImGui::Text((name).c_str());
			if (!player&&ImGui::Button("Delete")&&father->messagebox("Are u sure?", title)) {
				eraseCurrent();
				obj = 0;
				return;
			}
			inputBox.render();
			ImGui::SameLine();
			if (ImGui::Button("OK")) {
				std:string model = inputBox;
				if ((*scene.resources)["model"].to<Combination>().find(model)) {
					obj->model = model;
				}
				else father->messagebox("Model not found", title);
			}
			ImGui::InputFloat3("Position", &obj->pos.x);
			ImGui::SliderFloat3("Rotation", &obj->rot.pitch, -180.0f, 180.0f);
			ImGui::InputFloat3("Scaling", &obj->scaling.x);
			if (ImGui::Button("Reset")) {
				obj->pos = vec3(0.0f);
				obj->rot = EulerAngles();
				obj->scaling = vec3(1.0f);
			}

			if (!father->isFocused()&&father->mouse.left.pressed()) {
				float vel = 5.0f*father->interval();
				if (KEY_DOWN(VK_LSHIFT))vel *= 5.0f;
				if (KEY_DOWN('W'))
					obj->pos.z -= vel;
				if (KEY_DOWN('S'))
					obj->pos.z += vel;
				if (KEY_DOWN('A')) 
					obj->pos.x -= vel;
				if (KEY_DOWN('D'))
					obj->pos.x += vel;
				if (KEY_DOWN('E'))
					obj->pos.y += vel;
				if (KEY_DOWN('C'))
					obj->pos.y -= vel;
			}
		}
		void askSave() {
			Path file = askSaveFileName(father->hwnd, L"All(*.*)\0*.*\0\0");
			if (file) {
				scene.save(file);
			}
		}
		void renderScene() {
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			(*scene.resources)["GLSL"]["phong"].to<GLSLProgram>().bind();

			PointLight light(vec3(3.0f), vec3(0.5f, 0.31f, 0.57f), vec3(0.7f, 0.57f, 0.7f), vec3(1.0f, 0.77f, 0.98f), 0.5f, 0.01f, 0.01f);

			light.bind("pointLights[0]");
			camera.shape = CameraShape(father->viewport, 45.0f, 1.0f, 100.0f);
			cc.update();
			camera.bind();

			if (renderFloor) {
				Material mat(MaterialData(vec3(0.44f, 0.88f, 0.33f), vec3(0.44f, 0.93f, 0.3f), vec3(1.0f), 2.0f));
				Transformation trans;
				trans.setPosition(vec3(0.0f, -1.0f, 0.0f));
				trans._scale(vec3(100.0f, 1.0f, 100.0f));
				drawCuboid(mat, trans);
			}

			if (obj) {
				axes.setTransformation(obj->getMatrix());
				axes.render();
			}
			axes.setTransformation(Transformation());
			axes.render();

			scene.render();
		}
		void update() {

		}
	};

	struct SceneEditor {
		SEDesktop desktop;
		SceneEditor() {}
		void init(GLWindow& window, const std::string& title, Resources& resources) {
			desktop.initialize(window, title, resources);
		}
		void update() {
			desktop.render();
			desktop.renderScene();
		}
		
	};
}