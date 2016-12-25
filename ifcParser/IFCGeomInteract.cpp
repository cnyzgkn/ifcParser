﻿#include "IFCGeomInteract.h"
#include "camera.h"
#include "IFCEngineInteract.h"
#include <list>

extern	bool	showFaces, showWireFrame, enableOnOver;

STRUCT__IFC__OBJECT	* highLightedIfcObject = nullptr;
VECTOR3				vPickRayDir, vPickRayOrig;
VECTOR3				center;
double				size;

extern	STRUCT__IFC__OBJECT				* ifcObjectsLinkedList;
extern	STRUCT__SELECTABLE__TREEITEM	* baseTreeItem;

int_t		noVertices = 0, noIndices = 0;

const float kCameraMoveAmt = 0.002f; // Amount to move camera by
const float kMaxAngle = 89.0f;

float		* pVerticesDeviceBuffer;
int32_t		* pIndicesDeviceBuffer;

void	VECTOR3CROSS(VECTOR3 *pOut, const  VECTOR3 * pV1, const VECTOR3 * pV2)
{
	VECTOR3	v;

	v.x = pV1->y * pV2->z - pV1->z * pV2->y;
	v.y = pV1->z * pV2->x - pV1->x * pV2->z;
	v.z = pV1->x * pV2->y - pV1->y * pV2->x;

	pOut->x = v.x;
	pOut->y = v.y;
	pOut->z = v.z;
}

double	VECTOR3DOT(const VECTOR3 * pV1, const VECTOR3 * pV2)
{
	return	pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

void	VECTOR3NORMALIZE(VECTOR3 * pOut, const VECTOR3 * pV)
{
	double	size;

	size = pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;

	if  (size) {
		pOut->x = pV->x / sqrt(size);
		pOut->y = pV->y / sqrt(size);
		pOut->z = pV->z / sqrt(size);
	}
}

bool	IntersectTriangle(VECTOR3 * v0, VECTOR3 * v1, VECTOR3 * v2, double * pDistance)
{
	VECTOR3	edge1, edge2, pvec, tvec, qvec;
	edge1.x = v1->x - v0->x;
	edge1.y = v1->y - v0->y;
	edge1.z = v1->z - v0->z;
	edge2.x = v2->x - v0->x;
	edge2.y = v2->y - v0->y;
	edge2.z = v2->z - v0->z;

	// Begin calculating determinant - also used to calculate U parameter
    VECTOR3CROSS(&pvec, &vPickRayDir, &edge2);

    // If determinant is near zero, ray lies in plane of triangle
	double	det = VECTOR3DOT(&edge1, &pvec), u, v, eps = 0.000001;
	tvec.x = vPickRayOrig.x - v0->x;
	tvec.y = vPickRayOrig.y - v0->y;
	tvec.z = vPickRayOrig.z - v0->z;

	if	(det > -eps  &&  det < eps) { return  false; }
 
	u = VECTOR3DOT(&tvec, &pvec) / det;
	if	(u < 0  ||  u > 1) { return  false; }
 
    // Prepare to test V parameter
    VECTOR3CROSS(&qvec, &tvec, &edge1);

    // Calculate V parameter and test bounds
    v = VECTOR3DOT(&vPickRayDir, &qvec) / det;
	if	(v < 0  ||  u + v > 1) { return  false; }

    // Calculate t, scale parameters, ray intersects triangle
    double  distance = VECTOR3DOT(&edge2, &qvec) / det;

	(* pDistance) = distance;
	return	true;
}

void	checkPickBoundingBox(STRUCT__IFC__OBJECT * ifcObject, int_t * arrayOfIfcItems, double * arrayOfDistances, int_t * pArraySize)
{
	double			distance = 0, minDistance = 0;
	bool			intersect = false;

	VECTOR3			* vecMin = &ifcObject->vecMin, * vecMax = &ifcObject->vecMax,
					vec0, vec1, vec2;
	//
	//	z = Min (bottom), RHS = downwards
	//
	vec0.x = vecMin->x;
	vec0.y = vecMin->y;
	vec0.z = vecMin->z;
	vec1.x = vecMax->x;
	vec1.y = vecMin->y;
	vec1.z = vecMin->z;
	vec2.x = vecMin->x;
	vec2.y = vecMax->y;
	vec2.z = vecMin->z;
	if	(IntersectTriangle(&vec0, &vec2, &vec1, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}
	vec0.x = vecMax->x;
	vec0.y = vecMax->y;
	if	(IntersectTriangle(&vec0, &vec1, &vec2, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}

	//
	//	z = Max (top), RHS = upwards
	//
	vec0.x = vecMin->x;
	vec0.y = vecMin->y;
	vec0.z = vecMax->z;
	vec1.x = vecMax->x;
	vec1.y = vecMin->y;
	vec1.z = vecMax->z;
	vec2.x = vecMin->x;
	vec2.y = vecMax->y;
	vec2.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec1, &vec2, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}
	vec0.x = vecMax->x;
	vec0.y = vecMax->y;
	if	(IntersectTriangle(&vec0, &vec2, &vec1, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}

	//
	//	y = Min (front)
	//
	vec0.x = vecMin->x;
	vec0.y = vecMin->y;
	vec0.z = vecMin->z;
	vec1.x = vecMax->x;
	vec1.y = vecMin->y;
	vec1.z = vecMin->z;
	vec2.x = vecMin->x;
	vec2.y = vecMin->y;
	vec2.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec1, &vec2, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}
	vec0.x = vecMax->x;
	vec0.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec2, &vec1, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}

	//
	//	y = Max (back)
	//
	vec0.x = vecMin->x;
	vec0.y = vecMax->y;
	vec0.z = vecMin->z;
	vec1.x = vecMax->x;
	vec1.y = vecMax->y;
	vec1.z = vecMin->z;
	vec2.x = vecMin->x;
	vec2.y = vecMax->y;
	vec2.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec2, &vec1, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}
	vec0.x = vecMax->x;
	vec0.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec1, &vec2, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}

	//
	//	x = Min (left)
	//
	vec0.x = vecMin->x;
	vec0.y = vecMin->y;
	vec0.z = vecMin->z;
	vec1.x = vecMin->x;
	vec1.y = vecMax->y;
	vec1.z = vecMin->z;
	vec2.x = vecMin->x;
	vec2.y = vecMin->y;
	vec2.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec2, &vec1, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}
	vec0.y = vecMax->y;
	vec0.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec1, &vec2, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}

	//
	//	x = Max (right)
	//
	vec0.x = vecMax->x;
	vec0.y = vecMin->y;
	vec0.z = vecMin->z;
	vec1.x = vecMax->x;
	vec1.y = vecMax->y;
	vec1.z = vecMin->z;
	vec2.x = vecMax->x;
	vec2.y = vecMin->y;
	vec2.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec1, &vec2, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}
	vec0.y = vecMax->y;
	vec0.z = vecMax->z;
	if	(IntersectTriangle(&vec0, &vec2, &vec1, &distance)) {
		if	(intersect) {
			if	(minDistance > distance) {
				minDistance = distance;
			}
		} else {
			minDistance = distance;
			intersect = true;
		}
	}

	if	(intersect) {
		arrayOfIfcItems[(* pArraySize)] = (int_t) ifcObject;
		arrayOfDistances[(* pArraySize)] = minDistance;
		if	((* pArraySize) < 99) {
			(* pArraySize)++;
		}
	}
}

