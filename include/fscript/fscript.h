#pragma once

#include "../utils/game.h"
#include "../physics.h"

namespace Festa {
	inline void Pir_raise(const std::string& msg) {
		std::cout << msg << std::endl;
	}
	inline bool isNumber(const char* str) {
		uint64 size = strlen(str);
		if (!size)return false;

		bool P = false, F = false;
		uint64 epos = 0;
		for (uint64 i = 0; i < size; i++) {
			switch (str[i]) {
			case '.':
				if (size == 1 || P)return false;
				P = true;
				break;
			case 'e':
				if (size < 3 || epos || i == 0 || i == size - 1)return false;
				epos = i + 1;
				break;
			case '-':
				if (size < 2 || i == 0 || str[i - 1] != 'e')return false;
				break;
			default:
				if (str[i] < '0' || str[i]>'9')return false;
			}
		}
		return true;
	}

	inline int ParseNumber(const std::string& str, bool& neg) {
		neg = false;
		if (!str.size())
			return -1;
		uint64 idx = 0, size = str.size();
		if (str[0] == '-') {
			neg = true;
			if (size == 1)return -1;
			idx++;
		}
		if (str == "inf")return 2;

		bool P = false, F = false, isFloat = false;
		uint64 epos = 0;
		for (; idx < size; idx++) {
			switch (str[idx]) {
			case '.':
				isFloat = true;
				if (P)return -1;
				P = true;
				break;
			case 'f':
				isFloat = true;
				if (F || idx < size - 1 || idx == 0)return -1;
				F = true;
				size--;
				break;
			case 'e':
				isFloat = true;
				if (epos || idx == 0 || idx == size - 1)return -1;
				epos = idx + 1;
				break;
			case '-':
				if (idx == 0 || str[idx - 1] != 'e')return -1;
				break;
			default:
				if (str[idx] < '0' || str[idx]>'9')return -1;
			}
		}
		return isFloat;
	}

	struct FObject {
		typedef std::map<std::string, FObject> attr_t;
		void* ptr = 0;
		FObject* type = 0;
		FObject* father = 0;
		//attr_t attr;
		FObject() {}
		FObject(FObject* _type, void* _ptr) :type(_type), ptr(_ptr) {}
		void release() {
			if (ptr)SafeDelete(ptr);
			if (father)SafeDelete(father);
			ptr = 0;
			father = 0;
			//attr.clear();
		}
		operator bool()const {
			return ptr&&type;
		}
	};
	struct FFunction {
		struct Parameter {
			const FObject* type=0;
			FObject def;
		};
		typedef std::vector<FObject*> param_t;
		typedef FObject (*fn_t)(const param_t&);
		fn_t fn = 0;
		std::vector<Parameter> parameters;
		bool check(const param_t& params) {
			if (!parameters.size())return true;
			if (params.size() != parameters.size())return false;
			for (uint i = 0; i < params.size(); i++) {
				if (parameters[i].type && parameters[i].type != params[i]->type)return false;
			}
			return true;
		}
		FObject call(const param_t& param) {
			if (!fn || !check(param)) {
				Pir_raise("Illegal parameters");
				return FObject();
			}
			return fn(param);
		}
	};

	struct FType {
		typedef FObject (*fn0)();
		typedef FObject (*fn1)(const FObject&);
		typedef FObject (*fn2)(const FObject&, const FObject&);
		typedef FObject(*fn_call)(const FObject&, const FFunction::param_t& params);
		typedef std::string(*fn_string)(const FObject&);
		typedef FObject(*fn_attr)(const FObject&, const std::string&, const FObject&);
	
