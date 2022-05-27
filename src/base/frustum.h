#pragma once

#include <iostream>
#include "plane.h"
#include "bounding_box.h"

struct Frustum {
public:
	Plane planes[6];
	enum {
		LeftFace = 0,
		RightFace = 1,
		BottomFace = 2,
		TopFace = 3,
		NearFace = 4,
		FarFace = 5
	};

	bool Inplanes(const Plane plan,const glm::vec3 max,const glm::vec3 min) const
	{
		glm::vec3 points[] = {
		// Min side
		glm::vec3(min.x, min.y, min.z),
		glm::vec3(max.x, min.y, min.z),
		glm::vec3(min.x, max.y, min.z),
		glm::vec3(min.x, min.y, max.z),
		// Max side
		glm::vec3(max.x, max.y, max.z),
		glm::vec3(min.x, max.y, max.z),
		glm::vec3(max.x, min.y, max.z),
		glm::vec3(max.x, max.y, min.z)
		};
		for (int i=0;i<8;i++){
			if(plan.getSignedDistanceToPoint(points[i])>0) return true;
		}
		return false;
	}

	bool intersect(const BoundingBox& aabb, const glm::mat4& modelMatrix) const {
		// TODO: judge whether the frustum intersects the bounding box
		// write your code here
		// ------------------------------------------------------------
		glm::vec3 max{modelMatrix * glm::vec4(aabb.max, 1.0f) };
		glm::vec3 min{modelMatrix * glm::vec4(aabb.min,1.0f)};

		if(!Inplanes(planes[LeftFace],max,min)) return false;
		if(!Inplanes(planes[RightFace],max,min)) return false;
		if(!Inplanes(planes[BottomFace],max,min)) return false;
		if(!Inplanes(planes[TopFace],max,min)) return false;
		if(!Inplanes(planes[NearFace],max,min)) return false;
		if(!Inplanes(planes[FarFace],max,min)) return false;
		return true;
		// ------------------------------------------------------------
	}
};

inline std::ostream& operator<<(std::ostream& os, const Frustum& frustum) {
	os << "frustum: \n";
	os << "planes[Left]:   " << frustum.planes[0] << "\n";
	os << "planes[Right]:  " << frustum.planes[1] << "\n";
	os << "planes[Bottom]: " << frustum.planes[2] << "\n";
	os << "planes[Top]:    " << frustum.planes[3] << "\n";
	os << "planes[Near]:   " << frustum.planes[4] << "\n";
	os << "planes[Far]:    " << frustum.planes[5] << "\n";

	return os;
}