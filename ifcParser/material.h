#ifndef MATERIAL_H
#define MATERIAL_H

#include "ifcengine/include/engdef.h"

struct STRUCT_COLOR {
	float							R;
	float							G;
	float							B;
	float							A;
};

struct STRUCT_MATERIAL_VALUE {
	STRUCT_COLOR					ambient;
	STRUCT_COLOR					diffuse;
	STRUCT_COLOR					specular;
	STRUCT_COLOR					emissive;

	float							transparency;
	float							shininess;
};

struct STRUCT_MATERIAL {
	bool							active;

	STRUCT_COLOR					ambient;
	STRUCT_COLOR					diffuse;
	STRUCT_COLOR					specular;
	STRUCT_COLOR					emissive;

	double							transparency;
	double							shininess;

	STRUCT_MATERIAL_VALUE			* MTRL;

	STRUCT_MATERIAL					* next;
	STRUCT_MATERIAL					* prev;
};

struct STRUCT_MATERIALS {
	int64_t							indexArrayOffset;
	int64_t							indexArrayPrimitives;

	int64_t							indexOffsetForFaces;

	STRUCT_MATERIAL					* material;

	STRUCT_MATERIALS				* next;
};

struct STRUCT_IFCOBJECT_MATERIAL {
	int_t							indexOffsetForFaces;
	int_t							indexArrayPrimitives;
	STRUCT_MATERIAL_VALUE			materialValue;

	STRUCT_IFCOBJECT_MATERIAL(int_t IndexOffsetForFaces, int_t IndexArrayPrimitives, STRUCT_MATERIAL_VALUE MaterialValue)
		:indexOffsetForFaces(IndexOffsetForFaces), indexArrayPrimitives(IndexArrayPrimitives), materialValue(MaterialValue) {};
};

void				initializeMaterial(int_t owlModel);
STRUCT_MATERIALS	* ifcObjectMaterial(int_t ifcModel, int_t ifcInstance);

#endif