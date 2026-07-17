#include "Vacuum.h"
#include "Tutorial.h"

#define BLOCHSPHERE_RADIUS 0.6f

VAO DISC_VAO;
VAO BlochSphereLines;
VAO BlochSphereSphere;
VAO BlochSphereArrow;
std::vector<QuantumGate> CircuitEntity::mGateClasses;

static void DrawJointLine(const ivec2& pos, int minL, int maxL) {
	const float length = float(maxL - minL) * GRID_DIM;
	Transformation trans;
	trans.setScale(vec3(0.2f, 0.2f, length));
	trans.setPosition(grid2world(pos) + vec3(0.0f, 0.0f, float(minL + maxL) * 0.5f * GRID_DIM));
	GLSLProgram::activatedProgram->set("model", trans.toMatrix());
	CUBE332.get().Draw();
}

static void DrawJoint(const ivec2& origin) {
	Transformation trans;
	trans.setScale(vec3(0.3f));
	trans.setPosition(grid2world(origin));
	GLSLProgram::activatedProgram->set("model", trans.toMatrix());
	CUBE332.get().Draw();
}

void QuantumGate::createIcon() {
	int col = style & 0b11;
	int shape = style & 0b1100;
	Color color;
	switch (col) {
	case GATESTYLE_BLUE:
		color = Color(0, 0, 255, 255);
		break;
	case GATESTYLE_RED:
		color = Color(255, 50, 50, 255);
		break;
	case GATESTYLE_GREEN:
		color = Color(0, 255, 0, 255);
		break;
	case GATESTYLE_YELLOW:
		color = Color(255, 255, 0, 255);
		break;
	}
	if (style & GATESTYLE_BLOCHSPHERE) {
		color = Color(28, 28, 50, 255);
	}
	const int size = 100;
	const int dim = 80;
	const int margin = (size - dim) / 2;
	Image ico(Color(0, 0, 0, 0), size, size);
	Image* label = ui.getFont().GetImage(text.size()?text:"|_>", Color(255, 255, 255, 255), Color(0, 0, 0, 0));
	float factor = 0.0f;
	switch (shape) {
	case GATESTYLE_SQUARE:
		ico.Rectangle(margin, margin, margin + dim, margin + dim, color);
		ico.Rectangle(margin, margin, margin + dim, margin + dim, Color(255, 255, 255), 1);
		factor = 70.0f / (float)std::max(label->width(), label->height());
		break;
	case GATESTYLE_DISC:
		ico.Circle(size / 2, size / 2, dim/2, color);
		ico.Circle(size / 2, size / 2, dim/2, Color(255, 255, 255), 1);
		factor = 80.0f / glm::length(vec2(float(label->width()), float(label->height())));
		break;
	}
	label->Resize(factor, factor);
	ico.blendImage(label, size / 2 - label->width() / 2, size / 2 - label->height() / 2);
	delete label;
	off = iconMap.Store(ico);
}

bool QuantumGate::renderButton(float x, float y, const vec2& size, bool& hovered) {
	int code = ui.ButtonArea(vec2(x, y), size,
		Color(128, 128, 128, 128), Color(25, 25, 25),Color(255, 255, 255), &QuantumGate::iconMap, off);
	hovered |= code & UI_BUTTON_HOVERED;
	return code == (UI_BUTTON_HOVERED | UI_BUTTON_CLICKED);
}

void GateInstance::RenderBlochSphereBase() {
	GLSLProgram& program = *GLSLProgram::activatedProgram;

	Transformation trans; trans.setPosition(grid2world(origin));
	Color col(28, 28, 50);
	Material mat(MaterialData(col, col, vec3(1.0f), 8.0f));
	mat.bind(program, "material");
	trans.setScale(vec3(0.9f, 1.0f, 0.9f));
	program.set("model", trans.toMatrix());
	CUBE332.get().Draw();
}

