#pragma once
#include <Festa.hpp>
#include <bitset>
using namespace Festa;

struct complex {
	float real = 0.0f;
	float imaginary = 0.0f;
	complex() {}
	complex(float _real) :real(_real) {}
	complex(float _real, float _imaginary) :real(_real), imaginary(_imaginary) {}
	complex(const vec2& vec) :real(vec.x), imaginary(vec.y) {}
	complex operator+()const {
		return *this;
	}
	complex operator-()const {
		return complex(-real, -imaginary);
	}
	operator vec2()const {
		return vec2(real, imaginary);
	}
	friend float Re(const complex& a) {
		return a.real;
	}
	friend float Im(const complex& a) {
		return a.imaginary;
	}
	friend complex conjugate(const complex& a) {
		return complex(a.real, -a.imaginary);
	}
	friend std::ostream& operator<<(std::ostream& os, const complex& c) {
		os << c.serialize();
		return os;
	}
	std::string serialize()const {
		if (fabsf(imaginary) < 0.001f) {
			return toString(real);
		}
		if (fabsf(real) < 0.001f) {
			return toString(imaginary) + "i";
		}
		return toString(real) + "+" + toString(imaginary) + "i";
	}
	float Re()const {
		return real;
	}
	float Im()const {
		return imaginary;
	}
	complex conjugate()const {
		return complex(real, -imaginary);
	}
	float magnitude()const {
		return sqrtf(magnitudeSquared());
	}
	float magnitudeSquared()const {
		return real * real + imaginary * imaginary;
	}
	float phase()const {
		if (fabsf(imaginary) < EPS_FLOAT)return real >= 0.0f ? 0.0f : PI;
		float p = acosf(real / magnitude());
		return imaginary >= 0.0f ? p : -p;
	}
	friend complex operator+(const complex& a, const complex& b) {
		return complex(a.real + b.real, a.imaginary + b.imaginary);
	}
	friend complex operator-(const complex& a, const complex& b) {
		return complex(a.real - b.real, a.imaginary - b.imaginary);
	}
	friend complex operator*(const complex& a, const complex& b) {
		return complex(a.real * b.real - a.imaginary * b.imaginary, a.real * b.imaginary + a.imaginary * b.real);
	}
	friend complex operator/(const complex& a, const complex& b) {
		float m2 = b.real * b.real + b.imaginary * b.imaginary;
		return complex((a.real * b.real + a.imaginary * b.imaginary) / m2, (-a.real * b.imaginary + a.imaginary * b.real) / m2);
	}
	void operator+=(const complex& a) {
		real += a.real;
		imaginary += a.imaginary;
	}
	void operator-=(const complex& a) {
		real -= a.real;
		imaginary -= a.imaginary;
	}
	void operator*=(const complex& a) {
		*this = *this * a;
	}
	void operator/=(const complex& a) {
		*this = *this / a;
	}
};


class ComplexVector {
public:
	ComplexVector() {

	}
	ComplexVector(const std::vector<complex>& vec) {
		mDim = uint(vec.size());
		mPtr = new complex[mDim];
		for (uint i = 0; i < mDim; i++)mPtr[i] = vec[i];
	}
	ComplexVector(const ComplexVector& vec) {
		mDim = vec.mDim;
		mPtr = new complex[mDim];
		for (uint i = 0; i < mDim; i++)mPtr[i] = vec[i];
	}
	~ComplexVector() {
		release();
	}
	void release() {
		if (mPtr) {
			free(mPtr);
			mPtr = 0;
		}
		mDim = 0;
	}
	void set(const complex& c, uint dim) {
		release();
		mDim = dim;
		mPtr = new complex[mDim];
		for (uint i = 0; i < mDim; i++)mPtr[i] = c;
	}
	void zeros(uint dim) {
		set(0.0f, dim);
	}
	void ones(uint dim) {
		set(1.0f, dim);
	}
	void operator=(const ComplexVector& vec) {
		release();
		mDim = vec.mDim;
		mPtr = new complex[mDim];
		for (uint i = 0; i < mDim; i++)mPtr[i] = vec[i];
	}
	uint dim()const {
		return mDim;
	}
	const complex* ptr()const {
		return mPtr;
	}
	complex* ptr() {
		return mPtr;
	}
	const complex& operator[](uint i)const {
		assert(i < mDim);
		return mPtr[i];
	}
	complex& operator[](uint i) {
		assert(i < mDim);
		return mPtr[i];
	}
	void operator*=(const complex& c) {
		for (uint i = 0; i < mDim; i++)mPtr[i] *= c;
	}
	friend ComplexVector operator*(const complex& c, const ComplexVector& vec) {
		ComplexVector ret = vec;
		ret *= c;
		return ret;
	}
	friend complex innerProduct(const ComplexVector& a, const ComplexVector& b) {
		assert(a.dim() == b.dim());
		complex ret = 0.0f;
		for (uint i = 0; i < a.dim(); i++) {
			ret += a[i].conjugate() * b[i];
		}
		return ret;
	}
	float magnitude()const {
		return sqrtf(magnitudeSquared());
	}
	float magnitudeSquared()const {
		float m2 = 0.0f;
		for (uint i = 0; i < dim(); i++) {
			m2 += mPtr[i].magnitudeSquared();
		}
		return m2;
	}
	ComplexVector conjugate()const {
		ComplexVector ret; ret.zeros(dim());
		for (uint i = 0; i < mDim; i++)ret[i] = mPtr[i].conjugate();
		return ret;
	}
	void operator+=(const ComplexVector& a) {
		assert(a.dim() == dim());
		for (uint i = 0; i < mDim; i++) {
			mPtr[i] += a[i];
		}
	}
	void operator-=(const ComplexVector& a) {
		assert(a.dim() == dim());
		for (uint i = 0; i < mDim; i++) {
			mPtr[i] -= a[i];
		}
	}
	friend ComplexVector operator+(const ComplexVector& a, const ComplexVector& b) {
		assert(a.dim() == b.dim());
		ComplexVector ret = a;
		ret += b;
		return ret;
	}
	friend ComplexVector operator-(const ComplexVector& a, const ComplexVector& b) {
		assert(a.dim() == b.dim());
		ComplexVector ret = a;
		ret -= b;
		return ret;
	}
private:
	complex* mPtr = 0;
	uint mDim = 0;
};

