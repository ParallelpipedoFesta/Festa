#pragma once

#include "../common/window.h"
#include "../resources.h"

#include "../font.h"
#include "../audio.h"

namespace Festa {
    template<typename T>
    struct Coord2 {
        T x = 0, y = 0, prex = 0, prey = 0;
        void update(T x_, T y_) {
            prex = x; prey = y;
            x = x_; y = y_;
        }
        vec2 dir()const {
            return vec2(float(x - prex), float(y - prey));
        }
    };

    inline void KeyboardPosition(const Window& win, vec3& x, float vel, float acc,
        const vec3& right = VEC3X, const vec3& front = vec3(0.0f, 0.0f, -1.0f)) {
        static Timer timer;
        vel *= (float)timer.interval();
        acc *= (float)timer.interval();
        if (win.GetKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)vel = acc;
        if (win.GetKey(GLFW_KEY_A) == GLFW_PRESS)x -= vel * right;
        if (win.GetKey(GLFW_KEY_D) == GLFW_PRESS)x += vel * right;
        if (win.GetKey(GLFW_KEY_W) == GLFW_PRESS)x += vel * front;
        if (win.GetKey(GLFW_KEY_S) == GLFW_PRESS)x -= vel * front;
        if (win.GetKey(GLFW_KEY_E) == GLFW_PRESS)x.y += vel;
        if (win.GetKey(GLFW_KEY_C) == GLFW_PRESS)x.y -= vel;
        timer.reset();
    }

    struct FrameVel {
        Timer timer;
        float low = 0.0f, high = 0.0f, dt = 0.0f;
        bool first = true;
        FrameVel() {}
        FrameVel(float low, float high) :low(low), high(high) {
            timer.reset();
        }
        float velocity(bool highMode) {
            if (first) {
                first = false;
                timer.reset();
                dt = 0.0f;
                return 0.0f;
            }
            dt = float(timer.interval());
            timer.reset();
            if (highMode)return high * dt;
            else return low * dt;
        }
    };

    struct KeyboardAccelerator :public FrameVel {
        int key = -1;
        KeyboardAccelerator() {}
        KeyboardAccelerator(float low_)
        {
            low = low_;
            timer.reset();
        }
        void setAccel(float accelSpeed, int key_ = GLFW_KEY_LEFT_SHIFT) {
            high = accelSpeed;
            key = key_;
        }
        float velocity(const Window& win) {
            if (first) {
                first = false;
                timer.reset();
                return dt = 0.0f;
            }
            dt = float(timer.interval());
            timer.reset();
            if (key != -1 && win.GetKey(key) == GLFW_PRESS)return high * dt;
            else return low * dt;
        }
    };
    struct WASDCameraController {
        Camera* camera = 0;
        KeyboardAccelerator accelerator;
        float vel = 0.0f;
        WASDCameraController() {}
        WASDCameraController(Camera* camera, float speed = 5.0f) :camera(camera) {
            accelerator = KeyboardAccelerator(speed);
        }
        void update(const Window& win, bool activated = true) {
            vel = accelerator.velocity(win);
            vec3 right = camera->right(), front = camera->front();
            right.y = front.y = 0.0f; right = normalize(right), front = normalize(front);
            if (win.GetKey(GLFW_KEY_A) == GLFW_PRESS)camera->position() -= right * vel;
            if (win.GetKey(GLFW_KEY_D) == GLFW_PRESS)camera->position() += right * vel;
            if (win.GetKey(GLFW_KEY_W) == GLFW_PRESS)camera->position() += front * vel;
            if (win.GetKey(GLFW_KEY_S) == GLFW_PRESS)camera->position() -= front * vel;
        }
    };
    struct MouseDragging {
        Coord2<int> mouse;
        bool init = true;
        bool update(const Window& window) {
            if (window.mouse.left.up() || !window.mouse.inWindow()) {
                init = true;
                mouse = Coord2<int>();
                return false;
            }
            mouse.update(window.mouse.pos.x, window.mouse.pos.y);
            if (init) {
                init = false;
                return false;
            }
            return window.isFocused();
        }
        vec2 dir()const {
            return mouse.dir();
        }
        vec2 begin()const {
            return vec2(float(mouse.prex), float(mouse.prey));
        }
        vec2 end()const {
            return vec2(float(mouse.x), float(mouse.y));
        }
    };
    struct DraggingCameraController {
        Camera* camera = 0;
        float sensitivity = 0.0f;
        MouseDragging dragging;
        DraggingCameraController() {}
        DraggingCameraController(Camera* camera, float sensitivity = 0.1f) :camera(camera), sensitivity(sensitivity) {

        }
        void update(const Window& window, bool activated = true) {
            if (dragging.update(window)&&activated) {
                vec2 dir = dragging.dir() * sensitivity;
                camera->rot.pitch += dir.y;
                camera->rot.yaw += dir.x;
                camera->CalcViewAxes();
            }
        }

    };

