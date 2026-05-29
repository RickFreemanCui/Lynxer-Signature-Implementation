#pragma once
#include <time.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <NTL/ZZ_pX.h>
#include <NTL/mat_ZZ_p.h>
#include <NTL/mat_poly_ZZ_p.h>
#include <NTL/ZZ_pXFactoring.h>
#include <NTL/GF2XFactoring.h>
#include <NTL/GF2EXFactoring.h>
#include <NTL/mat_GF2.h>
#include <vector>
// #include "../Common/ArxCrypt/AStruct.h"

bool testPrimitiveGF2X(NTL::GF2X poly);

NTL::GF2X genRandPrimitivePolyOfDeg(long degree);

// get the GF2E value of the GF2X form x^e
NTL::GF2E getPowerGF2E(long e);

NTL::GF2E getRandomWithHwGF2E(int degr, const int hw);

NTL::mat_GF2 getRandomWithHwMatGF2(const long dim, const int hw);
NTL::mat_GF2 getInvertableRandomWithHwMatGF2(const long dim, const int hw);

// get the GF2E value of the GF2X form x^e
NTL::GF2E getExpDefGF2E(std::vector<long> exps);

// Test the feasibility of bit conditions deduced from the signed differential paths of ARX primitives
// bool checkBitConditions(std::vector<BitEquation*> eqns);