void GateInstance::RenderBlochSphere() {
	if (parameters.size() != 2)return;
	Transformation base;
	base.rotate(EulerAngles(-22.5f, -22.5f, 0.0f));

	GLSLProgram& program = *GLSLProgram::activatedProgram;

	activateHighlighted(Color(255, 255, 255));
	Transformation trans;
	trans.transform(base);
	trans.setPosition(grid2world(origin) + vec3(0.0f, 0.6f + BLOCHSPHERE_RADIUS, 0.0f));
	GLSLProgram::activatedProgram->set("model", trans.toMatrix());

	glLineWidth(5);
	BlochSphereLines.Draw(GL_LINES);

	if (parameters[0].magnitudeSquared() + parameters[1].magnitudeSquared() > 0.5f) {
		Transformation arrowTrans = base;
		const float phi_deg = (1.0f - parameters[0].magnitudeSquared()) * 180.0f;
		arrowTrans.rotate(Rotation(-phi_deg, base.z));
		const float theta = parameters[0].phase() - parameters[1].phase();
		arrowTrans.rotate(Rotation(glm::degrees(theta), base.y));
		arrowTrans.setPosition(trans.w);

		//GLSLProgram::activatedProgram->set("model", arrowTrans.toMatrix());
		//activateHighlighted(Color(255, 255, 128, 255));
		//BlochSphereArrow.Draw();

		program.bind();
		Color col(0, 255, 127);
		Material mat(MaterialData(col, col, vec3(1.0f), 8.0f));
		mat.bind(program, "material");
		program.set("model", arrowTrans.toMatrix());
		BlochSphereArrow.Draw();
	}

	activateHighlighted(Color(255, 255, 255, 64));
	GLSLProgram::activatedProgram->set("model", trans.toMatrix());
	BlochSphereSphere.Draw();
	
	program.bind();
}

void GateInstance::RenderBase() {
	if (!gate || !GLSLProgram::activatedProgram)return;
	GLSLProgram& program = *GLSLProgram::activatedProgram;
	if (id == GATE_BLOCHSPHERE) {
		RenderBlochSphereBase();
		return;
	}
	int col = gate->style & 0b11;
	Color color;
	switch (col) {
	case GATESTYLE_BLUE:
		color = Color(0, 0, 255);
		break;
	case GATESTYLE_RED:
		color = Color(255, 50, 50);
		break;
	case GATESTYLE_GREEN:
		color = Color(0, 255, 0);
		break;
	case GATESTYLE_YELLOW:
		color = Color(255, 255, 0);
		break;
	}
	int shape = gate->style & 0b1100;
	Transformation trans; trans.setPosition(grid2world(origin));
	Material mat(MaterialData(color, color, vec3(1.0f), 8.0f));
	mat.bind(program, "material");
	switch (shape) {
	case GATESTYLE_SQUARE:
		trans.setScale(vec3(0.9f, 1.0f, 0.9f));
		program.set("model", trans.toMatrix());
		CUBE332.get().Draw();
		break;
	case GATESTYLE_DISC:
		trans.setScale(vec3(0.5f, 0.5f, 0.5f));
		program.set("model", trans.toMatrix());
		DISC_VAO.Draw();
		break;
	}
	if (joints.size()) {
		int minL = 0;
		int maxL = 0;
		Material mat_joint(MaterialData(vec3(1.0f), vec3(1.0f), vec3(1.0f), 16.0f));
		mat_joint.bind(program, "material");
		for (int y : joints) {
			if (y != INT_MAX) {
				minL = std::min(minL, y - origin.y);
				maxL = std::max(maxL, y - origin.y);
				DrawJoint(ivec2(origin.x, y));
			}
		}
		mat.bind(program, "material");
		DrawJointLine(origin, minL, maxL);
	}
}

void GateInstance::RenderLabel() {
	if (!gate || !GLSLProgram::activatedProgram)return;
	GLSLProgram& program = *GLSLProgram::activatedProgram;
	Transformation trans;
	int shape = gate->style & 0b1100;
	generateLabel();
	Text3D* l = label ? label : gate->label;
	if (!l)return;
	trans.x = vec3(1.0f, 0.0f, 0.0f);
	trans.y = vec3(0.0f, 0.0f, -1.0f);
	trans.z = vec3(0.0f, 1.0f, 0.0f);

	float dist = shape == GATESTYLE_SQUARE ? std::max(l->dim.x, l->dim.y) : glm::length(vec2(l->dim));
	vec3 scale = l->dim * 0.8f / dist;
	scale.z = 1.0f;
	trans.scale(scale);

	trans.w = grid2world(origin) + vec3(0.0f, 0.5005f, 0.0f);

	l->col = vec3(1.0f);
	l->Render(trans);
	program.bind();

	if (id == GATE_BLOCHSPHERE) {
		RenderBlochSphere();
		return;
	}
}