    /*struct KeyboardCameraController {
        float sensitivity = 0.1f;
        WASDCameraController* base = 0;
        KeyboardCameraController() {}
        KeyboardCameraController(WASDCameraController* base, float sensitivity = 1.0f) :base(base), sensitivity(sensitivity) {}
        void apply(const Window& win, activated) {
            float deltaTime = base->accelerator.dt;
            float vel = base->accelerator.velocity(win);
            vec3 right = base->camera->right(), front = base->camera->front();
            right.y = front.y = 0.0f;
            right = normalize(right), front = normalize(front);
            KeyboardPosition(win, base->camera->pos, vel, vel, right, front);
            if (KEY_DOWN(VK_UP))base->camera->rot.pitch += sensitivity * deltaTime;
            if (KEY_DOWN(VK_DOWN))base->camera->rot.pitch -= sensitivity * deltaTime;
            if (KEY_DOWN(VK_LEFT))base->camera->rot.yaw += sensitivity * deltaTime;
            if (KEY_DOWN(VK_RIGHT))base->camera->rot.yaw -= sensitivity * deltaTime;
        }
    };*/

    struct CameraController1 {
        Window* window = 0;
        Camera* camera = 0;
        WASDCameraController wasd;
        DraggingCameraController dcc;
        CameraController1() {}
        CameraController1(Window* window, Camera* camera, float vel = 5.0f, float accel = 10.0f) :window(window), camera(camera) {
            dcc = DraggingCameraController(camera);
            wasd = WASDCameraController(camera, vel);
            wasd.accelerator.setAccel(accel);
        }
        void update(bool activated = true) {
            if (!window)return;
            activated &= window->isFocused();
            dcc.update(*window, activated);
            wasd.update(*window, activated);
            if (activated) {
                float vel = wasd.vel;
                if (window->GetKey(GLFW_KEY_E) == GLFW_PRESS)camera->position().y += vel;
                else if (window->GetKey(GLFW_KEY_C) == GLFW_PRESS)camera->position().y -= vel;
            }
        }
    };


    class Joystick {
    public:
        std::vector<ButtonStatus> buttons;
        std::unordered_map<std::string, uint> buttonMap;
        ButtonStatus plugged;
        Joystick() {
            ID = Assign++;
            joyinfoex.dwSize = sizeof(JOYINFOEX);
            joyinfoex.dwFlags = JOY_RETURNALL;
            init();
        }
        void update() {
            state = joyGetPosEx(ID, &joyinfoex);
            plugged.update(!bool(state));
            if (plugged.firstPressed())init();
            for (uint i = 0; i < buttons.size(); i++) {
                buttons[i].update(joyinfoex.dwButtons & (1 << i));
            }
        }
        std::string name()const {
            return wstring2string(caps.szPname);
        }
        static uint numJoysticks() {
            return Assign;
        }
        void init() {
            buttons.clear();
            state = joyGetDevCaps(ID, &caps, sizeof(caps));
            if (state != JOYERR_PARMS && state != JOYERR_UNPLUGGED)
                switch (joyGetDevCaps(ID, &caps, sizeof(caps))) {
                case 0:
                    break;
                case MMSYSERR_NODRIVER:
                    LOGGER.error("No Diver for Joystick");
                    break;
                default:
                    LOGGER.error("Unknown Joystick Error: " + toString(state));
                    break;
                }
            //cout << "mid " << caps.wMid << endl;
            //cout << "pid " << caps.wPid << endl;
            //cout << "name " << name() << endl;
            buttons.resize(caps.wNumButtons, ButtonStatus());
        }
        double X()const {
            return double(joyinfoex.dwXpos - caps.wXmin)
                / double(caps.wXmax - caps.wXmin) * 2.0f - 1.0f;
        }
        double Y()const {
            return -double(joyinfoex.dwYpos - caps.wYmin)
                / double(caps.wYmax - caps.wYmin) * 2.0f + 1.0f;
        }
        double Z()const {
            return double(joyinfoex.dwZpos - caps.wZmin)
                / double(caps.wZmax - caps.wZmin) * 2.0f - 1.0f;
        }
        double U()const {
            return double(joyinfoex.dwUpos - caps.wUmin)
                / double(caps.wUmax - caps.wUmin) * 2.0f - 1.0f;
        }
        double R()const {
            return -double(joyinfoex.dwRpos - caps.wRmin)
                / double(caps.wRmax - caps.wRmin) * 2.0f + 1.0f;
        }
        double V()const {
            return double(joyinfoex.dwVpos - caps.wVmin)
                / double(caps.wVmax - caps.wVmin) * 2.0f - 1.0f;
        }
        uint POV()const {
            return joyinfoex.dwPOV;
        }
        static vec2 circle(vec2 dir) {
            const float len = length(dir);
            if (len > 1.0f)dir /= len;
            return dir;
        }
        vec2 XY()const {
            return vec2(float(X()), float(Y()));
        }
        vec2 UR()const {
            return vec2(float(U()), float(R()));
        }
        vec2 circleXY()const {
            return circle(XY());
        }
        vec2 circleUR()const {
            return circle(UR());
        }
        const ButtonStatus& button(uint button) {
            return buttons[button];
        }
        const ButtonStatus& button(const std::string& button) {
            return buttons[buttonMap[button]];
        }
    private:
        static uint Assign;
        JOYINFOEX joyinfoex;
        uint ID = 0;
        JOYCAPS caps;
        MMRESULT state = 0;
    };