bool	checkPick(STRUCT__IFC__OBJECT * ifcObject, double * pDistance)
{
	VECTOR3		vec0, vec1, vec2;
	bool		intersect = false;
	int_t		i = 0;
	double		distance = 0;
	if	(ifcObject->noPrimitivesForFaces) {
		while  (i < ifcObject->noPrimitivesForFaces) {
			vec0.x = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+0] + 0] - center.x) / size;
			vec0.y = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+0] + 1] - center.y) / size;
			vec0.z = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+0] + 2] - center.z) / size;
			vec1.x = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+1] + 0] - center.x) / size;
			vec1.y = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+1] + 1] - center.y) / size;
			vec1.z = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+1] + 2] - center.z) / size;
			vec2.x = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+2] + 0] - center.x) / size;
			vec2.y = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+2] + 1] - center.y) / size;
			vec2.z = (ifcObject->vertices[6 * ifcObject->indicesForFaces[3*i+2] + 2] - center.z) / size;
			if	(IntersectTriangle(&vec0, &vec2, &vec1, &distance)) {
				if	(intersect) {
					if	((*pDistance) > distance) {
						(*pDistance) = distance;
					}
				} else {
					(*pDistance) = distance;
					intersect = true;
				}
			}
			i++;
		}
	}

	return	intersect;
}

void	checkPickBoundingBoxNested(int_t * arrayOfIfcItems, double * arrayOfDistances, int_t * pArraySize)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;

	while  (ifcObject) {
		if	(ifcObject->ifcInstance  &&  ifcObject->treeItemGeometry  &&  ifcObject->treeItemGeometry->select == ITEM_CHECKED) {
			checkPickBoundingBox(ifcObject, arrayOfIfcItems, arrayOfDistances, pArraySize);
		}
		ifcObject = ifcObject->next;
	}
}

