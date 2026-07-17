#pragma once
#include "fscript.h"
#include "../physics.h"

using namespace Festa;
using namespace std;

PiroofGlobals Festa::PirGlobals;

typedef const FObject& var;


static std::string F2string(var x) {
	return x ? FType::Ftype(x)->_string(x) : "null";
}

static std::string type_string(var x) {
	return ((FType*)x.ptr)->name;
}
FObject Festa::FTypeType(
	&FTypeType, new FType{ "type", sizeof(FType), type_string}
);

static FObject function_call(var x, const FFunction::param_t& params) {
	FFunction* f = (FFunction*)x.ptr;
	return f->call(params);
}
FObject Festa::FFunctionType=PirGlobals.addType<FFunction>("function", 0, function_call);

typedef FInterpreter::FMyFunction FMyFunction;
static FObject myFunction_call(var x, const FFunction::param_t& params) {
	FMyFunction* f = (FMyFunction*)x.ptr;
	return f->call(params);
}
FObject Festa::FMyFunctionType = PirGlobals.addType<FFunction>("myFunction", 0, myFunction_call);

static std::string string_string(var x) {
	return *((std::string*)x.ptr);
}
FObject Festa::FStringType = PirGlobals.addType<std::string>("string", string_string);


static FObject print_fn(const FFunction::param_t& param) {
	for (auto i : param) {
		std::cout << F2string(*i) << "\n";
	}
	return FObject();
}

static std::string int_string(var x) {
	return toString(*((int*)x.ptr));
}
FObject Festa::FIntType = PirGlobals.addType<int>("int", int_string);

static std::string float_string(var x) {
	return toString(*((float*)x.ptr));
}
FObject Festa::FFloatType = PirGlobals.addType<float>("float", float_string);

static std::string bool_string(var x) {
	return *((bool*)x.ptr) ? "true" : "false";
}
FObject Festa::FBoolType = PirGlobals.addType<bool>("bool", bool_string);

static std::string vec2_string(var x) {
	vec2& v = *((vec2*)x.ptr);
	return "vec2(" + toString(v.x) + ", " + toString(v.y) + ")";
}
FObject Festa::FVec2Type = PirGlobals.addType<vec2>("vec2", vec2_string);
static FObject vec2_fn(const FFunction::param_t& param) {
	return FObject(&FVec2Type, new vec2(*((float*)param[0]->ptr), *((float*)param[1]->ptr)));
}

static std::string vec3_string(var x) {
	vec3& v = *((vec3*)x.ptr);
	return "vec3(" + toString(v.x) + ", " + toString(v.y) + ", " + toString(v.z) + ")";
}
FObject Festa::FVec3Type = PirGlobals.addType<vec3>("vec3", vec3_string);
static FObject vec3_fn(const FFunction::param_t& param) {
	return FObject(&FVec3Type, new vec3(*((float*)param[0]->ptr), *((float*)param[1]->ptr), *((float*)param[2]->ptr)));
}

static std::string vec4_string(var x) {
	vec4& v = *((vec4*)x.ptr);
	return "vec4(" + toString(v.x) + ", " + toString(v.y) + ", " + toString(v.z) + ", " + toString(v.w) + ")";
}
FObject Festa::FVec4Type = PirGlobals.addType<vec4>("vec4", vec4_string);
static FObject vec4_fn(const FFunction::param_t& param) {
	return FObject(&FVec4Type, new vec4(*((float*)param[0]->ptr), *((float*)param[1]->ptr), *((float*)param[2]->ptr), *((float*)param[3]->ptr)));
}

template<typename T>
static FObject vec_x(var x, var set) {
	T& v = *((T*)x.ptr);
	if (set) {
		v.x = *((float*)set.ptr);
		return FObject();
	}
	return FObject(&FFloatType, new float(v.x));
}
template<typename T>
static FObject vec_y(var x, var set) {
	T& v = *((T*)x.ptr);
	if (set) {
		v.y = *((float*)set.ptr);
		return FObject();
	}
	return FObject(&FFloatType, new float(v.y));
}
template<typename T>
static FObject vec_z(var x, var set) {
	T& v = *((T*)x.ptr);
	if (set) {
		v.z = *((float*)set.ptr);
		return FObject();
	}
	return FObject(&FFloatType, new float(v.z));
}
template<typename T>
static FObject vec_w(var x, var set) {
	T& v = *((T*)x.ptr);
	if (set) {
		v.w = *((float*)set.ptr);
		return FObject();
	}
	return FObject(&FFloatType, new float(v.w));
}