    struct JoystickCameraController {
        Camera* camera = 0;
        float sensitivity = 0.1f;
        FrameVel frame;
        JoystickCameraController() {}
        JoystickCameraController(Camera* camera, float speed = 10.0f, float sensitivity = 1.0f)
            :camera(camera), sensitivity(sensitivity) {
            frame = FrameVel(speed, 0);
        }
        void apply(const Window& win, Joystick& joystick) {
            if (!win.isFocused() || !joystick.plugged)return;
            float vel = frame.velocity(false);
            if (joystick.button(2).pressed())vel *= 2;
            vec3 right = camera->right(), front = camera->front();
            right.y = front.y = 0.0f; right = normalize(right), front = normalize(front);
            vec2 dir = joystick.circleXY() * vel;
            camera->position() += right * dir.x + front * dir.y;
            camera->position().y += vel * float(joystick.Z());
            float sen = sensitivity * frame.dt;
            camera->rot.pitch += float(joystick.R()) * sen;
            camera->rot.yaw -= float(joystick.U()) * sen;
        }
    };

    inline void rotateCamera(Window& window, Camera& camera,
        const vec3& ori, const EulerAngles& to,
        float time, const std::function<void()>& renderScene) {
        //window.update();
        const EulerAngles st = camera.rot;
        const EulerAngles t = to + (-st), v = t * (1.0f / time);
        const vec3 old = camera.position();
        float dur = 0.0f;
        FrameTimer timer;
        while (dur < time) {
            if (window.update() != 1)return;
            const EulerAngles delta = v * dur;
            camera.rot = st + delta;
            camera.position() = rotateOri(old, ori, Rotation(delta));
            dur += float(timer.interval());
            renderScene();

        }
        camera.rot = to;
        camera.position() = rotateOri(old, ori, Rotation(t));
        renderScene();
    }