void	GetDimensions(VECTOR3 * min, VECTOR3 * max, bool * pInitMinMax)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		if	(ifcObject->noVertices) {
			ifcObject->vecMin.x = ifcObject->vertices[0];
			ifcObject->vecMax.x = ifcObject->vecMin.x;
			ifcObject->vecMin.y = ifcObject->vertices[1];
			ifcObject->vecMax.y = ifcObject->vecMin.y;
			ifcObject->vecMin.z = ifcObject->vertices[2];
			ifcObject->vecMax.z = ifcObject->vecMin.z;
			int_t	i = 0;
			while  (i < ifcObject->noVertices) {
				if	(ifcObject->vecMin.x > ifcObject->vertices[6*i + 0]) { ifcObject->vecMin.x = ifcObject->vertices[6*i + 0]; }
				if	(ifcObject->vecMax.x < ifcObject->vertices[6*i + 0]) { ifcObject->vecMax.x = ifcObject->vertices[6*i + 0]; }
				if	(ifcObject->vecMin.y > ifcObject->vertices[6*i + 1]) { ifcObject->vecMin.y = ifcObject->vertices[6*i + 1]; }
				if	(ifcObject->vecMax.y < ifcObject->vertices[6*i + 1]) { ifcObject->vecMax.y = ifcObject->vertices[6*i + 1]; }
				if	(ifcObject->vecMin.z > ifcObject->vertices[6*i + 2]) { ifcObject->vecMin.z = ifcObject->vertices[6*i + 2]; }
				if	(ifcObject->vecMax.z < ifcObject->vertices[6*i + 2]) { ifcObject->vecMax.z = ifcObject->vertices[6*i + 2]; }
				i++;
			}

			if	( (ifcObject->vecMin.x > -1000000  &&  ifcObject->vecMax.x < 1000000)  &&
				  (ifcObject->vecMin.y > -1000000  &&  ifcObject->vecMax.y < 1000000)  &&
				  (ifcObject->vecMin.z > -1000000  &&  ifcObject->vecMax.z < 1000000) ) {
				if	((*pInitMinMax) == false) {
					min->x = ifcObject->vecMin.x;
					max->x = ifcObject->vecMax.x;
					min->y = ifcObject->vecMin.y;
					max->y = ifcObject->vecMax.y;
					min->z = ifcObject->vecMin.z;
					max->z = ifcObject->vecMax.z;
					(*pInitMinMax) = true;
				} else {
					if	(min->x > ifcObject->vecMin.x) {
						min->x = ifcObject->vecMin.x;
					}
					if	(max->x < ifcObject->vecMax.x) {
						max->x = ifcObject->vecMax.x;
					}
					if	(min->y > ifcObject->vecMin.y) {
						min->y = ifcObject->vecMin.y;
					}
					if	(max->y < ifcObject->vecMax.y) {
						max->y = ifcObject->vecMax.y;
					}
					if	(min->z > ifcObject->vecMin.z) {
						min->z = ifcObject->vecMin.z;
					}
					if	(max->z < ifcObject->vecMax.z) {
						max->z = ifcObject->vecMax.z;
					}
				}
			}
		}
		ifcObject = ifcObject->next;
	}
}

void	GetBufferSizes_ifcFaces(int_t * pVBuffSize, int_t * pIBuffSize)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		if	(ifcObject->ifcInstance  &&  ifcObject->noVertices  &&  ifcObject->noPrimitivesForFaces) {
			ifcObject->vertexOffsetForFaces = (int_t) (*pVBuffSize);
			ifcObject->indexOffsetForFaces = (int_t) (*pIBuffSize);

			(*pVBuffSize) += ifcObject->noVertices;
			(*pIBuffSize) += 3 * ifcObject->noPrimitivesForFaces;
		}
		ifcObject = ifcObject->next;
	}
}

void	GetBufferSizes_ifcWireFrame(int_t * pVBuffSize, int_t * pIBuffSize)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		if	(ifcObject->ifcInstance  &&  ifcObject->noVertices  &&  ifcObject->noPrimitivesForWireFrame) {
			ifcObject->vertexOffsetForWireFrame = (int_t) (*pVBuffSize);
			ifcObject->indexOffsetForWireFrame = (int_t) (*pIBuffSize);

			(*pVBuffSize) += ifcObject->noVertices;
			(*pIBuffSize) += 2 * ifcObject->noPrimitivesForWireFrame;
		}
		ifcObject = ifcObject->next;
	}
}

void	AdjustMinMax(VECTOR3 * center, double size)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		if	(ifcObject->ifcInstance  &&  ifcObject->noVertices) {
			ifcObject->vecMin.x = (ifcObject->vecMin.x - center->x) / size;
			ifcObject->vecMin.y = (ifcObject->vecMin.y - center->y) / size;
			ifcObject->vecMin.z = (ifcObject->vecMin.z - center->z) / size;

			ifcObject->vecMax.x = (ifcObject->vecMax.x - center->x) / size;
			ifcObject->vecMax.y = (ifcObject->vecMax.y - center->y) / size;
			ifcObject->vecMax.z = (ifcObject->vecMax.z - center->z) / size;
		}
		ifcObject = ifcObject->next;
	}
}

