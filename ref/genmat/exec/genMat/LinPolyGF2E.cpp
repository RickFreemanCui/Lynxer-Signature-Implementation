#include "LinPolyGF2E.h"
#include<NTL/ZZ.h>
#include<assert.h>
using namespace std;

NTL_CLIENT


/*
b0,...,bn-1 are basis of GF2E
The power basis mat is:
b0 b1 ... bn-1
b0^p b1^p ... bn-1^p
...
b0^{p^n-1} b1^{p^n-1} ... bn-1^{p^n-1}}

*/
mat_GF2E genPowerBasisMat(long logDegree) {
	mat_GF2E rhsMat;
	rhsMat.SetDims(logDegree, logDegree);
	//row-> exp
	//col-> coeff
	for (auto row = 0; row < logDegree; ++row) {
		ZZ exp = power2_ZZ(row);
		for (auto col = 0; col < logDegree; ++col) {
			rhsMat[row][col] = power(getPowerGF2E(col), exp);
		}
	}
	return rhsMat;
}

mat_GF2E matGF2ToMatGF2E(mat_GF2 inMat) {
	long col = inMat.NumCols();
	long row = inMat.NumCols();
	assert(col == row);
	mat_GF2E resMat;
	resMat.SetDims(row, col);
	for (auto iR = 0; iR < row; ++iR) {
		for (auto iC = 0; iC < col; ++iC) {
			if (IsZero(inMat[iR][iC]))
				resMat[iR][iC] = 0;
			else
				resMat[iR][iC] = 1;
		}
	}
	return resMat;
}

LinPolyGF2E::LinPolyGF2E(long iniDeg)
{
	logDegree = iniDeg;
	coeff = vector<GF2E>(iniDeg, to_GF2E(0));
	isInitialized = false;
}

LinPolyGF2E::LinPolyGF2E(std::vector<NTL::GF2E> poly)
{
	logDegree = poly.size();
	coeff = poly;
	setDicksonMat();
	setMatGF2ML();
	isInitialized = true;
}

LinPolyGF2E::LinPolyGF2E(NTL::mat_GF2 fpMat)
{
	mat_GF2E f2EMat = matGF2ToMatGF2E(fpMat);
	GF2 detVal = determinant(fpMat);
	assert(detVal == 1);
	logDegree = fpMat.NumCols();
	matGF2ML = fpMat;
	coeff = vector<GF2E>(logDegree, to_GF2E(0));
	mat_GF2E dickRhs = genPowerBasisMat(logDegree);
	mat_GF2E dickLhs = inv(dickRhs);
	dicksonMat = dickRhs * f2EMat * dickLhs;
	for (auto idx = 0; idx < logDegree; ++idx) {
		coeff[idx] = dicksonMat[0][idx];
	}
	isInitialized = true;
}

bool LinPolyGF2E::isAllCoeffNonZero()
{
	assert(isInitialized == true);
	bool res = true;
	for (auto id = 0; id < logDegree; ++id) {
		if (IsZero(coeff[id])) {
			res = false;
			break;
		}
	}
	return res;
}

std::vector<NTL::GF2E> LinPolyGF2E::getCoeff()
{
	assert(isInitialized == true);
	return coeff;
}

NTL::mat_GF2 LinPolyGF2E::getMatGF2ML()
{
	assert(isInitialized == true);
	return matGF2ML;
}

NTL::mat_GF2E LinPolyGF2E::getDicksonMat()
{
	assert(isInitialized == true);
	return dicksonMat;
}

long LinPolyGF2E::getLogDegree()
{
	assert(isInitialized == true);
	return logDegree;
}


GF2E LinPolyGF2E::eval(GF2E inX) {
	GF2E res = GF2E(0);
	for (auto mono = 0; mono < coeff.size(); ++mono) {
		ZZ exp = power2_ZZ(mono);
		res += coeff[mono] * power(inX, exp);
	}
	return res;
}