    enum ResourceType : uchar {
        RT_NONE = 0,
        RT_COMBINATION,
        RT_MODEL,
        RT_GLSL,
        RT_SHADER,
        RT_AUDIO,
        RT_FREETYPE,
        RT_PHYMAT
    };
    struct Combination {
        struct Object {
            void* ptr = 0;
            ResourceType type = RT_NONE;
            Object() {}
            void release() {
                //std::cout << "rel " << int(type) << " " << ull(ptr) << '\n';
                if (ptr) {
                    delete ptr;
                    ptr = 0;
                }
            }
            template<typename T>
            void copy(const T& val) {
                release();
                ptr = new T(val);
            }
            void setCombination() {
                release();
                ptr = new Combination();
                type = RT_COMBINATION;
            }
            void setModel(const Path& file) {
                release();
                ptr = new Model(file);
                type = RT_MODEL;
            }
            void setShader(const Path& file) {
                release();
                ptr = new Shader(file);
                type = RT_SHADER;
            }
            void setGLSL(const std::vector<uint>& shaders) {
                release();
                ptr = new GLSLProgram(shaders);
                type = RT_GLSL;
            }
            void setSound(const Path& file) {
                release();
                ptr = new Sound(file);
                type = RT_AUDIO;
            }
            void setFont(GLWindow& window, const Path& file, uint size) {
                release();
                ptr = new FontRender(window, file, size);
                type = RT_AUDIO;
            }
            bool init(GLWindow& window, Combination& resources, const std::list<Combination*>& st, const ResourceType& _type, const std::vector<std::string>& parameters) {
                switch (_type) {
                case RT_COMBINATION:
                    setCombination();
                    return true;
                case RT_MODEL:
                    if (parameters.size() != 1)return false;
                    setModel(parameters[0]);
                    return true;
                case RT_SHADER:
                    if (parameters.size() != 1)return false;
                    setShader(parameters[0]);
                    return true;
                case RT_GLSL: {
                    std::vector<uint> shaders; shaders.resize(parameters.size());
                    for (uint i = 0; i < parameters.size(); i++) {
                        Object obj = resources.get(parameters[i], st);
                        if (!obj || obj.type != RT_SHADER)return false;
                        shaders[i] = obj.to<Shader>().ID();
                    }
                    setGLSL(shaders);
                    return true;
                }
                case RT_AUDIO:
                    if (parameters.size() != 1)return false;
                    setSound(parameters[0]);
                    return true;
                case RT_FREETYPE:
                    if (parameters.size() != 2)return false;
                    setFont(window, parameters[0], stringTo<uint>(parameters[1]));
                    return true;
                }
                return false;
            }
            operator bool()const {
                return ptr;
            }
            template<typename T>
            T& to() {
                return *((T*)ptr);
            }
            template<typename T>
            const T& to()const {
                return *((T*)ptr);
            }
            Object index(const std::string& name)const {
                return type == RT_COMBINATION ? to<Combination>()[name] : Object();
            }
            Object operator[](const std::string& name)const {
                Object ret = index(name);
                if (!ret)LOGGER.error("Attribution not found: " + name);
                return ret;
            }
            Object operator[](const char* name)const {
                Object ret = index(name);
                if (!ret)LOGGER.error("Attribution not found: " + std::string(name));
                return ret;
            }
        };
        std::map<std::string, Object> attributes;
        Object get(const std::string& name)const {
            return attributes.find(name) == attributes.end() ? Object() : (*attributes.find(name)).second;
        }
        bool find(const std::string& name)const {
            return attributes.find(name) != attributes.end();
        }
        Object operator[](const std::string& name)const {
            Object ret = index(name);
            if (!ret)LOGGER.error("Attribution not found: " + name);
            return ret;
        }
        Object index(const std::string& name)const {
            uint i = 0;
            while (i < name.size() && name[i] != '.')i++;
            if (i == name.size())
                return get(name);
            std::string fname = name.substr(0, i);
            if (find(fname)) {
                Object obj = get(fname);
                return obj.type == RT_COMBINATION ? obj.to<Combination>()[name.substr(i + 1)] : Object();
            }
            return Object();
        }
        void erase(const std::string& name) {
            uint i = 0;
            while (i < name.size() && name[i] != '.')i++;
            if (i == name.size()) {
                if (find(name)) {
                    attributes[name].release();
                    attributes.erase(name);
                }
                return;
            }
            std::string fname = name.substr(0, i);
            if (find(fname)) {
                Object obj = get(fname);
                if (obj.type == RT_COMBINATION)obj.to<Combination>().erase(name.substr(i + 1));
            }
        }
        void release() {
            for (auto& i : attributes)i.second.release();
            attributes.clear();
        }
        ~Combination() {
            release();
        }
        Object get(const std::string& name, const std::list<Combination*>& st) {
            int i = 0;
            while (i < name.size() && name[i] == '.')i++;
            if (i > st.size())return Object();
            auto it = st.begin(); int t = std::max(0, i - 1);
            while (t--)it++;
            //std::cout << "get " << name.substr(i) << " " << std::max(0, i - 1) << '\n';
            return (**it)[name.substr(i)];
        }
        Model* getModel(const std::string& name) {
            Object obj = (*this)[name];
            return obj ? &obj.to<Model>() : 0;
        }
        Shader* getShader(const std::string& name) {
            Object obj = (*this)[name];
            return obj ? &obj.to<Shader>() : 0;
        }
        GLSLProgram* getGLSL(const std::string& name) {
            Object obj = (*this)[name];
            return obj ? &obj.to<GLSLProgram>() : 0;
        }
        Sound* getSound(const std::string& name) {
            Object obj = (*this)[name];
            return obj ? &obj.to<Sound>() : 0;
        }
    };

