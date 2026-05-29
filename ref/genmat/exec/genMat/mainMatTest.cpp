#include <time.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <omp.h>
#include <boost/program_options.hpp>
// #include<NTL/ZZ_p.h>
// #include<NTL/mat_ZZ_p.h>
#include <NTL/RR.h>
#include <NTL/mat_GF2.h>
#include <boost/program_options.hpp>
#include <sstream>
#include <NtlCommon.h>
#include "LinPolyGF2E.h"
#include <set>
#include <boost/program_options.hpp>
// #include "HashImpl.h"
#include <boost/regex.hpp>

NTL_CLIENT

namespace po = boost::program_options;

using namespace std;

int main(int argc, char **argv)
{
	// srand_64(time(NULL));
	srand(time(NULL));

	int dim = 64;
	double hwRate = 0.26;

	po::options_description desc("Newly Designed VOLEitH Hash");
	desc.add_options()("help,h", "produce help message.")("dim,d", po::value<int>(&dim)->default_value(64), "The dimension (degree) of primitive polynomial n of field GF(2^n), default n=64")("rate,r", po::value<double>(&hwRate)->default_value(0.35), "The maximum hamming weight of input words. Default 3");
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 0;
	}

	GF2X basePoly; //= genRandPrimitivePolyOfDeg(dim);
	switch (dim)
	{
	case 64:
		SetCoeff(basePoly, 64);
		SetCoeff(basePoly, 4);
		SetCoeff(basePoly, 3);
		SetCoeff(basePoly, 1);
		SetCoeff(basePoly, 0);
		break;
	case 96:
		SetCoeff(basePoly, 96);
		SetCoeff(basePoly, 10);
		SetCoeff(basePoly, 9);
		SetCoeff(basePoly, 6);
		SetCoeff(basePoly, 0);
		break;
	case 128:
		SetCoeff(basePoly, 128);
		SetCoeff(basePoly, 7);
		SetCoeff(basePoly, 2);
		SetCoeff(basePoly, 1);
		SetCoeff(basePoly, 0);
		break;
	case 192:
		SetCoeff(basePoly, 192);
		SetCoeff(basePoly, 7);
		SetCoeff(basePoly, 2);
		SetCoeff(basePoly, 1);
		SetCoeff(basePoly, 0);
		break;
	case 256:
		SetCoeff(basePoly, 256);
		SetCoeff(basePoly, 10);
		SetCoeff(basePoly, 5);
		SetCoeff(basePoly, 2);
		SetCoeff(basePoly, 0);
		break;
	case 384:
		SetCoeff(basePoly, 384);
		SetCoeff(basePoly, 16);
		SetCoeff(basePoly, 15);
		SetCoeff(basePoly, 6);
		SetCoeff(basePoly, 0);
		break;
	case 512:
		SetCoeff(basePoly, 512);
		SetCoeff(basePoly, 8);
		SetCoeff(basePoly, 5);
		SetCoeff(basePoly, 2);
		SetCoeff(basePoly, 0);
		break;
	default:
		basePoly = genRandPrimitivePolyOfDeg(dim);
		break;
	}

	cout << "Base Poly: \n"
		 << basePoly << endl;
	cout << testPrimitiveGF2X(basePoly) << endl;

	GF2E::init(basePoly);

	int hwUBound = dim * hwRate;
	vector<LinPolyGF2E> targMats;
	do{
		mat_GF2 rand;
		GF2 det=to_GF2(0);
		rand=random_mat_GF2(dim,dim);
		det=determinant(rand);
		if(det==to_GF2(0)){
			continue;
		}
		targMats.push_back(LinPolyGF2E(rand));
	}while(targMats.size()<4);






	// mat_GF2 mm = getInvertableRandomWithHwMatGF2(dim, hwUBound);
	// cout << determinant(mm) << endl;

	// GF2E randGF2E = getRandomWithHwGF2E(dim, hwUBound);
	// cout <<randGF2E<<endl<< randGF2E.degree() << endl;

	ofstream oFile("OurNewVOLEitH" + to_string(dim)  + ".txt");
	for (auto i = 0; i < 4; ++i)
	{
		LinPolyGF2E linRes = targMats[i];
		//generateLinPolyGF2E(hwRate, dim);
		// cout << linRes.getMatGF2ML() << endl;
		targMats.push_back(linRes);
		oFile << "Mat " << dec << i << endl
			  << linRes.getMatGF2ML() << endl;
	}
	oFile.close();

	return 0;
}