LinPolyGF2E LinPolyGF2E::getInvPoly() {
	assert(isInitialized == true);
	return LinPolyGF2E(inv(matGF2ML));
}


void LinPolyGF2E::genRandomCoeff(std::set<long> targLogDegs)
{
	GF2E detVal;
	do {

		for (auto idx = 0; idx < logDegree; ++idx) {
			if (targLogDegs.empty()) {
				coeff[idx] = random_GF2E();
			}
			else {
				if (targLogDegs.find(idx) != targLogDegs.end()) {
					coeff[idx] = random_GF2E();
				}
				else {
					coeff[idx] = GF2E(0);
				}
			}
		}

		setDicksonMat();
		detVal = determinant(dicksonMat);
	} while (IsZero(detVal));
	setMatGF2ML();
	isInitialized = true;
}

void LinPolyGF2E::setDicksonMat()
{
	mat_GF2E resMat;
	resMat.SetDims(logDegree, logDegree);
	//row-> exp
	//col-> coeff
	for (auto row = 0; row < logDegree; ++row) {
		ZZ exp = power2_ZZ(row);
		for (auto col = 0; col < logDegree; ++col) {
			resMat[row][col] = power(coeff[(col - row + logDegree) % logDegree], exp);
		}
	}
	dicksonMat = resMat;
}

void LinPolyGF2E::setMatGF2ML()
{
	mat_GF2E dickMat = dicksonMat;
	mat_GF2E rhsMat = genPowerBasisMat(logDegree);
	mat_GF2E lhsMat = inv(rhsMat);
	mat_GF2E finalMatOfGF2E = (lhsMat * dickMat * rhsMat);
	//cout << finalMatOfGF2E;
	mat_GF2 resMat;
	resMat.SetDims(logDegree, logDegree);
	for (auto row = 0; row < logDegree; ++row) {
		for (auto col = 0; col < logDegree; ++col) {
			if (!IsZero(finalMatOfGF2E[row][col])) {
				resMat[row][col] = 1;
			}
		}
	}
	matGF2ML = resMat;
	//invMatFp = inv(resMat);
}

LinPolyGF2E generateLinPolyGF2E(long targLength)
{

	//GF2X basePoly = genRandPrimitivePolyOfDeg(targLength);
	//GF2E::init(basePoly);
	//cout << "Base Poly:\n" << basePoly << endl;
	LinPolyGF2E linPoly = LinPolyGF2E(targLength);
	LinPolyGF2E invPoly = LinPolyGF2E(targLength);
	do {
		linPoly.genRandomCoeff();
		invPoly = linPoly.getInvPoly();
		if (linPoly.isAllCoeffNonZero() && invPoly.isAllCoeffNonZero()) {
			break;
		}
	} while (true);
	return linPoly;
}


LinPolyGF2E generateLinPolyGF2E(double rate, long targLength)
{

	//GF2X basePoly = genRandPrimitivePolyOfDeg(targLength);
	//GF2E::init(basePoly);
	//cout << "Base Poly:\n" << basePoly << endl;
	LinPolyGF2E res;
	long iterTime = 0;
	long hwUBond = targLength * rate;
	hwUBond = hwUBond % 2 == 1 ? hwUBond : hwUBond - 1;
	cout << "Low HW Matrix " << dec << hwUBond << "/" << targLength << endl;
	do {
		++iterTime;
		mat_GF2 mm = getInvertableRandomWithHwMatGF2(targLength, hwUBond);
		LinPolyGF2E linPoly = LinPolyGF2E(mm);
		LinPolyGF2E invPoly = linPoly.getInvPoly();
		if (linPoly.isAllCoeffNonZero() && invPoly.isAllCoeffNonZero()) {
			res = linPoly;
			std::cout << "Get Linear Poly after " << dec << iterTime << " iterations\n";
			break;
		}
	} while (true);
	return res;
}