static FObject script_fn(const FFunction::param_t& param) {
	//cout << "path " << *((std::string*)param[0]) << endl;
	return FObject(&FScriptType, new FScript(*((std::string*)param[0]->ptr)));
}
static FObject script_attr(const FObject& x, const std::string& name, const FObject& set) {
	FScript* fs = (FScript*)x.ptr;
	if (!fs->globals.find(name)) {
		Pir_raise("Attr not found: "+name);
		return FObject();
	}
	if (set) {
		fs->globals.erase(name);
		fs->globals[name] = set;
		return FObject();
	}
	FObject& obj = fs->globals[name];
	FType* type = FType::Ftype(obj);
	if (!type)return FObject();
	if (type == FMyFunctionType.ptr) {
		return FObject(&FMyFunctionType, new FMyFunction(*((FMyFunction*)obj.ptr)));
	}
	//return fs->globals[name];
	return FType::Fcopy(obj);
}
static FObject class_fn(const FFunction::param_t& param) {
	return FObject(&FClassType, new std::string(*((std::string*)param[0]->ptr)));
}
static FObject class_call(var x, const FFunction::param_t& param) {
	FScript* fs = new FScript();
	fs->initGlobals();
	fs->input(param);
	fs->load((*(std::string*)x.ptr));
	return FObject(&FScriptType, fs);
}

FObject Festa::FScriptType = PirGlobals.addType<FScript>("fscript", 0);
FObject Festa::FClassType = PirGlobals.addType<std::string>("fclass", 0, class_call);

static FObject transformation_fn(const FFunction::param_t& param) {
	return FObject(&FTransformationType, new Transformation());
}
static FObject transformation_pos(var x, var set) {
	Transformation& v = *((Transformation*)x.ptr);
	if (set) {
		v.setPosition(*((vec3*)set.ptr));
		return FObject();
	}
	return FObject(&FVec3Type, new vec3(v.getPosition()));
}
static FObject transformation_translate_fn(const FFunction::param_t& param) {
	Transformation& v = *((Transformation*)param[0]->ptr);
	v.setPosition(*((vec3*)param[1]->ptr));
	return FObject();
}
static FObject aabb_max(var x, var set) {
	AABB& v = *((AABB*)x.ptr);
	if (set) {
		v.max = (*((vec3*)set.ptr));
		return FObject();
	}
	return FObject(&FVec3Type, new vec3(v.max));
}
static FObject aabb_min(var x, var set) {
	AABB& v = *((AABB*)x.ptr);
	if (set) {
		v.min = (*((vec3*)set.ptr));
		return FObject();
	}
	return FObject(&FVec3Type, new vec3(v.min));
}
static FObject aabb_size(const FFunction::param_t& param) {
	return FObject(&FVec3Type, new vec3(((AABB*)param[0]->ptr)->size()));
}
static FObject aabb_center(const FFunction::param_t& param) {
	return FObject(&FVec3Type, new vec3(((AABB*)param[0]->ptr)->center()));
}


FObject Festa::FTransformationType = PirGlobals.addType<Transformation>("transformation");
FObject Festa::FAABBType = PirGlobals.addType<AABB>("aabb");

template<FObject* t, typename T>
static FObject add_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr);
	return FObject(t, new T(va));
}
template<FObject* t, typename T>
static FObject sub_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr);
	return FObject(t, new T(-va));
}
template<FObject* t, typename T>
static FObject T_add_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	return FObject(t, new T(va + vb));
}
template<FObject* t, typename T>
static FObject T_sub_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	return FObject(t, new T(va - vb));
}
template<FObject* t, typename T>
static FObject T_mul_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	return FObject(t, new T(va * vb));
}
template<FObject* t, typename T>
static FObject T_div_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	return FObject(t, new T(va / vb));
}
template<FObject* t, typename T>
static FObject T_addeq_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	va += vb;
	return FObject();
}
template<FObject* t, typename T>
static FObject T_subeq_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	va -= vb;
	return FObject();
}
template<FObject* t, typename T>
static FObject T_muleq_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	va *= vb;
	return FObject();
}
template<FObject* t, typename T>
static FObject T_diveq_T(FObject a, FObject b) {
	T& va = *((T*)a.ptr), & vb = *((T*)b.ptr);
	va /= vb;
	return FObject();
}