#if RAINIER_EQNS

class Rainier
{
public:
	int round, degree;
	vector<LinPolyGF2E> mats;
	vector<GF2E> consts, states;
	GF2E input, output, key;
	GF2X basePoly;
	Rainier(int rd = 3, int deg = 8)
	{
		round = rd;
		degree = deg;
		basePoly = genRandPrimitivePolyOfDeg(degree);
		GF2E::init(basePoly);
		for (int r = 0; r < round; ++r)
		{
			mats.push_back(generateLinPolyGF2E(degree));
		}
		for (int r = 0; r <= round; ++r)
		{
			consts.push_back(random_GF2E());
		}
		input = 0; // random_GF2E();
		key = 0;   // random_GF2E();
		states = {};
	}

	void impl()
	{
		input = random_GF2E();
		key = random_GF2E();
		states = {input};
		for (int r = 0; r < round; ++r)
		{
			states.push_back(doRound(r));
		}
		output = states[round] + key + consts[round];
		states.push_back(output);
	}

	GF2E doRound(int r)
	{
		GF2E beforeInv = states[r] + key + consts[r];
		assert(IsZero(beforeInv) == 0);
		GF2E afterInv = inv(beforeInv);
		GF2E out = mats[r].eval(afterInv);
		return out;
	}

	void showAll(ostream &o)
	{
		o << "Base Poly: \n"
		  << basePoly << endl;
		o << "Consts:\n";
		for (auto id = 0; id < consts.size(); ++id)
		{
			o << consts[id] << endl;
		}
		o << "Matrices:\n";
		for (auto i = 0; i < mats.size(); ++i)
		{
			o << "Mat No." << dec << i << endl;
			o << mats[i].getMatGF2ML() << endl;
			o << "Poly No." << dec << i << endl;
			vector<GF2E> coff = mats[i].getCoeff();
			for (auto d = 0; d < coff.size(); ++d)
			{
				o << coff[d] << endl;
			}
		}
		o << "Key:\n";
		o << key << endl;
		o << "States:\n";
		for (auto r = 0; r < states.size(); ++r)
		{
			o << states[r] << endl;
		}
	}
};

vector<int> extractVecIntFromString(string monoStr)
{
	vector<int> res;

	boost::regex expr("\\d+");

	boost::smatch what;
	// boost::match_results<string::const_iterator> findIndex;
	std::string::const_iterator start = monoStr.begin();
	std::string::const_iterator end = monoStr.end();
	while (boost::regex_search(start, end, what, expr))
	{
		// cout << what[0] << endl;
		res.push_back(atoi(what[0].str().c_str()));
		// res[atoi(what[0].str().c_str())] = 1;
		start = what[0].second;
	}
	return res;
}

string vec2string(vector<int> vec)
{
	string res = "";
	std::set<int> vset;
	vset.insert(vec.begin(), vec.end());
	for (auto ite = vset.begin(); ite != vset.end(); ++ite)
	{
		res += "+" + to_string(*ite);
	}
	return res;
}