void	FillVertexBuffers_ifcFaces(int_t * pVBuffSize, float * pVertices, VECTOR3 * center, double size)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		if	(ifcObject->ifcInstance  &&  ifcObject->noVertices  &&  ifcObject->noPrimitivesForFaces) {
			int_t	i = 0;
			while  (i < ifcObject->noVertices) {
				pVertices[6 * ((*pVBuffSize) + i) + 0] = (float) ((ifcObject->vertices[6 * i + 0] - center->x) / size);
				pVertices[6 * ((*pVBuffSize) + i) + 1] = (float) ((ifcObject->vertices[6 * i + 1] - center->y) / size);
				pVertices[6 * ((*pVBuffSize) + i) + 2] = (float) ((ifcObject->vertices[6 * i + 2] - center->z) / size);
				pVertices[6 * ((*pVBuffSize) + i) + 3] = ifcObject->vertices[6 * i + 3];
				pVertices[6 * ((*pVBuffSize) + i) + 4] = ifcObject->vertices[6 * i + 4];
				pVertices[6 * ((*pVBuffSize) + i) + 5] = ifcObject->vertices[6 * i + 5];
				i++;
			}

			ifcObject->vertexOffsetForFaces = (int_t) (*pVBuffSize);

			(*pVBuffSize) += ifcObject->noVertices;
		}
		ifcObject = ifcObject->next;
	}
}

/*
void	FillIndexBuffers_ifcFaces(int_t * pIBuffSize, int32_t * pIndices, D3DMATERIAL9 * mtrl)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		STRUCT_MATERIALS	* materials = ifcObject->materials;
		while  (materials) {
			if	(ifcObject->ifcInstance  &&  ifcObject->noVertices  &&  ifcObject->noPrimitivesForFaces  &&  materials->material->MTRL == (void*) mtrl) {
				int_t	i = 0;
				while  (i < materials->indexArrayPrimitives) {
					pIndices[(*pIBuffSize) + 3 * i + 0] = (int32_t) (ifcObject->indicesForFaces[3 * i + materials->indexArrayOffset + 0] + ifcObject->vertexOffsetForFaces);
					pIndices[(*pIBuffSize) + 3 * i + 1] = (int32_t) (ifcObject->indicesForFaces[3 * i + materials->indexArrayOffset + 1] + ifcObject->vertexOffsetForFaces);
					pIndices[(*pIBuffSize) + 3 * i + 2] = (int32_t) (ifcObject->indicesForFaces[3 * i + materials->indexArrayOffset + 2] + ifcObject->vertexOffsetForFaces);
					i++;
				}

				materials->indexOffsetForFaces = (int_t) (*pIBuffSize);

				(*pIBuffSize) += 3 * (int_t) materials->indexArrayPrimitives;
			}
			materials = materials->next;
		}
		ifcObject = ifcObject->next;
	}
}
*/

void	FillBuffers_ifcWireFrame(int_t * pVBuffSize, int_t * pIBuffSize, float * pVertices, int32_t * pIndices, VECTOR3 * center, double size)
{
	size = size;
	center = center;
	pVertices = pVertices;

	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		if	(ifcObject->ifcInstance  &&  ifcObject->noVertices  &&  ifcObject->noPrimitivesForFaces  &&  ifcObject->noPrimitivesForWireFrame) {
			(*pVBuffSize) = ifcObject->vertexOffsetForFaces;

			int_t	i = 0;
			while  (i < ifcObject->noPrimitivesForWireFrame) {
				pIndices[(*pIBuffSize) + 2 * i + 0] = (int32_t) (ifcObject->indicesForLinesWireFrame[2 * i + 0] + (*pVBuffSize));
				pIndices[(*pIBuffSize) + 2 * i + 1] = (int32_t) (ifcObject->indicesForLinesWireFrame[2 * i + 1] + (*pVBuffSize));
				i++;
			}

			ifcObject->vertexOffsetForWireFrame = (int_t) (*pVBuffSize);
			ifcObject->indexOffsetForWireFrame = (int_t) (*pIBuffSize);

			(*pVBuffSize) += ifcObject->noVertices;
			(*pIBuffSize) += 2 * ifcObject->noPrimitivesForWireFrame;
		}
		ifcObject = ifcObject->next;
	}
}