class ComplexMatrix {
public:
	ComplexMatrix() {}
	ComplexMatrix(const std::vector<complex>& mat, uint m, uint n) {
		assert(uint(mat.size()) == m * n);
		mMat.resize(n);
		for (uint j = 0; j < n; j++) {
			mMat[j].set(0.0f, m);
			for (uint i = 0; i < m; i++) {
				mMat[j][i] = mat[i * m + j];
			}
		}
		mM = m;
		mN = n;
	}
	const ComplexVector& column(uint j)const {
		return mMat[j];
	}
	ComplexVector& column(uint j) {
		return mMat[j];
	}
	const ComplexVector& operator[](uint j)const {
		return mMat[j];
	}
	ComplexVector& operator[](uint j) {
		return mMat[j];
	}
	uint m()const {
		return mM;
	}
	uint numRows()const {
		return mM;
	}
	uint n()const {
		return mN;
	}
	uint numColumns()const {
		return mN;
	}
	friend ComplexVector operator*(const ComplexMatrix& mat, const ComplexVector& vec) {
		assert(mat.numRows() == vec.dim());
		ComplexVector ret; ret.zeros(vec.dim());
		for (uint j = 0; j < mat.numColumns(); j++) {
			ret += vec[j] * mat[j];
		}
		return ret;
	}
private:
	std::vector<ComplexVector> mMat;
	uint mM = 0;
	uint mN = 0;
};

struct Qubit {
	complex k0 = 1.0f;
	complex k1 = 0.0f;
	char measurement = -1;
	Qubit() {}
	Qubit(const complex& alpha, const complex& beta) :k0(alpha), k1(beta) {}
	bool hasBeenMeasured()const {
		return measurement != -1;
	}
	bool measure() {
		if (hasBeenMeasured())return measurement;
		if (randf() < k0.magnitudeSquared()) {
			k0 = 1.0f;
			k1 = 0.0f;
			measurement = 0;
			return false;
		}
		else {
			k0 = 0.0f;
			k1 = 1.0f;
			measurement = 1;
			return true;
		}
	}
};

#define MAX_QUBITS 32

