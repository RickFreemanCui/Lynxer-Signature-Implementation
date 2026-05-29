#include "NtlCommon.h"
#include <vector>
#include <map>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
// #include<Common.h>
#include <Common.hpp>
using namespace std;
NTL_CLIENT

class DistinctPrimeRepository
{
public:
	DistinctPrimeRepository()
	{
		// 2^6-1=7*9
		repository[6] = {to_ZZ(7), to_ZZ(9)};
		// 2^8-1=3*5*17
		repository[8] = {to_ZZ(3), to_ZZ(5), to_ZZ(17)};
		// 2^16-1=3 * 5 * 17 * 257
		repository[16] = {to_ZZ(3), to_ZZ(5), to_ZZ(17), to_ZZ(257)};
		// 2^32-1=3 * 5 * 17 * 257 * 65537
		repository[32] = {to_ZZ(3), to_ZZ(5), to_ZZ(17), to_ZZ(257), to_ZZ(65537)};
		// 2^64-1=3 * 5 * 17 * 257 * 641 * 65537 * 6700417
		repository[64] = {to_ZZ(3), to_ZZ(5), to_ZZ(17), to_ZZ(257), to_ZZ(641), to_ZZ(65537), to_ZZ(6700417)};
		// 2^96-1=3^2 * 5 * 7 * 13 * 17 * 97 * 193 * 241 * 257 * 673 * 65537 * 22253377
		repository[96] = {to_ZZ(3), to_ZZ(5), to_ZZ(7), to_ZZ(13), to_ZZ(17), to_ZZ(97), to_ZZ(193), to_ZZ(241), to_ZZ(257), to_ZZ(673), to_ZZ(65537), to_ZZ(22253377)};
		// 2^128-1=3 * 5 * 17 * 257 * 641 * 65537 * 274177 * 6700417 * 67280421310721
		repository[128] = {
			to_ZZ(3), to_ZZ(5), to_ZZ(17), to_ZZ(257), to_ZZ(641),
			to_ZZ(65537), to_ZZ(274177), to_ZZ(6700417),
			to_ZZ("67280421310721")};
		// 2^256-1=3 * 5 * 17 * 257 * 641 * 65537 * 274177 * 6700417 * 67280421310721 * 59649589127497217 * 5704689200685129054721
		repository[256] = {
			to_ZZ(3), to_ZZ(5), to_ZZ(17), to_ZZ(257), to_ZZ(641),
			to_ZZ(65537), to_ZZ(274177), to_ZZ(6700417),
			to_ZZ("67280421310721"),
			to_ZZ("59649589127497217"),
			to_ZZ("5704689200685129054721")};
		// 2^384-1=3^2 * 5 * 7 * 13 * 17 * 97 * 193 * 241 * 257 * 641 * 673 * 769 * 65537
		//* 274177 * 6700417 * 22253377 * 67280421310721 * 18446744069414584321 * 442499826945303593556473164314770689
		repository[384] = {
			to_ZZ(3), to_ZZ(5), to_ZZ(7), to_ZZ(13), to_ZZ(17), to_ZZ(97),
			to_ZZ(193),
			to_ZZ(241),
			to_ZZ(257),
			to_ZZ(641),
			to_ZZ(673),
			to_ZZ(769),
			to_ZZ(65537),
			to_ZZ(274177),
			to_ZZ(6700417),
			to_ZZ("22253377"),
			to_ZZ("67280421310721"),
			to_ZZ("18446744069414584321"),
			to_ZZ("442499826945303593556473164314770689")};
		// 2^512-1=3 * 5 * 17 * 257 * 641 * 65537 * 274177 * 6700417 * 67280421310721 * 1238926361552897 * 59649589127497217 * 5704689200685129054721 * 93461639715357977769163558199606896584051237541638188580280321
		repository[512] = {
			to_ZZ(3), to_ZZ(5), to_ZZ(17), to_ZZ(257), to_ZZ(641),
			to_ZZ(65537), to_ZZ(274177), to_ZZ(6700417),
			to_ZZ("67280421310721"),
			to_ZZ("1238926361552897"),
			to_ZZ("59649589127497217"),
			to_ZZ("5704689200685129054721"),
			to_ZZ("93461639715357977769163558199606896584051237541638188580280321")};
	}

	bool isCapable(int bitLen)
	{
		return (repository.find(bitLen) != repository.end());
	}
	vector<ZZ> operator[](const int i)
	{
		return repository[i];
	}

protected:
	map<int, vector<ZZ>> repository;
};
DistinctPrimeRepository DISTINCT_PRIME_REPOSITORY = DistinctPrimeRepository();

bool testPrimitiveGF2X(NTL::GF2X poly)
{
	if (!IterIrredTest(poly))
		return false;
	GF2E::init(poly);
	GF2X x;
	SetCoeff(x, 1);
	GF2E xsub = to_GF2E(x);
	bool isPrimitivePolynomial = true;
	long length = deg(poly);
	vector<ZZ> distinctPrimes = DISTINCT_PRIME_REPOSITORY[length];
	ZZ cycle = power2_ZZ(length) - 1;
	for (int i = 0; i < distinctPrimes.size(); ++i)
	{
		GF2E tmpRes = power(xsub, (cycle / distinctPrimes[i]));
		GF2X tmp32X = rep(tmpRes);
		if (deg(tmp32X) < 1)
		{
			isPrimitivePolynomial = false;
		}
	}
	return isPrimitivePolynomial;
}

