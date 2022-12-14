#include "SimpleMinimization.h"

#include<sys/time.h>
#include<iostream>
#include<stdlib.h>
#include<vector>
#include<math.h>
#include<omp.h>

const double DR_SHRINK = 0.8;

using namespace std;

struct drand48_data randBuffer;

SimpleMinimization::SimpleMinimization(Function *f, double timeLimit) : Minimization(f, timeLimit) {

	generateRandomPosition();
	bestX = x;
	bestY = y;
	bestZ = z;

	bestV = function->value(bestX, bestY, bestZ);

	unsigned long seed = (unsigned long) time(NULL);

	srand48_r(seed, &randBuffer);
}


SimpleMinimization::~SimpleMinimization() {}


void SimpleMinimization::find(double dr_ini, double dr_fin, int idleStepsLimit ) {
	double v, xnew, ynew, znew, vnew, dr;
	int idleSteps = 0;

	std::cout << "Start " << std::endl;

	#pragma omp parallel firstprivate(idleSteps, dr, v)
	while (hasTimeToContinue()) {
		// inicjujemy losowo polozenie startowe w obrebie kwadratu o bokach od min do max

		#pragma omp critical
		{
			generateRandomPosition();
		}

		v = function->value(x, y, z); // wartosc funkcji w punkcie startowym

		idleSteps = 0;
		dr = dr_ini;

		while ( ( dr > dr_fin ) && ( idleSteps < idleStepsLimit ) ) {

			double randomx;
			double randomy;
			double randomz;
			randomx = drand48_r(&randBuffer, &randomx);
			randomy = drand48_r(&randBuffer, &randomy);
			randomz = drand48_r(&randBuffer, &randomz);
			xnew = x + (randomx - 0.5) * dr;
			ynew = y + (randomy - 0.5) * dr;
			znew = z + (randomz - 0.5) * dr;

			// upewniamy sie, ze nie opuscilismy przestrzeni poszukiwania rozwiazania
			xnew = limitX(xnew);
			ynew = limitY(ynew);
			znew = limitZ(znew);

			// wartosc funkcji w nowym polozeniu
			vnew = function->value(xnew, ynew, znew);

			#pragma omp critical
			{
				if (vnew < v) {
					x = xnew;  // przenosimy sie do nowej, lepszej lokalizacji
					y = ynew;
					z = znew;
					v = vnew;
					idleSteps = 0; // resetujemy licznik krokow, bez poprawy polozenia
				} else {
					idleSteps++; // nic sie nie stalo

					if ( idleSteps > idleStepsLimit ) {
						dr *= DR_SHRINK; // zmniejszamy dr
						idleSteps = 0;
					}
				}
			}
		} // dr wciaz za duze

		#pragma omp critical
		{
			addToHistory();
		}

		#pragma omp critical
		if (v < bestV) {  // znalezlismy najlepsze polozenie globalnie
			bestV = v;
			bestX = x;
			bestY = y;
			bestZ = z;

			std::cout << "New better position: " << x << ", " << y << ", " << z
					<< " value = " << v << std::endl;
		}
	} // mamy czas na obliczenia
	cout << "\t---\tdone!!!\t---\t"<< endl;
}

void SimpleMinimization::generateRandomPosition() {
	x = drand48() * (maxX - minX) + minX;
	y = drand48() * (maxY - minY) + minY;
	z = drand48() * (maxZ - minZ) + minZ;
}
