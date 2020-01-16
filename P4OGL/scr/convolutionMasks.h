#ifndef __CONVOLUTION_MASKS__
#define __CONVOLUTION_MASKS__

#include <glm/glm.hpp>

const float maskFactor1 = float(1.0 / 14.0);
const int maskSize9 = 9;
const glm::vec2 texIdx9[maskSize9] = {
	glm::vec2(-1.0, 1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0),
	glm::vec2(-1.0, 0.0), glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0),
	glm::vec2(-1.0, -1.0), glm::vec2(0.0, -1.0), glm::vec2(1.0, -1.0) 
};

 const float mask1[maskSize9] = {
	float(1.0 * maskFactor1), float(2.0 * maskFactor1), float(1.0 * maskFactor1),
	float(2.0 * maskFactor1), float(2.0 * maskFactor1), float(2.0 * maskFactor1),
	float(1.0 * maskFactor1), float(2.0 * maskFactor1), float(1.0 * maskFactor1)
};

 const float laplacianEdgeFilterMask[maskSize9] = {
	  -1.0, -1.0, -1.0,
	  -1.0, 8.0, -1.0,
	  -1.0, -1.0, -1.0 
 };

 const  float northDirectionMask[maskSize9] = {
	  -3.0, -3.0, 5.0,
	  -3.0, 0.0, 5.0,
	  -3.0, -3.0, 5.0
 };

 const float embossFilterMask[maskSize9] = {
	 2.0, 0.0, 0.0,
	 0.0, -1.0, 0.0,
	 0.0, 0.0, -1.0
 };


const float maskFactor2 = float(1.0 / 65.0);
const int maskSize25 = 25;
const glm::vec2 texIdx25[maskSize25] = {
	glm::vec2(-2.0, 2.0), glm::vec2(-1.0, 2.0), glm::vec2(0.0, 2.0), glm::vec2(1.0, 2.0), glm::vec2(2.0, 2.0),
	glm::vec2(-2.0, 1.0), glm::vec2(-1.0, 1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0), glm::vec2(2.0, 1.0),
	glm::vec2(-2.0, 0.0), glm::vec2(-1.0, 0.0), glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0), glm::vec2(2.0, 0.0),
	glm::vec2(-2.0, -1.0), glm::vec2(-1.0, -1.0), glm::vec2(0.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(2.0, -1.0),
	glm::vec2(-2.0, -2.0), glm::vec2(-1.0, -2.0), glm::vec2(0.0, -2.0), glm::vec2(1.0, -2.0), glm::vec2(2.0, -2.0) };

	const float mask25[maskSize25] = {
		1.0 * maskFactor2, 2.0 * maskFactor2, 3.0 * maskFactor2, 2.0 * maskFactor2, 1.0 * maskFactor2,
		2.0 * maskFactor2, 3.0 * maskFactor2, 4.0 * maskFactor2, 3.0 * maskFactor2, 2.0 * maskFactor2,
		3.0 * maskFactor2, 4.0 * maskFactor2, 5.0 * maskFactor2, 4.0 * maskFactor2, 3.0 * maskFactor2,
		2.0 * maskFactor2, 3.0 * maskFactor2, 4.0 * maskFactor2, 3.0 * maskFactor2, 2.0 * maskFactor2,
		1.0 * maskFactor2, 2.0 * maskFactor2, 3.0 * maskFactor2, 2.0 * maskFactor2, 1.0 * maskFactor2
	};


#endif