NTL::GF2X genRandPrimitivePolyOfDeg(long targDeg)
{
	assert(DISTINCT_PRIME_REPOSITORY.isCapable(targDeg) == true);
	bool isPrimitive = false;
	GF2X poly;
	long repeatTime = 0;
	do
	{
		repeatTime++;
		poly = random_GF2X(targDeg);
		SetCoeff(poly, targDeg);
		SetCoeff(poly, 0);
		isPrimitive = testPrimitiveGF2X(poly);
	} while (!isPrimitive);
	return poly;
}

NTL::GF2E getPowerGF2E(long e)
{
	GF2X myRep;
	SetCoeff(myRep, e);
	ostringstream oss;
	oss << (myRep);
	GF2E res;
	stringstream iss(oss.str());
	iss >> res;
	return res;
}

NTL::GF2E getRandomWithHwGF2E(int degr, const int hw)
{
	std::set<int> onePos = randomPick(degr, hw);
	GF2E res = to_GF2E(0);
	for (int val : onePos)
	{
		cout<<val<<endl;
		res += getPowerGF2E(val);
	}
	return res;
}

NTL::mat_GF2 getRandomWithHwMatGF2(const long dim, const int hw)
{
	mat_GF2 mm;
	mm.SetDims(dim, dim);
	for (auto row = 0; row < dim; ++row)
	{
		std::set<int> onePos = randomPick(dim, hw);
		for (int pos : onePos)
		{
			mm[row][pos] = to_GF2(1);
		}
	}
	return mm;
}
// NTL::mat_GF2 getRandomWithHwMatGF2(const long dim, const int hw,) {
//	mat_GF2 mm;
//	mm.SetDims(dim, dim);
//	for (auto row = 0; row < dim; ++row) {
//		std::set<int> onePos = randomPick(dim, hw);
//		for (int pos : onePos) {
//			mm[row][pos] = to_GF2(1);
//		}
//	}
//	return mm;
// }

NTL::mat_GF2 getInvertableRandomWithHwMatGF2(const long dim, const int hw)
{
	mat_GF2 mm;
	GF2 detVal = to_GF2(0);
	long iteTime = 0;
	std::set<int> remaining;
	for (auto i = 0; i < dim; ++i)
		remaining.insert(i);
	do
	{
		++iteTime;
		diag(mm, dim, to_GF2(0));
		std::set<int> usedIndx;
		for (auto row = 0; row < dim - 1; ++row)
		{
			std::set<int> onePos = randomPick(dim, hw);
			usedIndx.insert(onePos.begin(), onePos.end());
			for (int pos : onePos)
			{
				mm[row][pos] = to_GF2(1);
			}
		}
		if (dim - usedIndx.size() >= hw)
		{
			continue;
		}
		else
		{
			for (int ele : usedIndx)
			{
				remaining.erase(ele);
			}
			std::set<int> onePos = randomPick(dim, hw, remaining);
			for (int pos : onePos)
			{
				mm[dim - 1][pos] = to_GF2(1);
			}
		}
		detVal = determinant(mm);
	} while (detVal == to_GF2(0));
	std::cout << dec << "Get Inversible mat_GF2 with rate " << hw << "/" << dim << " after " << iteTime << " iterations!\n";

	return mm;
}

NTL::GF2E getExpDefGF2E(std::vector<long> exps)
{
	GF2E res(0);
	for (auto idx = 0; idx < exps.size(); ++idx)
	{
		assert(exps[idx] >= 0);
		res += getPowerGF2E(exps[idx]);
	}
	return res;
}

// bool checkBitConditions(vector<BitEquation*> allEqns)
// {

// 	std::set<BitNode*> involvBits;
// 	map<BitNode*, int> bit2Col;
// 	for (auto eq : allEqns) {
// 		involvBits.insert(eq->lhs.begin(), eq->lhs.end());
// 	}
// 	int col = 0;
// 	for (auto iteBit : involvBits) {
// 		bit2Col[iteBit] = col;
// 		col++;
// 	}
// 	mat_GF2 matA, matAb;
// 	matA.SetDims(allEqns.size(), involvBits.size());
// 	matAb.SetDims(allEqns.size(), involvBits.size() + 1);
// 	for (auto row = 0; row < allEqns.size(); ++row) {
// 		for (auto btIte : allEqns[row]->lhs) {
// 			matA[row][bit2Col[btIte]] = 1;
// 			matAb[row][bit2Col[btIte]] = 1;
// 		}
// 		if (allEqns[row]->rhs != 0)matAb[row][involvBits.size()] = 1;
// 	}
// 	auto degA = gauss(matA);
// 	auto degAb = gauss(matAb);
// 	if (degA == degAb) {
// 		return true;
// 	}
// 	else {
// 		return false;
// 	}
// }