int main()
{
	// simplify mono:
	vector<vector<int>> allMonoRaw;
	std::ifstream monofile("monos.txt"); // �滻Ϊ����ļ���

	std::set<string> monoStrSet;

	std::map<string, int> uniqStr2UniqIndx;
	std::map<int, string> rawIndx2UniqStr;
	if (monofile.is_open())
	{
		string line;
		int linNo = 0;
		while (std::getline(monofile, line))
		{
			// std::cout << line << std::endl;
			if (line == "1")
			{
				cout << linNo << endl;
				allMonoRaw.push_back({});
				string oneStr = "+";
				monoStrSet.insert(oneStr);
				rawIndx2UniqStr[linNo] = oneStr;
				++linNo;
				continue;
			}
			vector<int> lineIndx = extractVecIntFromString(line);
			if (!lineIndx.empty())
			{
				allMonoRaw.push_back(lineIndx);
				string lcStr = vec2string(lineIndx);
				monoStrSet.insert(lcStr);
				rawIndx2UniqStr[linNo] = lcStr;
				++linNo;
			}
		}
		monofile.close();
	}
	int unqIndx = 0;
	for (auto itStr : monoStrSet)
	{
		uniqStr2UniqIndx[itStr] = unqIndx;
		++unqIndx;
	}

	mat_GF2 testMat;
	int totrow = 4000, totcol = uniqStr2UniqIndx.size();
	testMat.SetDims(totrow, totcol);
	std::set<int> allIndx;
	std::ifstream file("output.txt"); // �滻Ϊ����ļ���
	vector<vector<int>> mtIndx;
	if (file.is_open())
	{
		string line;
		while (std::getline(file, line))
		{
			// std::cout << line << std::endl;
			vector<int> lineIndx = extractVecIntFromString(line);
			vector<int> unqIndxSet;
			for (auto num : lineIndx)
			{
				unqIndxSet.push_back(uniqStr2UniqIndx[rawIndx2UniqStr[num]]);
			}
			if (!lineIndx.empty())
			{
				mtIndx.push_back(unqIndxSet);
				allIndx.insert(unqIndxSet.begin(), unqIndxSet.end());
			}
		}
		file.close();
	}

	for (auto row = 0; row < mtIndx.size(); ++row)
	{
		for (auto col = 0; col < mtIndx[row].size(); ++col)
		{
			testMat[row][mtIndx[row][col]] = 1;
			allIndx.insert(mtIndx[row][col]);
		}
	}
	long rank = gauss(testMat);
	cout << rank << endl;

	GF2X bpoly;
	SetCoeff(bpoly, 0);
	SetCoeff(bpoly, 1);
	SetCoeff(bpoly, 2);
	SetCoeff(bpoly, 7);
	SetCoeff(bpoly, 8);
	cout << bpoly << endl;
	GF2E::init(bpoly);
	vector<long> poly = {0, 1, 3, 4, 5, 6, 7};
	GF2E beforeInv = getExpDefGF2E(poly);
	cout << beforeInv << endl;
	cout << inv(beforeInv) << endl;
	mat_GF2 myMat;
	cin >> myMat;
	LinPolyGF2E linPly = LinPolyGF2E(myMat);
	cout << linPly.getMatGF2ML();

	vector<GF2E> mm = linPly.getCoeff();

	int targDeg = 8;
	int rd = 3;
	Rainier ran(rd, targDeg);
	ofstream paramFile("ToyVersion.txt");
	ran.impl();
	ran.showAll(std::cout);
	ran.showAll(paramFile);
	paramFile.close();

	return 0;
}

#endif

#if IMPLEMENT_INSTANCE

int main(int argc, char **argv)
{
	srand_64(time(NULL));

	int targDeg = 64;
	int testTime = 100;
	int maxInHw = 3;

	po::options_description desc("Newly Designed VOLEitH Hash");
	desc.add_options()("help,h", "produce help message.")("degree,d", po::value<int>(&targDeg)->default_value(64), "The degree of primitive polynomial n of field GF(2^n), default n=64")("weightMaximum,w", po::value<int>(&maxInHw)->default_value(3), "The maximum hamming weight of input words. Default 3")("testtime,t", po::value<int>(&testTime)->default_value(100), "Repeat time, default 100");
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 0;
	}

	long inHwSum = 0, outHwSum = 0;
	for (int tt = 0; tt < testTime; ++tt)
	{
		GF2X basePoly = genRandPrimitivePolyOfDeg(targDeg);
		GF2E::init(basePoly);

		// cout << "Base Poly: \n" << basePoly << endl;
		// cout << GF2E::modulus() << endl;
		int hw0 = rand_64() % maxInHw;
		int hw1 = rand_64() % maxInHw;
		long inHw = hw0 + hw1;
		HashImpl hashM = HashImpl(targDeg);
		string filename = "Sandwich" + to_string(targDeg) + "Test" + to_string(tt) + "Param.txt";
		ofstream paramFile(filename);
		// paramFile << "Base:" << basePoly << endl;
		hashM.showParams(paramFile);
		paramFile.close();
		GF2E inX0 = getRandomWithHwGF2E(targDeg, hw0);
		GF2E inX1 = getRandomWithHwGF2E(targDeg, hw1);

		// cout << inX0 << "," << inX1 << endl;
		inHwSum += inHw;
		cout << dec << "No." << tt << ":" << (hw0 + hw1) << "(" << hw0 << "," << hw1 << ")->";
		vector<GF2E> out2 = hashM.run(inX0, inX1);
		long outHw = 0;
		for (auto idx = 0; idx < out2.size(); ++idx)
		{
			outHw += weight(rep(out2[idx]));
		}
		cout << outHw << endl;
		outHwSum += outHw;
	}
	double inHwAvrg = ((double)inHwSum) / testTime;
	double outHwAvrg = ((double)outHwSum) / testTime;
	cout << "Average:" << inHwAvrg << "->" << outHwAvrg << endl;

	// cout << basePoly[0] << " " << basePoly[1] << " " << basePoly[2] << endl;

	// std::set<int> mm = randomPick(targDeg, 2);
	// for (auto idx : mm) {
	//	cout << dec << idx << " ";
	// }
	// cout << endl;

	// LinPolyGF2E lin0 = generateLinPolyGF2E(targDeg);
	// mat_GF2 mat0 = lin0.getMatGF2ML();
	////cout << mat0 << endl;
	// GF2E inX = getRandomWithHwGF2E(targDeg, 2);
	// cout << inX << endl;
	// GF2E outY = lin0.eval(inX);
	// cout << outY << endl;

	return 0;
}

