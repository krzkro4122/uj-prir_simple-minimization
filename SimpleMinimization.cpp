#include "SimpleMinimization.h"

#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "omp.h"

const double DR_SHRINK = 0.8;

using namespace std;

SimpleMinimization::SimpleMinimization(Function *f, double timeLimit) : Minimization(f, timeLimit) {

	generateRandomPosition();
	bestX = x;
	bestY = y;
	bestZ = z;

	bestV = function->value(bestX, bestY, bestZ);

	cout << bestX << " " << bestY << " " << bestZ << " " << bestV << endl;

	// unsigned long seed = (unsigned long) time(NULL);
	// srand48_r(seed, &randBuffer);
}


SimpleMinimization::~SimpleMinimization() {}


void SimpleMinimization::find(double dr_ini, double dr_fin, int idleStepsLimit ) {

	double v;

	std::cout << "Start " << std::endl;

	#pragma omp parallel
	{
		double x_temp, y_temp, z_temp, v_temp, xnew, ynew, znew, vnew, dr;
		int idleSteps = 0;
		struct drand48_data randBuffer;

		while (hasTimeToContinue()) {
			// inicjujemy losowo polozenie startowe w obrebie kwadratu o bokach od min do max

			#pragma omp critical
			{
				double randomx;
				double randomy;
				double randomz;
				drand48_r(&randBuffer, &randomx);
				drand48_r(&randBuffer, &randomy);
				drand48_r(&randBuffer, &randomz);
				x = randomx * (maxX - minX) + minX;
				y = randomy * (maxY - minY) + minY;
				z = randomz * (maxZ - minZ) + minZ;
				v = function->value(x, y, z); // wartosc funkcji w punkcie startowym
				v_temp = v;
				x_temp = x;
				y_temp = y;
				z_temp = z;
			}

			idleSteps = 0;
			dr = dr_ini;

			while ( ( dr > dr_fin ) && ( idleSteps < idleStepsLimit ) ) {

				double randomx;
				double randomy;
				double randomz;
				drand48_r(&randBuffer, &randomx);
				drand48_r(&randBuffer, &randomy);
				drand48_r(&randBuffer, &randomz);
				xnew = x_temp + (randomx - 0.5) * dr;
				ynew = y_temp + (randomy - 0.5) * dr;
				znew = z_temp + (randomz - 0.5) * dr;

				// upewniamy sie, ze nie opuscilismy przestrzeni poszukiwania rozwiazania
				xnew = limitX(xnew);
				ynew = limitY(ynew);
				znew = limitZ(znew);

				// wartosc funkcji w nowym polozeniu
				vnew = function->value(xnew, ynew, znew);

				#pragma omp critical
				{
					if (vnew < v_temp) {
						x_temp = xnew;  // przenosimy sie do nowej, lepszej lokalizacji
						y_temp = ynew;
						z_temp = znew;
						v_temp = vnew;
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
				x = x_temp;
				y = y_temp;
				z = z_temp;

				addToHistory();

				if (v_temp < bestV) {  // znalezlismy najlepsze polozenie globalnie

					bestV = v_temp;
					bestX = x_temp;
					bestY = y_temp;
					bestZ = z_temp;

					// std::cout << "New better position:\t" << x_temp << ", " << y_temp << ", " << z_temp
					// 		<< ", value = " << v_temp << std::endl;
				}
			} // mamy czas na obliczenia
		}
	}
}

void SimpleMinimization::generateRandomPosition() {
	x = drand48() * (maxX - minX) + minX;
	y = drand48() * (maxY - minY) + minY;
	z = drand48() * (maxZ - minZ) + minZ;
}