static FObject int_pow_int(FObject a, FObject b) {
	int& va = *((int*)a.ptr), & vb = *((int*)b.ptr);
	return FObject(&FIntType, new int(fastpow(va, vb)));
}

static FObject float_pow_float(FObject a, FObject b) {
	float& va = *((float*)a.ptr), & vb = *((float*)b.ptr);
	return FObject(&FFloatType, new float(powf(va, vb)));
}
void PiroofGlobals::loadOperatorCallbacks() {
	callbacks["+int"] = add_T<&FIntType, int>;
	callbacks["-int"] = sub_T<&FIntType, int>;
	callbacks["int+int"] = T_add_T<&FIntType, int>;
	callbacks["int-int"] = T_sub_T<&FIntType, int>;
	callbacks["int*int"] = T_mul_T<&FIntType, int>;
	callbacks["int/int"] = T_div_T<&FIntType, int>;
	callbacks["int^int"] = int_pow_int;
	callbacks["int+=int"] = T_addeq_T<&FIntType, int>;
	callbacks["int-=int"] = T_subeq_T<&FIntType, int>;
	callbacks["int*=int"] = T_muleq_T<&FIntType, int>;
	callbacks["int/=int"] = T_diveq_T<&FIntType, int>;

	callbacks["+float"] = add_T<&FFloatType, float>;
	callbacks["-float"] = sub_T<&FFloatType, float>;
	callbacks["float+float"] = T_add_T<&FFloatType, float>;
	callbacks["float-float"] = T_sub_T<&FFloatType, float>;
	callbacks["float*float"] = T_mul_T<&FFloatType, float>;
	callbacks["float/float"] = T_div_T<&FFloatType, float>;
	callbacks["float^float"] = float_pow_float;
	callbacks["float+=float"] = T_addeq_T<&FFloatType, float>;
	callbacks["float-=float"] = T_subeq_T<&FFloatType, float>;
	callbacks["float*=float"] = T_muleq_T<&FFloatType, float>;
	callbacks["float/=float"] = T_diveq_T<&FFloatType, float>;

	callbacks["+vec2"] = add_T<&FVec2Type, vec2>;
	callbacks["-vec2"] = sub_T<&FVec2Type, vec2>;
	callbacks["vec2+vec2"] = T_add_T<&FVec2Type, vec2>;
	callbacks["vec2-vec2"] = T_sub_T<&FVec2Type, vec2>;
	callbacks["vec2*vec2"] = T_mul_T<&FVec2Type, vec2>;
	callbacks["vec2/vec2"] = T_div_T<&FVec2Type, vec2>;

	callbacks["+vec3"] = add_T<&FVec3Type, vec3>;
	callbacks["-vec3"] = sub_T<&FVec3Type, vec3>;
	callbacks["vec3+vec3"] = T_add_T<&FVec3Type, vec3>;
	callbacks["vec3-vec3"] = T_sub_T<&FVec3Type, vec3>;
	callbacks["vec3*vec3"] = T_mul_T<&FVec3Type, vec3>;
	callbacks["vec3/vec3"] = T_div_T<&FVec3Type, vec3>;

	callbacks["+vec4"] = add_T<&FVec4Type, vec4>;
	callbacks["-vec4"] = sub_T<&FVec4Type, vec4>;
	callbacks["vec4+vec4"] = T_add_T<&FVec4Type, vec4>;
	callbacks["vec4-vec4"] = T_sub_T<&FVec4Type, vec4>;
	callbacks["vec4*vec4"] = T_mul_T<&FVec4Type, vec4>;
	callbacks["vec4/vec4"] = T_div_T<&FVec4Type, vec4>;
}