		std::string name;
		size_t size;
		fn_string _string = 0;
		fn_call _call = 0;
		fn_attr _attr = 0;
		std::map<std::string, fn2> attrs;
		std::map<std::string, FFunction> methods;
		void addAttr(const std::string& name, fn2&& callback) {
			attrs[name] = callback;
		}
		FObject _getAttr(const FObject& self, const std::string& name) {
			if (_attr)return _attr(self, name, FObject());
			if (attrs.find(name) == attrs.end() || !attrs[name])
				return FObject();
			return attrs[name](self, FObject());
		}
		FObject getAttr(const FObject& self, const std::string& name) {
			if (_attr)return _attr(self, name, FObject());
			if (attrs.find(name) == attrs.end()||!attrs[name]) {
				Pir_raise("Attr not found: " + name);
				return FObject();
			}
			return attrs[name](self, FObject());
		}
		void setAttr(const FObject& self, const std::string& name, const FObject& val) {
			if (_attr) {
				_attr(self, name, val);
				return;
			}
			if (attrs.find(name) == attrs.end()||!attrs[name]) {
				Pir_raise("Attr not found: " + name);
				return;
			}
			attrs[name](self, val);
		}
		FObject callMethod(FObject& self, const std::string& name, const FFunction::param_t& _params) {
			FFunction::param_t params(_params.size() + 1);
			params[0] = &self;
			for (uint i = 0; i < _params.size(); i++)params[i + 1] = _params[i];
			if (methods.find(name) != methods.end())return methods[name].call(params);
			return Fcall(getAttr(self, name), params);
		}
		FObject _callMethod(const std::string& name, const FFunction::param_t& params) {
			if (methods.find(name) == methods.end()) {
				Pir_raise("method not found: " + name);
				return FObject();
			}
			return methods[name].call(params);
		}
		static FType* Ftype(const FObject& x) {
			return x ? ((FType*)x.type->ptr) : 0;
		}
		static FObject Fcopy(const FObject& x) {
			FType* type = Ftype(x);
			if (!type)return FObject();
			uchar* buf = new uchar[type->size];
			for (size_t i = 0; i < type->size; i++)buf[i] = ((uchar*)x.ptr)[i];
			return FObject(x.type, buf);
		}
		static FObject Fcall(const FObject& x, const FFunction::param_t& params) {
			FType* type = Ftype(x);
			return type ? type->_call(x, params) : FObject();
		}
		void addMethod(const std::string& name , const FFunction& fn){
			methods[name] = fn;
		}
	};

	extern FObject FTypeType, FFunctionType, FMyFunctionType;
	extern FObject FIntType, FFloatType, FStringType, FBoolType;
	extern FObject FVec2Type, FVec3Type, FVec4Type;
	extern FObject FScriptType, FClassType;
	extern FObject FTransformationType, FAABBType;
	

	typedef enum : char {
		NONE,

		OBJECT,
		NUMBER,
		BOOLEAN,
		STRING,
		MAPPING,
		FNCALL,
		CODE_BLOCK,
		MACRO,
		FINPUTS,

		COPY,

		IF,
		ELIF,
		ELSE,
		FOR,
		WHILE,
		BREAK,
		CONTINUE,
		RETURN,

		ADD,
		SUB,
		MUL,
		DIV,
		POW,

		ADD_EQ,
		SUB_EQ,
		MUL_EQ,
		DIV_EQ,
		POW_EQ,

		EQ,
		GETATTR,
		GETITEM,
		LT,
		GT,
		LE,
		GE,
		NEQ,

		NOT,
		AND,
		OR,

		FUNCTION,

		COMMA,
		MAX_TOKEN,
	}Token;
	struct PiroofGlobals {
		struct Operator {
			int p = 0;
			char m = 0;
			bool post = false;
			std::string name;
		};
		typedef FObject(*opcallback)(FObject, FObject);
		std::unordered_map<std::string, char> tkmp;
		std::unordered_map<std::string, opcallback> callbacks;
		std::unordered_map<std::string, std::string> macros;
		std::list<FObject> garbage;
		std::unordered_map<std::string, FObject> globals;
		std::vector<Operator> operators;