    struct Resources : public Combination {
        typedef Combination::Object Object;
        static std::map<std::string, ResourceType> typeMapping;
        GLWindow* window = 0;
        Resources() {}
        ~Resources() {
            release();
        }
        bool parse(const std::string& line, std::list<Combination*>& st) {
            //std::cout << "parsing " << line <<" "<<st.size()<< '\n';
            int i = 0, j = int(line.size()) - 1;
            while (i <= j && line[i] == ' ' || line[i] == '\t')i++;
            while (j >= i && line[j] == ' ' || line[j] == '\t')j--;
            if (i > j || line[i] == '#')return true;
            if (line[i] == '}') {
                st.pop_front();
                if (!st.size()) {
                    LOGGER.error("Unexpected '}'");
                    return false;
                }
                i++;
                return i < j ? parse(line.substr(i + 1, j - i), st) : true;
            }
            int k = i;
            while (k < j && ('A' <= line[k] && line[k] <= 'z') || ('0' <= line[k] && line[k] <= '9') || line[k] == '_')k++;
            if (i == k) {
                LOGGER.error("Incomplete expression");
                return false;
            }
            std::string type = line.substr(i, k - i);
            i = k;
            while (i <= j && line[i] == ' ' || line[i] == '\t')i++;
            if (i > j) {
                LOGGER.error("Incomplete expression");
                return false;
            }
            if (line[i] == '{') {
                Object& obj = (*st.front()).attributes[type];
                if (!obj)obj.setCombination();
                st.push_front((Combination*)obj.ptr);
                return i < j ? parse(line.substr(i + 1, j - i), st) : true;
            }
            if (typeMapping.find(type) == typeMapping.end()) {
                LOGGER.error("Unknown type: " + type);
                return false;
            }
            ResourceType t = typeMapping[type];

            k = i;
            while (k <= j && ('A' <= line[k] && line[k] <= 'z') || ('0' <= line[k] && line[k] <= '9') || line[k] == '_')k++;
            if (i == k) {
                LOGGER.error("Incomplete expression");
                return false;
            }
            std::string name = line.substr(i, k - i);
            if ((*st.front()).find(name))return true;

            i = k;
            while (i <= j && line[i] == ' ' || line[i] == '\t')i++;
            if (i > j || line[i] != '=') {
                LOGGER.error("Missing '='");
                return false;
            }
            i++;
            std::vector<std::string> parameters;
            parameters.emplace_back(std::string());
            while (i <= j) {
                if ((!parameters.back().size() || (parameters.back()[0] != '"' && parameters.back()[0] != '\''))
                    && (line[i] == ' ' || line[i] == '\t')) {
                    if (parameters.back().size())parameters.emplace_back(std::string());
                    i++;
                    continue;
                }
                if (line[i] == ',')parameters.emplace_back(std::string());
                else parameters.back().push_back(line[i]);
                i++;
            }
            for (std::string& param : parameters) {
                if (!param.size()) {
                    LOGGER.error("Incomplete parameter");
                    return false;
                }
                if (param[0] == '"' || param[0] == '\'' || param.back() == '"' || param.back() == '\'') {
                    if (param.size() == 1 || param[0] != param.back()) {
                        LOGGER.error("Incomplete string");
                        return false;
                    }
                    param = param.substr(1, param.size() - 2);
                }
                //std::cout << param << '\n';
            }
            if (!parameters.size()) {
                LOGGER.error("Incomplete expression");
                return false;
            }
            if (!(*st.front()).attributes[name].init(*window, *this, st, t, parameters)) {
                LOGGER.error("Invalid parameters");
                return false;
            }
            return true;
        }
        void init(GLWindow& _window) {
            window = &_window;
        }
        bool load(const Path& file) {
            std::list<Combination*> st; st.push_front(this);
            std::string code; size_t pos = 0, prev = 0;
            File f(file); f.readLines(code);
            while (pos < code.size()) {
                if (code[pos] == '\n') {
                    if (!parse(code.substr(prev, pos - prev), st))return false;
                    prev = pos + 1;
                }
                pos++;
            }
            if (!parse(code.substr(prev), st))return false;
            if (st.size() != 1) {
                LOGGER.error("Missing '}'");
                return false;
            }
            return true;
        }
    };

}
