#ifndef FLUIDSOLVER_H
#define FLUIDSOLVER_H

#define IX(i,j,k) ((i)+(N+2)*(j)+(N+2)*(N+2)*(k))
#define SWAP(x0, x) {float *tmp=x0;x0=x;x=tmp;}

#include "../../TSBB11/src/common/glm/glm.hpp"
#include <vector>

class fluidsolver
{
public:
	fluidsolver(int N);
	fluidsolver(int height, int width, int depth);
	~fluidsolver() {};

	void set_bnd(int b, float* x);
	void add_source(float *x, float *s);
	void diffuse(int b, float *x, float *x0, float diff);
	void advect (int b, float *d, float *d0, float *u, float *v, float *w);

	void project (float *u, float *v, float *w, float *p, float *div);

	void dens_step(float *x, float *x0, float *u, float *v, float *w, float diff);
	void vel_step(float *u, float *v, float *w, float *u0, float *v0, float *w0, float visc);

	void print(float* v);
private:
	
	std::vector<glm::vec3> _velocity;
	int _height;
	int _width;
	int _depth;
	int _size;

	float _dt;
};

#endif //fluidsolver