		PiroofGlobals() {
			loadOperatorCallbacks();
			initOperators();
			initTkmp();
			//initGlobals();
		}
		~PiroofGlobals() {
			release();
		}
		void release();
		void initGlobals();
		void initOperators();
		void initTkmp();
		void loadOperatorCallbacks();
		FObject _operate(char tk, FObject a, FObject b, std::string& fname);
		FObject operate(char tk, FObject a, FObject b);
		FObject operate(char tk, FObject a);
		void replaceMacro(std::string& str) {
			while (macros.find(str) != macros.end())
				str = macros[str];
		}
		bool findtkstr(const std::string& tkstr) {
			return tkmp.find(tkstr) != tkmp.end();
		}
		opcallback getcallback(const std::string& str) {
			return callbacks.find(str) == callbacks.end() ? 0 : callbacks[str];
		}
		char gettk(const std::string& tkstr) {
			return tkmp.find(tkstr) == tkmp.end() ? NONE : tkmp[tkstr];
		}
		Operator& getop(char tk) {
			//std::cout << "getop " << int(tk) << '\n';
			return operators[tk];
		}
		Operator& getop(const std::string& tkstr) {
			return operators[gettk(tkstr)];
		}
		void sepcallback(std::string& name, opcallback f) {
			callbacks.insert({ name,f });
		}
		template<typename T>
		FObject& addType(const std::string& name, FType::fn_string&& _str=0, FType::fn_call&& _call=0) {
			garbage.emplace_back(FObject(&FTypeType, new FType{ name, sizeof(T), _str, _call }));
			return garbage.back();
		}
		FObject& addFunction(const std::string& name, const FFunction& fn) {
			return globals[name]=FObject(&FFunctionType,new FFunction(fn));
		}
	};
	extern PiroofGlobals PirGlobals;
	
	class FInterpreter {
	public:
		struct _Expr {
			char tk;
			FObject val;
			std::vector<_Expr> ch;
			bool comp = false;
			bool trash = true;
			std::string name;

			std::string tostr() {
				//PirGlobals.getop(tk).name
				std::string ret = name+PirGlobals.getop(tk).name + "_" + toString(int(tk)) + "(";
				//std::string ret = "";
				for (size_t i = 0; i < ch.size(); i++)
					ret += ch[i].tostr() + ",";
				if (comp)
					if (ch.size())ret.back() = ')';
					else ret.push_back(')');
				return ret;
			}
			void release() {
				for (_Expr& expr : ch)expr.release();
				ch.clear();
				if (trash)val.release();
				tk = 0;
				comp = false;

			}
			_Expr& operator[](uint i) {
				return ch[i];
			}
		};
		struct IfState {
			size_t line, depth;
			bool val;
		};
		struct State {
			char tk;
			void* val = 0;
			~State() {
				if (val) {
					delete val; val = 0;
				}
			}
		};
		struct Variables {
			std::unordered_map<std::string, FObject> mp;
			std::set<void*> garbage;
			Variables() {}
			~Variables() {
				//release();
			}
			bool find(const std::string& name) {
				return mp.find(name) != mp.end();
			}
			void release() {
				//std::cout << "rrr "<<ull(this)<<'\n';
				//std::set<void*> _garbage;
				for (auto& i : mp)
					if (PirGlobals.globals.find(i.first) == PirGlobals.globals.end()) {
					//std::cout << "rel "<<i.first << '\n';
					garbage.insert(i.second.ptr);
				}
				for (void* ptr : garbage)SafeDelete(ptr);
				mp.clear();
				//std::cout << "rrr\n";
			}
			FObject& get(const std::string& name) {
				return mp[name];
			}
			FObject& operator[](const std::string& name) {
				return mp[name];
			}
			void erase(const std::string& name) {
				if (find(name))garbage.insert(mp[name].ptr);
				mp.erase(name);
			}
		};
		struct FMyFunction {
			struct Parameter {
				std::string name;
				const FObject* type = 0;
				FObject def;
			};
			FInterpreter* inter = 0;
			FInterpreter::_Expr expr;
			std::vector<Parameter> parameters;
			bool check(const FFunction::param_t& params) {
				if (!parameters.size())return true;
				if (params.size() != parameters.size())return false;
				for (uint i = 0; i < params.size(); i++) {
					if (parameters[i].type && parameters[i].type != params[i]->type)return false;
				}
				return true;
			}
			FObject call(const FFunction::param_t& param) {
				if (!inter || !check(param)) {
					Pir_raise("Illegal parameters");
					return FObject();
				}
				return inter->runFunction(expr, parameters, param);
			}
			void release() {
				//constants
				expr.release();
			}
			static void releaseExpr(_Expr& src) {
				for (_Expr& i : src.ch)releaseExpr(i);
				if (src.tk != OBJECT && src.tk != STRING && src.tk != NUMBER && src.trash)src.val.release();
			}
		};
		Variables globals;
		std::stack<Variables> locals_st;
		std::stack<_Expr> st;
		std::queue<FObject*> inputs;

		size_t indent = 0, lineid = 0, depth = 0;
		char ifstatus = -1;