void PiroofGlobals::initTkmp() {
	tkmp = {
		{"=",EQ},
		{"$", COPY},
		{".",GETATTR},
		{":",MAPPING},
		{":{",CODE_BLOCK},

		{"+",ADD},
		{"-",SUB},
		{"*",MUL},
		{"/",DIV},
		{"^",POW},
		{"+=",ADD_EQ},
		{"-=",SUB_EQ},
		{"*=",MUL - EQ},
		{"/=",DIV_EQ},
		{"^=",POW_EQ},

		{"<",LT},
		{">",GT},
		{"<=",LE},
		{">=",GE},
		{"!=",NEQ},

		{"not",NOT},
		{"or",OR},
		{"and",AND},

		{"function",FUNCTION},

		{"if",IF},
		{"elif",ELIF},
		{"else",ELSE},
		{"for",FOR},
		{"while",WHILE},
		{"break",BREAK},
		{"return",RETURN},	
		{"@",FINPUTS},
		{"continue",CONTINUE},

		//{"delete",DELETE},
		{"macro",MACRO},
	};
}

#define OP_NONE 0
#define OP_PRE 1
#define OP_MID 2
#define OP_POST 4
#define OP_STATEMENT 8

void PiroofGlobals::initGlobals() {
	globals.insert({ "null", FObject() });
	globals.insert({ "true", FObject(&FBoolType, new bool(true)) });
	globals.insert({ "false", FObject(&FBoolType, new bool(false)) });

	addFunction("print", FFunction{print_fn});

	//std::cout << (FType*)(FVec2Type.ptr) << '\n';
	((FType*)(FVec2Type.ptr))->addAttr("x", vec_x<vec2>);
	((FType*)(FVec2Type.ptr))->addAttr("y", vec_y<vec2>);
	((FType*)(FVec3Type.ptr))->addAttr("x", vec_x<vec3>);
	((FType*)(FVec3Type.ptr))->addAttr("y", vec_y<vec3>);
	((FType*)(FVec3Type.ptr))->addAttr("z", vec_z<vec3>);
	((FType*)(FVec4Type.ptr))->addAttr("x", vec_x<vec4>);
	((FType*)(FVec4Type.ptr))->addAttr("y", vec_y<vec4>);
	((FType*)(FVec4Type.ptr))->addAttr("x", vec_z<vec4>);
	((FType*)(FVec4Type.ptr))->addAttr("y", vec_w<vec4>);
	addFunction("vec2",FFunction{vec2_fn,std::vector<FFunction::Parameter>{{&FFloatType},{&FFloatType}}});
	addFunction("vec3", FFunction{ vec3_fn,std::vector<FFunction::Parameter>{{&FFloatType},{&FFloatType},{&FFloatType}} });
	addFunction("vec4", FFunction{ vec4_fn,std::vector<FFunction::Parameter>{{&FFloatType},{&FFloatType},{&FFloatType},{&FFloatType}} });

	addFunction("fscript", FFunction{ script_fn, std::vector<FFunction::Parameter>{{&FStringType} }});
	((FType*)(FScriptType.ptr))->_attr = script_attr;
	addFunction("fclass", FFunction{ class_fn, std::vector<FFunction::Parameter>{{&FStringType} } });

	((FType*)(FTransformationType.ptr))->addAttr("pos", transformation_pos);
	addFunction("Transformation", FFunction{ transformation_fn, std::vector<FFunction::Parameter>{} });
	((FType*)(FTransformationType.ptr))->addMethod("translate", FFunction{transformation_translate_fn,std::vector<FFunction::Parameter>{{&FTransformationType},{&FVec3Type}} });

	((FType*)(FAABBType.ptr))->addAttr("min", aabb_min);
	((FType*)(FAABBType.ptr))->addAttr("max", aabb_max);
	((FType*)(FAABBType.ptr))->addMethod("size", FFunction{ aabb_size, std::vector<FFunction::Parameter>{{&FAABBType} } });
	((FType*)(FAABBType.ptr))->addMethod("center", FFunction{ aabb_center, std::vector<FFunction::Parameter>{{&FAABBType} } });
}

void PiroofGlobals::release() {
	FTypeType.release();
	for (FObject& i : garbage)i.release();
	for (auto& i : globals)i.second.release();
}