class QuantumEntanglement {
public:
	typedef ComplexVector(*quantum_fn)(ComplexVector);
	struct QuantumCalculator {
		typedef pair<ull, ComplexVector> pair_t;
		QuantumEntanglement* qe = 0;
		ull entanglement = 0;
		std::vector<uchar> qubits;
		std::list<pair_t> inputs;
		std::list<ull> powerSet;
		void SaveVector(ull base, const ComplexVector& vec) {
			if (!qe)return;
			qe->LoadVector(entanglement, base, vec, powerSet);
		}
		void Transform(const ComplexMatrix& matrix) {
			if (!qe)return;
			const uint dim = 1 << uchar(qubits.size());
			assert(mat.numColumns() == mat.numRows() && mat.numRows() == dim);
			for (pair<ull, ComplexVector>& p : inputs) {
				ull base = p.first;
				for (uint i = 0; i < dim; i++)std::cout << p.second[i].real << "+" << p.second[i].imaginary << "i "; printf("\n");
				ComplexVector v = matrix * p.second;
				for (uint i = 0; i < dim; i++)std::cout << v[i].real << "+" << v[i].imaginary << "i "; printf("\n");
				SaveVector(base, v);
			}
			//qe->ReleaseEntanglement();
		}
		void Operate(const quantum_fn& fn) {
			if (!qe)return;
			if (!fn)return;
			for (pair<ull, ComplexVector>& p : inputs) {
				ull base = p.first;
				SaveVector(base, fn(p.second));
			}
			//qe->ReleaseEntanglement();
		}
	};
	QuantumEntanglement() {}
	~QuantumEntanglement() {
		Release();
	}
	void Release() {
		if (mPtr) {
			free(mPtr);
			mPtr = 0;
		}
		mQubits.clear();
		mEntanglements.clear();
	}
	uchar numQubits()const {
		return uchar(mQubits.size());
	}
	void Allocate(uchar num) {
		assert(num <= MAX_QUBITS);
		Release();
		mQubits.resize(num, 0);
		mEntanglements.resize(num, 0);
		mPtr = (complex*)malloc(sizeof(complex) * fastpow(3ull, num));
		uchar q = 0;
		ull mask = 1;
		while (q < num) {
			mEntanglements[q] = mask;
			// init qubits to |0>
			Index(0, mask) = 1.0f;
			Index(mask, mask) = 0.0f;
			mask <<= 1;
			q++;
		}
	}
	ull MakeEntangled(const std::vector<uchar>& qubits) {
		ull mask = 0;
		for (uchar q : qubits)mask |= EntangledMask(q);
		if (qubits.size() && EntangledMask(qubits[0]) == mask)return mask;

		std::list<ull> powerSet; PowerSet(powerSet, mask);
		for (ull bits : powerSet) {
			complex prob = 1.0f;
			//std::cout << "mul " << bitset<32>(bits) << "\n";
			for (uchar q : qubits) {
				ull decomposition = EntangledMask(q);
				//std::cout << "decomp " << bitset<32>(decomposition) << " " << Index(bits & decomposition, decomposition).magnitudeSquared() << "\n";
				prob *= Index(bits&decomposition, decomposition);
			}
			Index(bits, mask) = prob;
		}
		const ull entanglement = mask;

		uchar q = 0;
		while (q < numQubits()) {
			if (mask & 1)mEntanglements[q] = entanglement;
			mask >>= 1;
			q++;
		}

		return entanglement;
	}
	ull EntangledMask(uchar qubit)const {
		return mEntanglements[qubit];
	}
	void ReleaseEntanglement(ull entanglement, float eps) {
		ull tmp = entanglement;
		ull qmask = 1;
		uchar q = 0;
		uchar target = numQubits();
		while (q < numQubits()) {
			if (tmp & 1) {
				std::list<ull> items; PowerSet(items, entanglement ^ qmask);
				std::cout << "power " << bitset<32>(entanglement ^ qmask) << "\n";
				const complex k0 = Index(qmask, entanglement) / Index(0, entanglement);
				std::cout << "k0 " << k0 << " "<<bitset<32>(qmask)<<"\n";
				bool brk = true;
				for (ull item : items) {
					complex k = Index(qmask | item, entanglement) / Index(item, entanglement);
					std::cout << "k " << bitset<32>(item)<<" "<<k << " " << (k0 - k).magnitudeSquared() << "\n";
					if ((k0 - k).magnitudeSquared() > eps) {
						brk = false;
						break;
					}
				}
				if (brk) {
					target = q;
					break;
				}
			}
			qmask <<= 1;
			tmp >>= 1;
			q++;
		}
		if (target<numQubits()) {
			ReleaseEntanglement(ReleaseEntanglement(entanglement, target), eps);
		}
	}
	ull ReleaseEntanglement(ull entanglement, uchar qubit) {
		const ull qmask = 1 << qubit;
		const ull newMask = entanglement ^ qmask;
		if (!newMask)return 0;
		//std::cout << "rel " << bitset<32>(entanglement) << " " << int(qubit) << "\n";
		mEntanglements[qubit] = qmask;
		uchar q = 0;
		ull tmp = newMask;
		while (q < numQubits()) {
			if (tmp & 1)mEntanglements[q] = newMask;
			tmp >>= 1;
			q++;
		}
		return newMask;
	}
	void ClearEntanglement(ull entanglement) {
		uchar q = 0;
		ull mask = 1;
		while (q < numQubits()) {
			if (entanglement & 1)mEntanglements[q] = mask;
			mask <<= 1;
			entanglement >>= 1;
			q++;
		}
	}
	ull Measure(uchar qubit) {
		ull entanglement = EntangledMask(qubit);
		std::list<ull> tensor;
		PowerSet(tensor, entanglement);
		float m = randf();
		float prob = 0.0f;
		for (ull index : tensor) {
			prob += Index(index, entanglement).magnitudeSquared();
			std::cout << "m " << m << " " << bitset<32>(index) << " " << prob << '\n';
			if (m <= prob) {
				ClearEntanglement(entanglement);
				CollapseQubits(index, entanglement);
				//clear tensor
				return index;
			}
		}
		ull index = tensor.back();
		ClearEntanglement(entanglement);
		CollapseQubits(index, entanglement);
		return index;
	}
	void GetCalculator(QuantumCalculator& ret, const std::vector<uchar>& qubits) {
		ret.qe = this;
		ret.qubits = qubits;
		assert(qubits.size() < numQubits());
		ret.entanglement = FetchVector(ret.inputs, qubits);
		OrderedPowerSet(ret.powerSet, qubits);
	}
	void Transform(const std::vector<uchar>& qubits, const ComplexMatrix& matrix) {
		QuantumCalculator calculator;
		GetCalculator(calculator, qubits);
		calculator.Transform(matrix);
	}
	ull FetchVector(std::list<pair<ull, ComplexVector>>& vec, const std::vector<uchar>& qubits) {
		ull entanglement = MakeEntangled(qubits);
		ull inputs = 0;
		for (uchar q : qubits)inputs |= (1 << q);
		std::list<ull> nonrelevant; 
		std::list<ull> variables;
		PowerSet(nonrelevant, entanglement ^ inputs);
		OrderedPowerSet(variables, qubits);
		for (ull base : nonrelevant) {
			vec.push_back(std::make_pair(base, ComplexVector()));
			ComplexVector& v = vec.back().second;
			v.set(0.0f, variables.size());
			uint i = 0;
			for (ull mask : variables) {
				v[i++] = Index(base | mask, entanglement);
			}
		}
		return entanglement;
	}
	void LoadVector(ull entanglement, ull base, const ComplexVector& vec, const std::list<ull>& powerSet) {
		uint i = 0;
		for (ull mask : powerSet) {
			Index(base | mask, entanglement) = vec[i++];
		}
	}
	complex& Index(ull bits, ull entanglement) {
		ull i = 0;
		uchar q = 0;
		while (q < numQubits()) {
			i *= 3;
			if (entanglement & 1)i += (bits & 1) ? 1 : 2;
			entanglement >>= 1;
			bits >>= 1;
			q++;
		}
		return mPtr[i];
	}
	const complex& Index(ull bits, ull entanglement)const {
		ull i = 0;
		uchar q = 0;
		while (q < numQubits()) {
			i *= 3;
			if (entanglement & 1)i += (bits & 1) ? 1 : 2;
			entanglement >>= 1;
			bits >>= 1;
			q++;
		}
		return mPtr[i];
	}
	void InitializeQubit(uchar qubit, const Qubit& q) {
		assert(qubit < numQubits());
		ull mask = 1 << qubit;
		Index(0, mask) = q.k0;
		Index(mask, mask) = q.k1;
	}
	void CollapseQubits(ull bits, ull mask) {
		uchar q = 0;
		ull qmask = 1;
		while (q < numQubits()) {
			if (mask & 1) {
				Index(0, qmask) = (bits&1) ? 0.0f : 1.0f;
				Index(qmask, qmask) = (bits & 1) ? 1.0f : 0.0f;
				mQubits[q] = (bits & 1) ? 2 : 1;
			}
			qmask <<= 1;
			bits >>= 1;
			mask >>= 1;
			q++;
		}
	}
	uchar GetState(uchar q)const {
		return mQubits[q];
	}
	void GetSuperposition(uchar q, complex& alpha, complex& beta)const {
		assert(q < numQubits());
		ull qmask = 1 << q;
		alpha = Index(0, qmask);
		beta = Index(qmask, qmask);
	}
	void PowerSet(std::list<ull>& set, ull mask) {
		set.emplace_back(0);
		uchar q = 0;
		uchar x = 0;
		ull qmask = 1;
		while (q < numQubits()) {
			if (mask & 1) {
				ull num = 1 << x;
				auto it = set.begin();
				while (num--) {
					set.emplace_back(qmask | (*it));
					it++;
				}
				x++;
			}
			mask >>= 1;
			qmask <<= 1;
			q++;
		}
	}
	void OrderedPowerSet(std::list<ull>& set, const std::vector<uchar>& qubits) {
		set.emplace_back(0);
		uchar q = 0;
		for (int i = int(qubits.size() - 1); i >= 0;i--) {
			ull qmask = 1 << qubits[i];
			auto it = set.begin();
			ull num = 1 << q;
			while (num--) {
				set.emplace_back(qmask | (*it));
				it++;
			}
			q++;
		}
	}
private:
	//collapsed states
	std::vector<uchar> mQubits;
	complex* mPtr = 0;
	std::vector<ull> mEntanglements;
};