		FInterpreter() {
			//initGlobals();
		}
		~FInterpreter() {
			release();
		}
		void release() {
			globals.release();
			while (locals_st.size()) {
				locals_st.top().release();
				locals_st.pop();
			}
		}
		FObject perform(_Expr& expr, char& code, FObject& ret) {
			//cout << "perform " << expr.tostr() << endl;
			int ifst = -1;
			FObject res = _perform(expr, ifst, code, ret);
			ifstatus = ifst;
			return res;
		}
		FObject perform_(_Expr& expr) {
			char code = 0;
			FObject ret;
			int ifst = -1;
			FObject res = _perform(expr, ifst, code, ret);
			ifstatus = ifst;
			return code?ret:res;
		}
		FObject interpret(const std::string&);
		bool interpretFile(const Path& file) {
			std::ifstream f(file);
			if (!f.is_open()) {
				Pir_raise("file not found");
				return false;
			}
			while (st.size())st.pop();
			std::string line;
			lineid = 0;
			while (std::getline(f, line)) {
				uint64 _lineid = lineid;
				interpret(line);
				lineid = _lineid + 1;
			}
			if (st.size()) {
				Pir_raise("Incomplete expression at the end of the file");
				while (st.size())st.pop();
				return false;
			}
			return true;
		}
		Variables& locals() {
			return locals_st.top();
		}
		FObject& getVar(const std::string& name) {
			return locals_st.size() && !globals.find(name) ? locals()[name] : globals[name];
		}
		void eraseVar(const std::string& name) {
			if (locals_st.size() && !locals().find(name))locals().erase(name);
			else if (globals.find(name))globals.erase(name);
		}
		bool findVar(const std::string& name) {
			return globals.find(name)||(locals_st.size()&&locals().find(name));
		}
		FObject runFunction(_Expr expr, const std::vector<FMyFunction::Parameter>& t, const FFunction::param_t& param) {
			pushLocals();
			for (uint i = 0; i < t.size(); i++) {
				getVar(t[i].name) = *param[i];
			}
			FObject ret=perform_(expr);
			for (uint i = 0; i < t.size(); i++) {
				locals().erase(t[i].name);
			}
			FMyFunction::releaseExpr(expr);
			popLocals();
			return ret;
		}
		FObject& operator[](const std::string& name) {
			return globals[name];
		}
		void initGlobals() {
			if (!PirGlobals.globals.size())PirGlobals.initGlobals();
			globals.mp = PirGlobals.globals;
		}
	protected:
		FObject _perform(_Expr& expr, int& ifst, char& code, FObject& ret);
		char token(const char* bg, size_t len);
		int fold(char back = 0);
		int normalize(_Expr& expr);
		void pushLocals() {
			locals_st.push(Variables());
		}
		void popLocals() {
			locals_st.top().release();
			locals_st.pop();
		}
		FObject fncall(const FObject& f, _Expr& expr, char& code, FObject& ret) {
			std::vector<FObject*> params(expr.ch.size());
			for (uint i = 0; i < params.size(); i++) {
				perform(expr[i], code, ret);
				params[i] = &expr[i].val;
			}
			//std::cout << "construct "<<expr.tostr()<<'\n';
			return FType::Fcall(f, params);
		}
	};

	struct FScript:public FInterpreter {
		FScript() {
			
		}
		FScript(const Path& path) {
			initGlobals();
			load(path);
		}
		void load(const Path& path) {
			interpretFile(path);
		}
		void input(const FFunction::param_t& params) {
			for (const auto& i : params)inputs.push(i);
		}
		void call(const std::string& fn) {
			if (!findVar(fn))return;
			FObject& obj = getVar(fn);
			if (obj.type != &FMyFunctionType)return;
			_Expr expr = ((FMyFunction*)obj.ptr)->expr;
			pushLocals();
			FObject ret = perform_(expr);
			FMyFunction::releaseExpr(expr);
			popLocals();
		}
		void run() {
			/*if (!findVar("loop"))return;
			FObject& obj = getVar("loop");
			if (obj.type != &FMyFunctionType)return;
			_Expr expr = ((FMyFunction*)obj.ptr)->expr;
			FObject ret = perform_(expr);
			FMyFunction::releaseExpr(expr);*/
			call("loop");
		}
	};
}