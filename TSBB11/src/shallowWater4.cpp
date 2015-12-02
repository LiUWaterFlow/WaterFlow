#define MANNING 10
#define g 9.81f


float u_bar(float height_left, float height_right, float u_left, float u_right)
{
	float sqrHeightLeft = sqrt(height_left);
	float sqrtHeightRight = sqrt(height_right);
	return (sqrtHeightLeft*u_left + sqrtHeightRight*u_right)/(sqrtHeightLeft + sqrtHeightRight);
}

float v_bar(float height_left, float height_right, float v_left, float v_right)
{
	return u_bar(height_left, height_right, v_left,v_right);
}

float h_bar(float height_left, float height_right)
{
	return 0.5f*(height_left + height_right);
}

float c_bar(float height_left,float height_right)
{
	return sqrt(g*h_bar(height_left,height_right);
}

float alpha_one(float height_left, float height_right, float u_left, float u_right, float hu_left, float hu_right)
{
	float delta_h = height_left - height_right;
	float delta_hu = hu_left - hu_right;
	float nomi = (c_bar(height_left,height_right) - u_bar(height_left, height_right, u_left, u_right))*(delta_h) + delta_hu;
	return nomi/(2*c_bar(height_left,height_right));
}

float alpha_two(float height_left, float height_right, float u_left, float u_right, float hu_left, float hu_right)
{
	float delta_h = height_left - height_right;
	float delta_hu = hu_left - hu_right;
	float nomi = (c_bar(height_left,height_right) + u_bar(height_left, height_right, u_left, u_right))*(delta_h) + delta_hu;
	return nomi/(2*c_bar(height_left,height_right));
}

float alpha_three(float height_left, float height_right, float v_left, float v_right, float hv_left, float hv_right)
{
	float delta_hv = hv_left - hv_right;
	float delta_h = h_left - h_right;
	return delta_hv - h * v_bar(height_left, height_right, v_left, v_right);
}

float lambda_one(float height_left, float height_right, float u_left, float u_right)
{
	return u_bar(height_left, height_right, u_left, u_right) + c_bar(height_left,height_right);
}

float lambda_two(float height_left, float height_right, float u_left, float u_right)
{
	return u_bar(height_left, height_right, u_left, u_right) - c_bar(height_left,height_right);
}

float lambda_three(float height_left, float height_right, float u_left, float u_right)
{
	return u_bar(height_left, height_right, u_left, u_right);
}

glm::vec3 e_one(float height_left, float height_right, float u_left, float u_right, float v_left, float v_right)
{
	return glm::vec3(1,
			lambda_one(height_left, height_right, u_left, u_right),
			v_bar(height_left, height_right, v_left,v_right));
}

glm::vec3 e_two(float height_left, float height_right, float u_left, float u_right, float v_left, float v_right)
{
	return glm::vec3(1,
			u_bar(height_left,height_right,u_left,u_right) - c_bar(height_left,height_right),
			v_bar(height_left, height_right, v_left,v_right));
}

glm::vec3 e_three(float height_left, float height_right, float u_left, float u_right, float v_left, float v_right)
{
	return glm::vec3(0,0,1);
}

glm::vec3 sum_alpha_lambda_e(float height_left, float height_right, float u_left, float u_right, float v_left, float v_right, float hu_left, float hu_right, float hv_left, float hv_right)
{
	glm::vec3 first = alpha_one(height_left, height_right, u_left, u_right, hu_left, hu_right)
			  * glm::abs(lambda_one(height_left, height_right, u_left, u_right)
			  * e_one(height_left, height_right, u_left, u_right, v_left, v_right);
			  
	glm::vec3 second = alpha_two(height_left, height_right, u_left, u_right, hu_left, hu_right)
			  * glm::abs(lambda_two(height_left, height_right, u_left, u_right)
			  * e_two(height_left, height_right, u_left, u_right, v_left, v_right);
			  
	glm::vec3 third = alpha_three(height_left, height_right, v_left, v_right, hv_left, hv_right)
			  * glm::abs(lambda_three(height_left, height_right, u_left, u_right)
			  * e_three(height_left, height_right, u_left, u_right, v_left, v_right);
	
	return first + second + third;
}

glm::vec3 F_half(glm::vec3 U_plus, glm::vec3 U_org)
{
	float height_left = U_org.x;
	float height_right = U_plus.x;
	float uh_left = U_org.y;
	float uh_right = U_plus.y;
	float vh_left = U_org.z;
	float vh_right = U_plus.z;

	float u_left = uh_left/height_left;
	float u_right = uh_right/height_right;
	float v_left = vh_left/height_left;
	float v_right = vh_right/height_right;

	return 0.5f*(F(U_plus) + F(U_org) - sum_alpha_lambda_e(height_left, height_right, u_left, u_right, v_left, v_right, hu_left, hu_right, hv_left, hv_right));
}

glm::vec3 G_half(glm::vec3 U_plus, glm::vec3 U_org)
{
	float height_below = U_org.x;
	float height_above = U_plus.x;
	float uh_below = U_org.y;
	float uh_above = U_plus.y;
	float vh_below = U_org.z;
	float vh_above = U_plus.z;

	float u_below = uh_below/height_below;
	float u_above = uh_above/height_above;
	float v_below = vh_below/height_below;
	float v_above = vh_above/height_above;
	return 0.5f*(G(U_plus) + G(U) - sum_alpha_lambda_e();
}

glm::vec3 F(glm::vec3 U)
{
	float h = U.x;
	float hu = U.y;
	float hv = U.z;

	float u = hu/h; //haxxors
	float v = hv/h;
	
	glm::vec3 F;
	F.x = hu;
	F.y = h*u*u + 0.5f*g*h*h;
	F.z = hu*v;
	return F;
}

glm::vec3 G(glm::vec3 U)
{
	float h = U.x;
	float hu = U.y;
	float hv = U.z;

	float u = hu/h; //haxxors
	float v = hv/h;
	
	glm::vec3 G;
	G.x = hv;
	G.y = hu*v;
	G.z = h*v*v + 0.5f*(g*h*h);
	return G;
}
//bedslope
float S0x(float terrain_left, float terrain_right)
{
	return terrain_left - terrain_right;
}
//bedslope
float S0y(float terrain_above, float terrain_below)
{
	return terrain_above - terrain_below;
}

float Sfx(float u, float v, float h, float manning_n)
{
	float n2 = manning_n*manning_n;
	float u2 = u*u;
	float v2 = v*v;
	return (n2*u*sqrt(u2 + v2))/(glm::pow(h,4/3));
}

float Sfy(float u, float v, float h, float manning_n)
{
	float n2 = manning_n*manning_n;
	float u2 = u*u;
	float v2 = v*v;
	return (n2*v*sqrt(u2 + v2))/(glm::pow(h,4/3));
}

glm::vec3 R(float u, float v, float h, float terrain_left, float terrain_right, float terrain_above, float terrain_below)
{
	glm::vec3 R;
	R.x = 0.0f;
	R.y = g*h*(S0x(terrain_left,terrain_right) - Sfx(u,v,h,MANNING);
	R.z = g*h*(S0y(terrain_above,terrain_below) - Sfy(u,v,h,MANNING);
	return R;
}

glm::vec3 R(glm::vec3 U, float terrain_left, float terrain_right, float terrain_above, float terrain_below)
{
	float h = U.x;
	float u = U.y/h;
	float v = U.z/h;
	return R(u,v,h,terrain_left,terrain_right,terrain_above,terrain_below);
}

glm::vec3 oneStep(unsigned int i, unsigned int j, float dt, float dx, float dy)
{
	glm::vec3 U = getU(i,j); //get U from the grid at pos i,j
	glm::vec3 U_left = getU(i-1,j);
	glm::vec3 U_right = getU(i+1,j);
	glm::vec3 U_above = getU(i,j-1); //origo top left
	glm::vec3 U_below = getU(i,j+1);
	
	float terrain_left = getTerrain(i-1,j);
	float terrain_right = getTerrain(i+1,j);
	float terrain_above = getTerrain(i,j-1);
	float terrain_below = getTerrain(i,j+1);
	
	glm::vec3 delta_F = F_half(U_right,U) - F_half(U_left,U);
	glm::vec3 delta_G = G_half(U_below,U) - G_half(U_above,U);
	
	return U - (dt/dx)*delta_F - (dt/dy)*delta_G + dt*R(U,terrain_left,terrain_right,terrain_above,terrain_below);
}