#endif

#if 0
int main() {
	srand(time(NULL));
	long targDeg = 128;

	cout << "Generate linear polynomials of degree " << dec << targDeg << endl;
	LinPolyGF2E candPoly = generateLinPolyGF2E(targDeg);

	GF2X basePoly = genRandPrimitivePolyOfDeg(targDeg);
	GF2E::init(basePoly);
	GF2E randTest;// = random_GF2E();
	vector<long> myExps = { 1,5 };

	randTest = getExpDefGF2E(myExps);
	//LinPolyGF2E poly15 = LinPolyGF2E(randTest);


	cout << basePoly << endl;
	cout << randTest << endl;
	LinPolyGF2E linPoly = LinPolyGF2E(targDeg);
	linPoly.genRandomCoeff();

	LinPolyGF2E invPoly = linPoly.getInvPoly();
	cout << "Log Deg:" << invPoly.getLogDegree() << endl;

	//cout <<"GF2 Mat:\n" << linPoly.getMatGF2ML() << endl;
	GF2E inX = random_GF2E();
	GF2E oY = linPoly.eval(inX);
	GF2E oinvY = invPoly.eval(oY);
	cout << "x:" << (inX) << endl;
	cout << "L(x):" << (oY) << endl;
	cout << "L^-1*L(x)+x:" << (oinvY + inX) << endl;


	//for (auto idx = 0; idx < linPoly.coeff.size(); ++idx) {
	//	cout << "No." << dec << idx << ":" << linPoly.coeff[idx] << endl;
	//	for (auto de = 0; de < targDeg; ++de) {
	//		cout << rep(linPoly.coeff[idx])[de] << " ";
	//	}
	//	cout << endl;
	//}
	return 0;
}

#endif

// Compute the probabilities for invertable matrix and complement-invertable matrix
#if 0
int main(int argc, char** argv) {

	int dim = 64;
	long repeatTarget = 10000;

	po::options_description desc("Compute the probabilities for invertable matrix and complement-invertable matrix");
	desc.add_options()("help,h", "produce help message.")
		("repeat,r", po::value<long>(&repeatTarget)->default_value(10000), "The number invertable matrix (=3)")
		("dimension,d", po::value<int>(&dim)->default_value(64), "The dimention of matrix (=64)");
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	if (vm.count("help"))
	{
		cout << desc << endl;
		return 1;
	}
	mat_GF2 randMat, compMat, oneMat;
	oneMat = diag(dim, to_GF2(0));
	long testTime = 0, succTime = 0, invCount = 0;
	for (auto row = 0; row < dim; ++row) {
		for (auto col = 0; col < dim; ++col) {
			oneMat[row][col] = 1;
		}
	}
	do {
		bool isFeasible = false;
		++testTime;
		randMat = random_mat_GF2(dim, dim);

		if (determinant(randMat) != to_GF2(0)) {
			//cout << determinant(randMat) << endl;
			invCount++;
			compMat = oneMat + randMat;
			//cout << randMat<< "\n"<<compMat << endl;
			if (determinant(compMat) != to_GF2(0)) {
				isFeasible = true;
				++succTime;
			}
		}
		else {
			continue;
		}
	} while (invCount < repeatTarget);
	//while (testTime< repeatTarget);
	ofstream matFile;
	matFile.open("MatFile" + to_string(dim) + ".txt");
	matFile << randMat << endl;
	matFile << "Success " << dec << succTime << " out of " << repeatTarget << "/" << testTime << " trials!(Rate: " << (to_RR(succTime) / to_RR(repeatTarget)) << ")\n";
	matFile.close();
	cout << "Success " << dec << succTime << " out of " << repeatTarget << "/" << testTime << " trials!(Rate: " << (to_RR(succTime) / to_RR(repeatTarget)) << ")\n";

	return 0;
}
#endif