void PiroofGlobals::initOperators() {
	operators.resize(MAX_TOKEN + 1);
	operators[NONE] = { -1 };
	operators[OBJECT] = { 100,OP_PRE };
	operators[NUMBER] = { 100,0,false,"" };
	operators[STRING] = { 100,OP_PRE };
	operators[FNCALL] = { 100 };
	operators[MAPPING] = { 2,OP_MID };
	operators[CODE_BLOCK] = { 100 };
	operators[MACRO] = { 3,OP_PRE };
	operators[COPY] = { 100,OP_PRE };
	operators[IF] = { 1, OP_PRE | OP_MID };
	operators[ELIF] = { 1,OP_PRE };
	operators[ELSE] = { 1,OP_PRE };
	operators[FOR] = { 1,OP_PRE };
	operators[WHILE] = { 1,OP_PRE };
	operators[BREAK] = operators[CONTINUE] = { 100,OP_STATEMENT };
	operators[FINPUTS] = { 1,OP_PRE | OP_STATEMENT };
	operators[RETURN] = { 0,OP_PRE | OP_STATEMENT };
	//operators[YIELD] = { 100,OP_STATEMENT };

	operators[ADD] = { 10,OP_MID | OP_PRE,false,"+" };
	operators[SUB] = { 10,OP_MID | OP_PRE,false,"-" };
	operators[MUL] = { 11,OP_MID,false,"*" };
	operators[DIV] = { 11,OP_MID,false,"/" };
	operators[POW] = { 12,OP_MID,true,"^" };
	operators[ADD_EQ] = { 2,OP_MID,false,"+=" };
	operators[SUB_EQ] = { 2,OP_MID,false,"-=" };
	operators[MUL_EQ] = { 2,OP_MID,false,"*=" };
	operators[DIV_EQ] = { 2,OP_MID,false,"/=" };
	operators[POW_EQ] = { 2,OP_MID,false,"^=" };

	operators[EQ] = { 2,OP_MID,false,"=" };
	operators[GETITEM] = { 100 };
	operators[LT] = { 8,OP_MID,false,"<" };
	operators[GT] = { 8,OP_MID,false,">" };
	operators[LE] = { 8,OP_MID,false,"<=" };
	operators[GE] = { 8,OP_MID,false,">=" };
	operators[NEQ] = { 8,OP_MID,false,"!=" };

	operators[NOT] = { 6, OP_PRE, false,"not" };
	operators[AND] = { 5,OP_MID,false,"and" };
	operators[OR] = { 4,OP_MID,false,"or" };
	//operators[DELETE] = { 100,OP_PRE };
	operators[GETATTR] = { 100, OP_MID };
	operators[FUNCTION] = { 1,OP_PRE };
	operators[COMMA] = { 2 };
	operators[MAX_TOKEN] = { 2 };
}

FObject PiroofGlobals::_operate(int8 tk, FObject a, FObject b, std::string& fname) {
	//std::cout << "operate " << int(tk) << endl;
	if (!a) {
		Pir_raise("Null object while operating");
		return FObject(&FTypeType, 0);
	}
	FType* ta = FType::Ftype(a);
	PiroofGlobals::Operator& op = getop(tk);
	if (b) {
		FType* tb = FType::Ftype(b);
		fname = ta->name + op.name + tb->name;
	}
	else if (op.m & OP_POST)
		fname = ta->name + op.name;
	else if (op.m & OP_PRE)
		fname = op.name + ta->name;
	else return FObject(&FTypeType, 0);
	//cout << "fname " << fname << endl;
	PiroofGlobals::opcallback opcb = getcallback(fname);
	return opcb ? opcb(a, b) : FObject(&FTypeType, 0);
}

FObject PiroofGlobals::operate(int8 tk, FObject a) {
	std::string fname;
	FObject ret = _operate(tk, a, FObject(), fname);
	if (ret.ptr || !ret.type)return ret;
	if (a.father) {
		ret = _operate(tk, *a.father, FObject(), fname);
		if (ret.ptr || !ret.type)return ret;
	}
	Pir_raise("Cannot find callback function \"" + fname + "\"");
	return FObject();
}


FObject PiroofGlobals::operate(int8 tk, FObject a, FObject b) {
	std::string fname;
	FObject ret = _operate(tk, a, b, fname);
	if (ret.ptr || !ret.type)return ret;
	if (a.father) {
		ret = _operate(tk, *a.father, b, fname);
		if (ret.ptr || !ret.type)return ret;
		if (b.father) {
			ret = _operate(tk, a, *b.father, fname);
			if (ret.ptr || !ret.type)return ret;
		}
	}
	if (b.father) {
		ret = _operate(tk, a, *b.father, fname);
		if (ret.ptr || !ret.type)return ret;
	}
	Pir_raise("Cannot find callback function \"" + fname + "\"");
	return FObject();
}