void GateInstance::Render() {
	RenderBase();
	RenderLabel();
}

typedef QuantumEntanglement::QuantumCalculator QCalculator;

void performer_fn(QCalculator& calculator, const std::vector<complex>& parameters) {
	assert(parameters.size() == 2);
	bool f0 = parameters[0].real > 0.1f;
	bool f1 = parameters[1].real > 0.1f;
	for (const QCalculator::pair_t& p : calculator.inputs) {
		const ComplexVector& v = p.second;
		assert(v.dim() == 2);
		ComplexVector fv; fv.zeros(2);
		if (f0 == f1) {
			fv[f0 ? 1 : 0] = 1.0f;
		}
		else {
			fv[0] = v[f1 ? 0 : 1];
			fv[1] = v[f1 ? 1 : 0];
		}
		calculator.SaveVector(p.first, fv);
	}
}

void oracle_fn(QCalculator& calculator, const std::vector<complex>& parameters) {
	assert(parameters.size() == 4);
	bool f0 = parameters[0].real > 0.1f;
	bool f1 = parameters[1].real > 0.1f;
	for (const QCalculator::pair_t& p : calculator.inputs) {
		const ComplexVector& v = p.second;
		for (int i = 0; i < v.dim(); i++)std::cout << v[i] << " "; std::cout << "\n";
		assert(v.dim() == 4);
		ComplexVector fv; fv.zeros(4);
		fv[int(f0)] = v[0];
		fv[int(!f0)] = v[1];
		fv[2 + int(f1)] = v[2];
		fv[2 + int(!f1)] = v[3];
		for (int i = 0; i < v.dim(); i++)std::cout << fv[i] << " "; std::cout << "\n";
		calculator.SaveVector(p.first, fv);
	}
}