/*
void CRightPane::InitializeDeviceBuffer()
{
	if	(ifcObjectsLinkedList) {
		VECTOR3	min, max;
		bool			initSizes = false;
		GetDimensions(&min, &max, &initSizes);

		if	(initSizes) {
			center.x = (max.x + min.x)/2;
			center.y = (max.y + min.y)/2;
			center.z = (max.z + min.z)/2;
			size = max.x - min.x;
			if	(size < max.y - min.y) { size = max.y - min.y; }
			if	(size < max.z - min.z) { size = max.z - min.z; }

			int_t	vBuffSize = 0, iBuffSize = 0, tmpVBuffSize = 0;
			GetBufferSizes_ifcFaces(&vBuffSize, &iBuffSize);
			tmpVBuffSize = vBuffSize;
			vBuffSize = 0;
			GetBufferSizes_ifcWireFrame(&vBuffSize, &iBuffSize);

			if	(!DirectXStatus) {
				if	(vBuffSize) {
					if( FAILED( g_pd3dDevice->CreateVertexBuffer( ((int32_t) vBuffSize) * sizeof(CUSTOMVERTEX),
																   0, D3DFVF_CUSTOMVERTEX,
																   D3DPOOL_DEFAULT, &g_pVB, NULL ) ) ) {
						DirectXStatus = -1;
						return;
					}
				}

				if	(iBuffSize) {
					if( FAILED( g_pd3dDevice->CreateIndexBuffer( ((int32_t) iBuffSize) * sizeof(int32_t),
																  0, D3DFMT_INDEX32,
																  D3DPOOL_DEFAULT, &g_pIB, NULL ) ) ) {
						DirectXStatus = -1;
						return;
					}
				}

				if( FAILED( g_pVB->Lock( 0, (int32_t) vBuffSize*6*sizeof(float), (void **)&pVerticesDeviceBuffer, 0 ) ) ) {
					DirectXStatus = -1;
					return;
				}

				if( FAILED( g_pIB->Lock( 0, (int32_t) iBuffSize*sizeof(int32_t), (void **)&pIndicesDeviceBuffer, 0 ) ) ) {
					DirectXStatus = -1;
					return;
				}

				int_t vertexCnt = 0, indexCnt = 0;

				AdjustMinMax(&center, size);

				FillVertexBuffers_ifcFaces(&vertexCnt, pVerticesDeviceBuffer, &center, size);
				FillIndexBuffers_ifcFaces(&indexCnt, pIndicesDeviceBuffer, 0);
				for  (std::list<D3DMATERIAL9*>::iterator it=mtrls.begin() ; it != mtrls.end(); ++it) {
					FillIndexBuffers_ifcFaces(&indexCnt, pIndicesDeviceBuffer, (*it));
				}
				vertexCnt = 0;
				FillBuffers_ifcWireFrame(&vertexCnt, &indexCnt, pVerticesDeviceBuffer, pIndicesDeviceBuffer, &center, size);

				if	(FAILED( g_pIB->Unlock())) {
					DirectXStatus = -1;
					return;
				}
		
				if	(FAILED( g_pVB->Unlock())) {
					DirectXStatus = -1;
					return;
				}
			}
		}
	}
}

void	CRightPane::RenderFacesHighLighted()
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while	(ifcObject) {
		if	(ifcObject->treeItemGeometry  &&  ifcObject->treeItemGeometry->select == ITEM_CHECKED  &&  ifcObject == highLightedIfcObject) {
			if	(ifcObject->noPrimitivesForFaces) {
				STRUCT_MATERIALS	* materials = ifcObject->materials;
				while  (materials) {
					g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,  (int32_t) ifcObject->vertexOffsetForFaces,  (int32_t) ifcObject->noVertices,  (int32_t) materials->indexOffsetForFaces,  (int32_t) materials->indexArrayPrimitives);
					materials = materials->next;
				}
			}
		}
		ifcObject = ifcObject->next;
	}
}

bool		facesToDraw = false;
long long	vertexOffsetForFaces,
			noVerticesForFaces,
			indexOffsetForFaces,
			noPrimitivesForFaces;

void	CRightPane::RenderFaces(D3DMATERIAL9 * mtrl)
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while	(ifcObject) {
		STRUCT_MATERIALS	* materials = ifcObject->materials;
		while  (materials) {
			if	(ifcObject->treeItemGeometry  &&  ifcObject->treeItemGeometry->select == ITEM_CHECKED  &&  ifcObject->noPrimitivesForFaces  &&  materials->material->MTRL == (void*) mtrl  &&  ifcObject != highLightedIfcObject) {
				if	(facesToDraw) {
					if	(indexOffsetForFaces + 3 * noPrimitivesForFaces == materials->indexOffsetForFaces) {
						if	(ifcObject->vertexOffsetForFaces < vertexOffsetForFaces) {
							noVerticesForFaces +=  vertexOffsetForFaces - ifcObject->vertexOffsetForFaces;
							if	(noVerticesForFaces < ifcObject->noVertices) {
								noVerticesForFaces = ifcObject->noVertices;
							}
							vertexOffsetForFaces = ifcObject->vertexOffsetForFaces;
						} else {
							if	(noVerticesForFaces < ifcObject->noVertices + ifcObject->vertexOffsetForFaces - vertexOffsetForFaces) {
								noVerticesForFaces = ifcObject->noVertices + ifcObject->vertexOffsetForFaces - vertexOffsetForFaces;
							}
						}
						noPrimitivesForFaces += materials->indexArrayPrimitives;
					} else {
						g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (int32_t) vertexOffsetForFaces,  (int32_t) noVerticesForFaces,  (int32_t) indexOffsetForFaces,  (int32_t) noPrimitivesForFaces);
						facesToDraw = false;
					}
				}

				if	(facesToDraw == false) {
					vertexOffsetForFaces = ifcObject->vertexOffsetForFaces;
					noVerticesForFaces = ifcObject->noVertices;
					indexOffsetForFaces = materials->indexOffsetForFaces;
					noPrimitivesForFaces = materials->indexArrayPrimitives;
					facesToDraw = true;
				}
			}
			materials = materials->next;
		}
		ifcObject = ifcObject->next;
	}
}

bool		wireFrameToDraw = false;
long long	vertexOffsetForWireFrame,
			noVerticesForWireFrame,
			indexOffsetForWireFrame,
			noPrimitivesForWireFrame;

void	CRightPane::RenderWireFrame()
{
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while  (ifcObject) {
		if	(ifcObject->treeItemGeometry  &&  ifcObject->treeItemGeometry->select == ITEM_CHECKED  &&  ifcObject->noPrimitivesForWireFrame) {
			if	(wireFrameToDraw) {
				if	(indexOffsetForWireFrame + 2 * noPrimitivesForWireFrame == ifcObject->indexOffsetForWireFrame) {
					if	(ifcObject->vertexOffsetForWireFrame < vertexOffsetForWireFrame) {
						noVerticesForWireFrame +=  vertexOffsetForWireFrame - ifcObject->vertexOffsetForWireFrame;
						if	(noVerticesForWireFrame < ifcObject->noVertices) {
							noVerticesForWireFrame = ifcObject->noVertices;
						}
						vertexOffsetForWireFrame = ifcObject->vertexOffsetForWireFrame;
					} else {
						if	(noVerticesForWireFrame < ifcObject->noVertices + ifcObject->vertexOffsetForWireFrame - vertexOffsetForWireFrame) {
							noVerticesForWireFrame = ifcObject->noVertices + ifcObject->vertexOffsetForWireFrame - vertexOffsetForWireFrame;
						}
					}
					noPrimitivesForWireFrame += ifcObject->noPrimitivesForWireFrame;
				} else {
					g_pd3dDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, (int32_t) vertexOffsetForWireFrame,  (int32_t) noVerticesForWireFrame,  (int32_t) indexOffsetForWireFrame,  (int32_t) noPrimitivesForWireFrame);
					wireFrameToDraw = false;
				}
			}

			if	(wireFrameToDraw == false) {
				vertexOffsetForWireFrame = ifcObject->vertexOffsetForWireFrame;
				noVerticesForWireFrame = ifcObject->noVertices;
				indexOffsetForWireFrame = ifcObject->indexOffsetForWireFrame;
				noPrimitivesForWireFrame = ifcObject->noPrimitivesForWireFrame;
				wireFrameToDraw = true;
			}
		}
		ifcObject = ifcObject->next;
	}
}

void	CRightPane::Render()
{
	if	(!DirectXStatus) {
		// Clear the backbuffer and the zbuffer
		if( FAILED( g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
										 D3DCOLOR_XRGB(255,255,255), 1.0f, 0 ) ) ) {
			DirectXStatus = -1;
			return;
		}

		// Begin the scene
		if	(SUCCEEDED(g_pd3dDevice->BeginScene()))
		{
			// Setup the lights and materials
			if	(SetupLights()) {
				DirectXStatus = -1;
				return;
			}

			// Setup the world, view, and projection matrices
			if	(SetupMatrices()) {
				DirectXStatus = -1;
				return;
			}


			if	(g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX))) {
				DirectXStatus = -1;
				return;
			}

			if( g_pd3dDevice->SetIndices( g_pIB ) ) {
				DirectXStatus = -1;
				return;
			}

			if	(g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX)) {
				DirectXStatus = -1;
				return;
			}

			if	(showFaces) {

				//
				//	First the non-transparent faces
				//

				for  (std::list<D3DMATERIAL9*>::iterator it=mtrls.begin() ; it != mtrls.end(); ++it) {
					if	((*it)->Ambient.a == 1) {
						g_pd3dDevice->SetMaterial(*it);
						RenderFaces(*it);

						if	(facesToDraw) {
							g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (int32_t) vertexOffsetForFaces,  (int32_t) noVerticesForFaces,  (int32_t) indexOffsetForFaces,  (int32_t) noPrimitivesForFaces);
							facesToDraw = false;
						}
					}
				}

				g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
				g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
				g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

				//
				//	Now the transparant faces
				//
				for  (std::list<D3DMATERIAL9*>::iterator it=mtrls.begin() ; it != mtrls.end(); ++it) {
					if	((*it)->Ambient.a < 1) {
						g_pd3dDevice->SetMaterial(*it);
						RenderFaces(*it);

						if	(facesToDraw) {
							g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (int32_t) vertexOffsetForFaces,  (int32_t) noVerticesForFaces,  (int32_t) indexOffsetForFaces,  (int32_t) noPrimitivesForFaces);
							facesToDraw = false;
						}
					}
				}

				g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

				g_pd3dDevice->SetMaterial(&mtrlRed);
				RenderFacesHighLighted();
			}

			if	(showWireFrame) {
				g_pd3dDevice->SetMaterial(&mtrlBlack);

				RenderWireFrame();

				if	(wireFrameToDraw) {
					g_pd3dDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, (int32_t) vertexOffsetForWireFrame,  (int32_t) noVerticesForWireFrame,  (int32_t) indexOffsetForWireFrame,  (int32_t) noPrimitivesForWireFrame);
					wireFrameToDraw = false;
				}
			}

			// End the scene
			if( FAILED( g_pd3dDevice->EndScene() ) ) {
				DirectXStatus = -1;
				return;
			}
		}

		// Present the backbuffer contents to the display
		if( FAILED( g_pd3dDevice->Present( nullptr, nullptr, nullptr, nullptr ) ) ) {
			//DirectXStatus = -1;
			return;
		}
	}
}

bool	 CRightPane::SetupLights()
{
//	mtrlDefault.Diffuse.r = mtrlDefault.Ambient.r = mtrlDefault.Specular.r = 0.1f;
//	mtrlDefault.Diffuse.g = mtrlDefault.Ambient.g = mtrlDefault.Specular.g = 0.1f;
//	mtrlDefault.Diffuse.b = mtrlDefault.Ambient.b = mtrlDefault.Specular.b = 0.2f;
//	mtrlDefault.Diffuse.a = mtrlDefault.Ambient.a = mtrlDefault.Specular.a = 1.0f;
//	mtrlDefault.Emissive.r = 0.1f;
//	mtrlDefault.Emissive.g = 0.4f;
//	mtrlDefault.Emissive.b = 0.02f;
//	mtrlDefault.Emissive.a = 0.5f;
//	mtrlDefault.Power = 0.5f;

	mtrlBlack.Diffuse.r = mtrlBlack.Ambient.r = mtrlBlack.Specular.r = 0.0f;
	mtrlBlack.Diffuse.g = mtrlBlack.Ambient.g = mtrlBlack.Specular.g = 0.0f;
	mtrlBlack.Diffuse.b = mtrlBlack.Ambient.b = mtrlBlack.Specular.b = 0.0f;
	mtrlBlack.Diffuse.a = mtrlBlack.Ambient.a = mtrlBlack.Specular.a = 1.0f;
	mtrlBlack.Emissive.r = 0.0f;
	mtrlBlack.Emissive.g = 0.0f;
	mtrlBlack.Emissive.b = 0.0f;
	mtrlBlack.Emissive.a = 0.5f;
	mtrlBlack.Power = 0.5f;

	mtrlRed.Diffuse.r = mtrlRed.Ambient.r = mtrlRed.Specular.r = 0.4f;
	mtrlRed.Diffuse.g = mtrlRed.Ambient.g = mtrlRed.Specular.g = 0.05f;
	mtrlRed.Diffuse.b = mtrlRed.Ambient.b = mtrlRed.Specular.b = 0.05f;
	mtrlRed.Diffuse.a = mtrlRed.Ambient.a = mtrlRed.Specular.a = 1.0f;
	mtrlRed.Emissive.r = 0.1f;
	mtrlRed.Emissive.g = 0.02f;
	mtrlRed.Emissive.b = 0.02f;
	mtrlRed.Emissive.a = 0.5f;
	mtrlRed.Power = 0.5f;

	// Finally, turn on some ambient light.
    //g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00202020 );

    // Set up a white, directional light, with an oscillating direction.
    // Note that many lights may be active at a time (but each one slows down
    // the rendering of our scene). However, here we are just using one. Also,
    // we need to set the D3DRS_LIGHTING renderstate to enable lighting
    D3DXVECTOR3 vecDir;
    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type       = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = 0.1f;
	light.Diffuse.g  = 0.1f;
	light.Diffuse.b  = 0.1f;
	light.Diffuse.a  = 0.1f;
	light.Specular.r = 0.3f;
	light.Specular.g = 0.3f;
	light.Specular.b = 0.3f;
	light.Specular.a = 0.1f;//0.5f;
	light.Ambient.r  = 0.7f;
	light.Ambient.g  = 0.7f;
	light.Ambient.b  = 0.7f;
	light.Ambient.a  = 0.1f;
    light.Position.x = (float) 2.0f;
    light.Position.y = (float) 2.0f;
    light.Position.z = (float) 0.0f;
    vecDir.x = -3.0f;
    vecDir.y = -6.0f;
    vecDir.z = -2.0f;
    D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
    light.Range       = 10.0f;
//	if	(FAILED(g_pd3dDevice->SetLight(0, &light))) {
//		DirectXStatus = -1;
//		return	true;
//	}

//	if	(FAILED(g_pd3dDevice->LightEnable(0, TRUE))) {
//		DirectXStatus = -1;
//		return	true;
//	}

    D3DLIGHT9 light1;
    ZeroMemory(&light1, sizeof(D3DLIGHT9));
    light1.Type       = D3DLIGHT_DIRECTIONAL;
	light1.Diffuse.r  = 0.2f;
	light1.Diffuse.g  = 0.2f;
	light1.Diffuse.b  = 0.2f;
	light1.Diffuse.a  = 1.0f;
	light1.Specular.r = 0.2f;//0.3f;
	light1.Specular.g = 0.2f;//0.3f;
	light1.Specular.b = 0.2f;//0.3f;
	light1.Specular.a = 1.0f;//0.5f;
	light1.Ambient.r  = 0.2f;//0.7f;
	light1.Ambient.g  = 0.2f;//0.7f;
	light1.Ambient.b  = 0.2f;//0.7f;
	light1.Ambient.a  = 1.0f;
    light1.Position.x = (float) -1.0f;
    light1.Position.y = (float) -1.0f;
    light1.Position.z = (float) -0.5f;
    vecDir.x = 1.0f;
    vecDir.y = 1.0f;
    vecDir.z = 0.5f;
    D3DXVec3Normalize((D3DXVECTOR3*)&light1.Direction, &vecDir);
    light1.Range       = 2.0f;
	if	(FAILED(g_pd3dDevice->SetLight(0, &light1))) {
		DirectXStatus = -1;
		return	true;
	}

    D3DLIGHT9 light2;
    ZeroMemory(&light2, sizeof(D3DLIGHT9));
    light2.Type       = D3DLIGHT_DIRECTIONAL;
	light2.Diffuse.r  = 0.2f;
	light2.Diffuse.g  = 0.2f;
	light2.Diffuse.b  = 0.2f;
	light2.Diffuse.a  = 1.0f;
	light2.Specular.r = 0.2f;//0.3f;
	light2.Specular.g = 0.2f;//0.3f;
	light2.Specular.b = 0.2f;//0.3f;
	light2.Specular.a = 1.0f;//0.5f;
	light2.Ambient.r  = 0.2f;//0.7f;
	light2.Ambient.g  = 0.2f;//0.7f;
	light2.Ambient.b  = 0.2f;//0.7f;
	light2.Ambient.a  = 1.0f;
    light2.Position.x = (float) 1.0f;
    light2.Position.y = (float) 1.0f;
    light2.Position.z = (float) 0.5f;
    vecDir.x = -1.0f;
    vecDir.y = -1.0f;
    vecDir.z = -0.5f;
    D3DXVec3Normalize((D3DXVECTOR3*)&light2.Direction, &vecDir);
    light2.Range       = 2.0f;
	if	(FAILED(g_pd3dDevice->SetLight(1, &light2))) {
		DirectXStatus = -1;
		return	true;
	}

	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00000000 );

	if	(FAILED(g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE))) {
		DirectXStatus = -1;
		return	true;
	}


    // Finally, turn on some ambient light.
//    g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00707070);

	return	false;
}

bool	CRightPane::SetupMatrices()
{
    // For our world matrix, we will just leave it as the identity
    D3DXMATRIX	matWorld;
    D3DXMatrixIdentity( &matWorld );
	
	matWorld._22 = -1.0f;

	D3DXVec3TransformCoord((D3DXVECTOR3 *) &matWorld._41, &m_vectorOrigin, &matWorld);

	if( FAILED( g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld ) ) ) {
		DirectXStatus = -1;
		return	true;
	}

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.

	double	counter = 0;
    D3DXVECTOR3 vEyePt(2.0f, 3 * ((float) sin(counter)), 4 * ((float) cos(counter)));
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVector(0.0f, 1.0f, 0.0f);
    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVector );
	if( FAILED( g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView ) ) ) {
		DirectXStatus = -1;
		return	true;
	}

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4,  (float) m_iWidth/(float) m_iHeight, 0.03f, 10.0f );
	if( FAILED( g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj ) ) ) {
		DirectXStatus = -1;
		return	true;
	}

	D3DXMATRIXA16 matrix;

	// Create "D3D Vector" versions of our camera's eye, look at position, and up vector
	D3DXVECTOR3 eye(gCamera->getEye().z, gCamera->getEye().x, gCamera->getEye().y);
	D3DXVECTOR3 lookAt(gCamera->getTarget().z, gCamera->getTarget().x, gCamera->getTarget().y);
	D3DXVECTOR3 up(0, 0, 1); // The world's up vector

	// We create a matrix that represents our camera's view of the world.  Notice
	// the "LH" on the end of the function.  That says, "Hey create this matrix for
	// a left handed coordinate system".
	D3DXMatrixLookAtLH(&matrix, &eye, &lookAt, &up);
	if( FAILED( g_pd3dDevice->SetTransform( D3DTS_VIEW, &matrix ) ) ) {
		DirectXStatus = -1;
	}

	return	false;
}
*/
