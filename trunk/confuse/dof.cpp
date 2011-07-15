#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#include "dof.h"

DofFilter::DofFilter(float _near, float _far)
{
	kernel = NULL;
	useKernel = false;
	boostBokeh = false;

	focus = 4;
	focalLen = 0.7;
	aperture = 30;
	
	near = _near;
	far = _far;
}

DofFilter::~DofFilter() {};


void DofFilter::allocMatrixes(int w, int h)
{
//	db = (float*) realloc(db, h*w*sizeof(float));
	//fb1 = (rgb_t*) realloc(fb1, h*w*sizeof(rgb_t));
	//fb2 = (rgb_t*) realloc(fb2, h*w*sizeof(rgb_t));
}

void DofFilter::loadKernelBMP(const char* filename)
{
	if(kernel) free(kernel);
	
	kernel = loadBMP(filename);
	//printKernel();		
}

void DofFilter::printKernel()
{
	for(int y=kernel->height; y>0; y--) {
		for(int x=0; x<kernel->width; x++)
			//printf("%.1f  ",kernelVal(x,y));
			printf("%i  ",kernel->pixels[3*x + 3*y*kernel->width]);
		printf("\n");
	}
}

inline double DofFilter::kernelVal(int x, int y)
{
	static int val;
	y = kernel->height - y;
	val = kernel->pixels[3*y + 3*x*kernel->width];
	return val<0 ? (val+256)/256. : val/256.;
}

double DofFilter::biInterpolKernel( int s, int t, double kratio )
{
	return  0.25*kernelVal(floor(s*kratio),floor(t*kratio)) +
			0.25*kernelVal(ceil(s*kratio),ceil(t*kratio)) +
			0.25*kernelVal(ceil(s*kratio),floor(t*kratio)) +
			0.25*kernelVal(floor(s*kratio),ceil(t*kratio));
}

inline void DofFilter::filterPixelOnBuffer( float r, int x, int y )
{
	static int nx, ny, s, t;
	static double kratio, nr, ng, nb, interpol, area;
	static int i, j;
	static int maxPts;
	
	if(floor(r)==0) {
		//trivial case: no spread
		MPOS(fb2,x,y).r += MPOS(fb1,x,y).r;
		MPOS(fb2,x,y).g += MPOS(fb1,x,y).g;
		MPOS(fb2,x,y).b += MPOS(fb1,x,y).b;
		
		MPOS(sb,x,y) = 0;
	}
	else {
		// set the maximum array sizes with the bounding box size
		maxPts = (int)(4*r*r);
		int nxv[maxPts], nyv[maxPts], pts=0;
		float kval[maxPts];
	
		area = 0;
	
		// Convolution by Image Kernel
		if(useKernel && kernel!=NULL) {
			
			//ratio for linear super/subsampling from kernel image
			kratio = kernel->width/(2*r);
			
			for(i=-ceil(r), s=0; i<floor(r); i++, s++) {
				for(j=-floor(r), t=0; j<ceil(r); j++, t++) {
					nx = x+i;
					ny = y+j;
					if( nx >= 0 && nx < WIDTH && 
						ny >= 0 && ny < HEIGHT)
						//interpol = biInterpolKernel(s,t,kratio);
						interpol = kernelVal((int)s*kratio,(int)t*kratio);
						//printf("%i ",(int)round(interpol));
						if(interpol) {
							nxv[pts] = nx;
							nyv[pts] = ny;
							kval[pts] = interpol;
							pts++;
							area += interpol;
						}
				}
				//printf("\n");
			}
			//printf("\n");
			
		}
		else
		// Convolution by Circle Equation
		
			for(i=-ceil(r); i<=floor(r); i++)
				for(j=-floor(r); j<=ceil(r); j++) {
					nx = x+i;
					ny = y+j;
					if( nx > 0 && nx < WIDTH && 
						ny > 0 && ny < HEIGHT) {
						if( sqrt(pow((nx-x),2)+pow((ny-y),2)) < r ) { //circular spread
							nxv[pts] = nx;
							nyv[pts] = ny;
							/*if(MPOS(sb,nx,ny)==-1 || MPOS(sb,nx,ny) <= r)
								contrib[pts] = 1;
							else
								contrib[pts] = 1./MPOS(sb,nx,ny);*/
							pts++;
							area++;
						}
					}
				}
				
		
		nr = MPOS(fb1,x,y).r/area;
		ng = MPOS(fb1,x,y).g/area;
		nb = MPOS(fb1,x,y).b/area;
			
		if(boostBokeh && r>6) {
			static double mono, boost, variance;
			mono = 0.3*MPOS(fb1,x,y).r + 0.59*MPOS(fb1,x,y).g + 0.11*MPOS(fb1,x,y).b;
			boost = 1./(95.-mono*88) + (1. - 1./90);
			//DEBUG_VAR(boost);
			variance = r/(270.-r*3.) + 1;
			//variance = 1./(210.-r*4.) + (1. - 1./210);
			//variance = r*r/500 + 1;
			
			nr *= boost*variance;
			ng *= boost*variance;
			nb *= boost*variance;
		}
		
		// fill new Frame Buffer with the new color
		if(useKernel && kernel!=NULL)
			for(i=0; i<pts; i++) {
				MPOS(fb2,nxv[i],nyv[i]).r += kval[i]*nr;
				MPOS(fb2,nxv[i],nyv[i]).g += kval[i]*ng;
				MPOS(fb2,nxv[i],nyv[i]).b += kval[i]*nb;
			}
		else
			for(i=0; i<pts; i++) {
				MPOS(fb2,nxv[i],nyv[i]).r += nr;//*contrib[i];
				MPOS(fb2,nxv[i],nyv[i]).g += ng;//*contrib[i];
				MPOS(fb2,nxv[i],nyv[i]).b += nb;//*contrib[i];
				//MPOS(sb,nxv[i],nyv[i]) = r;
			}
	}
}

void DofFilter::setParams(float _focus, float _focalLen, float _aperture)
{
	focus = _focus;
	focalLen = _focalLen;
	aperture = _aperture;
	
	updateDistance();
}

void DofFilter::apply()
{
	cout << "Applying DOF Filter...\n";

	for(int x=0; x<WIDTH; x++)
		for(int y=0; y<HEIGHT; y++) {
			MPOS(fb2,x,y) = {0,0,0};
			MPOS(sb,x,y) = -1;
		}

	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, fb1);
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT, db);

	double z;
	float r;
	for(int x=0; x<WIDTH; x++)
		for(int y=0; y<HEIGHT; y++) {
			z = MPOS(db,x,y);
			
			if( z<1 ) { //only process objects, not the background

				//retrieve pixel depth from Camera Coord. Space
				z = near*far / (far - z*(far-near));
				
				//calculate CoC based on Single Lens Camera Model
				r = absf( aperture * ( sensorDist*(1./focalLen - 1./z) - 1 ) );
				
				filterPixelOnBuffer(r, x, y);
			}
		}

	glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, fb2);
	
	cout << "Done.\n";
}

void DofFilter::updateDistance()
{
	//calculate sensor distance according to the desired focus plane
	sensorDist = ((1./aperture)+1)/(1./focalLen - 1./focus);
}