void CircuitEntity::InitGates() {
	if (mGateClasses.size())return;
	const complex F = sqrtf(0.5f);
	mGateClasses.resize(NUMGATES);
	mGateClasses[GATE_0] = { GATESTYLE_RED | GATESTYLE_DISC | GATESTYLE_INPUT, 0, "|0>", "|0> Qubit", ComplexMatrix(), 0, {}, Qubit(1.0f, 0.0f) };
	mGateClasses[GATE_1] = { GATESTYLE_RED | GATESTYLE_DISC | GATESTYLE_INPUT, 0, "|1>", "|1> Qubit", ComplexMatrix(), 0, {}, Qubit(0.0f, 1.0f) };
	mGateClasses[GATE_INPUT] = { GATESTYLE_RED | GATESTYLE_DISC | GATESTYLE_INPUT, 0, "Input", "Input Gate", ComplexMatrix(), 0, 
		{{"Alpha", QuantumGate::PARAM_COMPLEX, 1}, {"Beta", QuantumGate::PARAM_COMPLEX, 1}} };
	mGateClasses[GATE_OUTPUT] = { GATESTYLE_RED | GATESTYLE_DISC | GATESTYLE_OUTPUT, 0, "Output", "Output Gate", ComplexMatrix(), 0,
		{{"Alpha", QuantumGate::PARAM_COMPLEX, 2}, {"Beta", QuantumGate::PARAM_COMPLEX, 2}} };
	mGateClasses[GATE_MEASURE] = { GATESTYLE_YELLOW | GATESTYLE_DISC | GATESTYLE_MEASURE, 0, "|_>", "Measurement", ComplexMatrix(), 0, 
		{{"Alpha", QuantumGate::PARAM_COMPLEX, 2}, {"Alpha", QuantumGate::PARAM_COMPLEX, 2}} };
	mGateClasses[GATE_BLOCHSPHERE] = { GATESTYLE_OUTPUT | GATESTYLE_BLOCHSPHERE, 0, "Bloch Sphere", "Bloch Sphere Illustrator", ComplexMatrix(), 0,
		{{"Alpha", QuantumGate::PARAM_COMPLEX, 2}, {"Beta", QuantumGate::PARAM_COMPLEX, 2}} };

	mGateClasses[GATE_X] = { GATESTYLE_BLUE | GATESTYLE_SQUARE, 0, "X", "X Gate",
		ComplexMatrix({
			0, 1,
			1, 0
		},
	2, 2)};
	mGateClasses[GATE_Y] = { GATESTYLE_BLUE | GATESTYLE_SQUARE, 0, "Y", "Y Gate",
	ComplexMatrix({
			0.0f, complex(0, -1),
			complex(0, 1), 0.0f
		},
	2, 2) };
	mGateClasses[GATE_Z] = { GATESTYLE_BLUE | GATESTYLE_SQUARE, 0, "Z", "Z Gate",
	ComplexMatrix({
			1, 0,
			0, -1
		},
	2, 2) };
	mGateClasses[GATE_HADAMARD] = { GATESTYLE_BLUE | GATESTYLE_SQUARE, 0, "H", "Hadamard Gate", 
		ComplexMatrix({
			F, F,
			F, -F
		},
	2, 2)};
	mGateClasses[GATE_PERFORMER] = { GATESTYLE_GREEN | GATESTYLE_SQUARE, 0, "Pf", "Performer", ComplexMatrix(),performer_fn,
		{{"f(0)", QuantumGate::PARAM_BIT}, {"f(1)", QuantumGate::PARAM_BIT}} };
	mGateClasses[GATE_ORACLE] = { GATESTYLE_GREEN | GATESTYLE_SQUARE, 1, "Uf", "Oracle", ComplexMatrix(),oracle_fn,
		{{"f(0)", QuantumGate::PARAM_BIT}, {"f(1)", QuantumGate::PARAM_BIT}} };
	mGateClasses[GATE_CNOT] = { GATESTYLE_BLUE | GATESTYLE_DISC, 1, "+", "CNOT Gate",
		ComplexMatrix({
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 1,
			0, 0, 1, 0
		},
	4, 4) };


	Mesh discMesh; discMesh.Cylinder(20);
	DISC_VAO.LoadMesh(discMesh);

	int n = 25;
	std::vector<vec3> vertices(6+n*2);
	uint i = 0;
	const float deltaTheta = PI * 2.0f / float(n);
	vertices[i++] = vec3(BLOCHSPHERE_RADIUS, 0.0f, 0.0f);
	vertices[i++] = vec3(-BLOCHSPHERE_RADIUS, 0.0f, 0.0f);
	vertices[i++] = vec3(0.0f, BLOCHSPHERE_RADIUS, 0.0f);
	vertices[i++] = vec3(0.0f, -BLOCHSPHERE_RADIUS, 0.0f);
	vertices[i++] = vec3(0.0f, 0.0f, BLOCHSPHERE_RADIUS);
	vertices[i++] = vec3(0.0f, 0.0f, -BLOCHSPHERE_RADIUS);
	float theta = 0.0f;
	while (n--) {
		vertices[i++] = vec3(cosf(theta) * BLOCHSPHERE_RADIUS, 0.0f, sinf(theta) * BLOCHSPHERE_RADIUS);
		theta += deltaTheta*0.5f;
		vertices[i++] = vec3(cosf(theta) * BLOCHSPHERE_RADIUS, 0.0f, sinf(theta) * BLOCHSPHERE_RADIUS);
		theta += deltaTheta * 0.5f;
	}
	BlochSphereLines.Init(vertices, "3p");

	Mesh BlochSphereSphereMesh; BlochSphereSphereMesh.Sphere();
	BlochSphereSphereMesh.transform(Transformation::scaling(vec3(BLOCHSPHERE_RADIUS)));
	BlochSphereSphere.LoadMesh(BlochSphereSphereMesh);

	Mesh BlochSphereArrowMesh; BlochSphereArrowMesh.Cylinder(16, true);
	const float height = BLOCHSPHERE_RADIUS*0.7f;
	BlochSphereArrowMesh.transform(Transformation::scaling(vec3(0.05f, height, 0.05f)));

	Mesh cone; cone.Cone(16);
	Transformation coneTrans;
	coneTrans.setScale(vec3(0.1f, BLOCHSPHERE_RADIUS*0.3f, 0.1f));
	coneTrans.setPosition(vec3(0.0f, height, 0.0f));
	cone.transform(coneTrans);

	BlochSphereArrowMesh += cone;
	BlochSphereArrow.LoadMesh(BlochSphereArrowMesh);
	
}

Button3D::ButtonStyle Button3D::defaultButtonStyle = {
	Color(255, 255, 255),
	Color(0, 0, 255),
	Color(128, 128, 255),
	Color(0, 0, 128),
	Color(0, 0, 50),
	false
};