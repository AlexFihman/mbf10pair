#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdlib.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "time_ms.h"

using namespace std;
using namespace boost::multiprecision;

typedef uint64_t mbf6;
typedef uint128_t mbf7;
typedef uint256_t mbf8;
typedef uint512_t mbf9;

mbf6** funclists;
int* funcsize;

//const mbf6 zero6 = 0;
const mbf6 F6_MAX = (mbf6)(-1);
const mbf7 zero7 ("0");
const mbf7 F7_MAX = (zero7 -1);
const mbf8 zero8 ("0");
const mbf8 F8_MAX = (zero8 -1);

std::mutex coutMutex; // Declare a mutex to protect std::cout

bool cmp6(mbf6* fl, int i, int j) {
	return ((~fl[i]) | fl[j]) ==  F6_MAX;
}

bool cmp6f(mbf6 f1, mbf6 f2) {
	return ((~f1) | f2) ==  F6_MAX;
}

bool cmp7(mbf7 f1, mbf7 f2) {
	return ((~f1) | f2) ==  F7_MAX;
}

bool cmp8(mbf8 f1, mbf8 f2) {
	return ((~f1) | f2) ==  F8_MAX;
}

void uplevel(int init_level){
	int i,j;
	int lsize = funcsize[init_level];
	int newsize = 0;
	for (i=0;i<lsize;i++){
		for (j=0;j<lsize;j++) {
			if (cmp6(funclists[init_level], i, j))
				newsize++;
		}
	}
	funcsize[init_level+1] = newsize;
	funclists[init_level+1] = new mbf6[newsize];

	newsize = 0;
	int shift = 1 << init_level;
	for (i=0;i<lsize;i++){
			for (j=0;j<lsize;j++) {
				if (cmp6(funclists[init_level], i, j)) {
					funclists[init_level+1][newsize] = ((funclists[init_level][i]) << shift) | funclists[init_level][j];
					//cout << funclists[init_level+1][newsize] << endl;
					newsize++;
				}
			}
		}
}

int N6 = 0;

mbf7 make7(boost::random::mt19937& rng) {
	mbf6 f1, f2;
	do {
		int i1 = rng() % N6;
		int i2 = rng() % N6;
		f1 = funclists[6][i1];
		f2 = funclists[6][i2];
	} while (!cmp6f(f1,f2));
	mbf7 result = f1;
	result = (result << 64) | f2;
	return result;
}

mbf8 make8(boost::random::mt19937& rng) {
	mbf7 f1, f2;
	do {
		f1 = make7(rng);
		f2 = make7(rng);
	} while (!cmp7(f1,f2));
	mbf8 result = f1;
	result = (result << 128) | f2;
	return result;
}

mbf9 make9(boost::random::mt19937& rng) {
	mbf8 f1, f2;
	do {
		f1 = make8(rng);
		f2 = make8(rng);
	} while (!cmp8(f1,f2));
	mbf9 result = f1;
	result = (result << 256) | f2;
	return result;
}

// A function that will be executed in a separate thread

double startTime;

int seed = 1;
int t_limit;
int n_thread;

void threadFunction()
{
    // Lock the mutex before accessing std::cout
    boost::random::mt19937 rng;
    double t1;
    while (true) {
      coutMutex.lock();
      int arg = seed++;
      coutMutex.unlock();

      rng.seed(arg);
      mbf9 m = make9(rng);
      coutMutex.lock();
      t1 = TimeMillis();
      std::cout << arg << "\t" << (t1-startTime) << "\t" << m << std::endl;
      coutMutex.unlock();
      if (t1 > (startTime + t_limit)) 
        return;
    };
}

int main(int argc, char* argv[])
{

	funclists = new mbf6*[7];
	funclists[0] = new mbf6[2];
	funclists[0][0] = 0;
	funclists[0][1] = 1;
	funcsize = new int[7];
	funcsize[0] = 2;
	uplevel(0);
	uplevel(1);
	uplevel(2);
	uplevel(3);
	uplevel(4);
	uplevel(5);
	N6 = funcsize[6];

     seed = atoi(argv[1]);
     t_limit = atoi(argv[2]);
     n_thread = atoi(argv[3]);
    //ofstream wf("mbf9rnd.dat", ios::out | ios::binary);
    //if(!wf) {
    //  cout << "Cannot open file!" << endl;
    //  return 1;
    //}

    startTime =TimeMillis();
    //for (int i=0;i<100;i++) {
    //	mbf9 m = make9(rng);
    //    //wf.write((char*)&m, sizeof(mbf9));
    //	cout << m << endl;
    //}
    //cout << m7 << endl;

    std::thread myThreads[n_thread];
    for (int i = 0; i < n_thread; i++)
    {
        myThreads[i] = std::thread(threadFunction);
    }

    for (int i = 0; i < n_thread; i++)
    {
        myThreads[i].join();
    }

    double t1 =TimeMillis();
    //cout << "run time: " << (t1-startTime) << endl;
    //wf.close();

    return 0;
}
