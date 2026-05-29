#pragma once
#include <NtlCommon.h>
#include <vector>
#include <set>
#include <NTL/mat_GF2E.h>
#include <NTL/mat_GF2.h>

// Linear polynomial L(x)= a[d] x^{2^(d)}  + a[d-1] x^{2^(d-1)}+ ... + a[0] x
class LinPolyGF2E
{
public:
	LinPolyGF2E(long iniDeg = 0);
	LinPolyGF2E(std::vector<NTL::GF2E> poly);
	LinPolyGF2E(NTL::mat_GF2 nonsingularMat);

	bool isAllCoeffNonZero();
	std::vector<NTL::GF2E> getCoeff();
	NTL::mat_GF2 getMatGF2ML();
	NTL::mat_GF2E getDicksonMat();
	long getLogDegree();

	void genRandomCoeff(std::set<long> logTargetDeg = {});

	// Evaluate the value of L(x) for a given x
	NTL::GF2E eval(NTL::GF2E x);

	// L^-1 s.t. L * L^-1(x)==x
	LinPolyGF2E getInvPoly();

private:
	void setDicksonMat();
	void setMatGF2ML();

	long logDegree;
	std::vector<NTL::GF2E> coeff;
	NTL::mat_GF2E dicksonMat;
	NTL::mat_GF2 matGF2ML;
	bool isInitialized;
};

// GF2E should be predefined
LinPolyGF2E generateLinPolyGF2E(long targLength);
LinPolyGF2E generateLinPolyGF2E(double rate, long targLength);
