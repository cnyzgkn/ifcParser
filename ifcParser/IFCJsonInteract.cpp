#include "IFCJsonInteract.h"
#include "IFCEngineInteract.h"
#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include "generic.h"

// need to delete the return char* pointer
char *wchar_t2char(const wchar_t* src)
{
	if (src == NULL)
		return NULL;

	char *str = new char[wcslen(src) + 1];
	wcstombs(str, src, wcslen(src) + 1);
	return str;
}

// need to delete the return wchar_t* pointer
wchar_t *char2wchar_t(const char* src)
{
	if (src == NULL)
		return NULL;

	wchar_t *str = new wchar_t[strlen(src) + 1];
	mbstowcs(str, src, strlen(src) + 1);
	return str;
}

void DumpIfcObjects2SeperateJson(const std::string& fileName)
{
	long int num = 1;

	Json::Value root;
	Json::FastWriter writer;
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while (ifcObject) {
		Json::Value JsonIFCObject;
		//GUID
		const char* strGUID = wchar_t2char(ifcObject->globalId);
		JsonIFCObject["globalId"] = strGUID;
		delete strGUID;
		//Type
		const char* strifcType = wchar_t2char(ifcObject->ifcType);
		JsonIFCObject["ifcType"] = strifcType;
		delete strifcType;
		//Instance
		//JsonIFCObject["ifcInstance"] = ifcObject->ifcInstance;
		//Entity
		//JsonIFCObject["ifcEntity"] = ifcObject->ifcEntity;
		//noVertices
		JsonIFCObject["noVertices"] = ifcObject->noVertices;
		//vertices length = 6 * noVertices
		for (int i = 0; i < 6 * ifcObject->noVertices; i++)
		{
			JsonIFCObject["vertices"].append(ifcObject->vertices[i]);
		}
		//noPrimitivesForFaces
		JsonIFCObject["noPrimitivesForFaces"] = ifcObject->noPrimitivesForFaces;
		//indicesForFaces length == 3 * ifcObject->noPrimitivesForFaces
		for (int i = 0; i < 3 * ifcObject->noPrimitivesForFaces; i++)
		{
			JsonIFCObject["indicesForFaces"].append(ifcObject->indicesForFaces[i]);
		}
		//vertexOffsetForFaces
		JsonIFCObject["vertexOffsetForFaces"] = ifcObject->vertexOffsetForFaces;
		/*
		//indexOffsetForFaces
		JsonIFCObject["indexOffsetForFaces"] = ifcObject->indexOffsetForFaces;
		//noPrimitivesForWireFrame
		JsonIFCObject["noPrimitivesForWireFrame"] = ifcObject->noPrimitivesForWireFrame;
		//indicesForLinesWireFrame length == 2 * ifcObject->noPrimitivesForWireFrame
		for (int i = 0; i < 2 * ifcObject->noPrimitivesForWireFrame; i++)
		{
			JsonIFCObject["indicesForLinesWireFrame"].append(ifcObject->indicesForLinesWireFrame[i]);
		}
		//vertexOffsetForWireFrame
		JsonIFCObject["vertexOffsetForWireFrame"] = ifcObject->vertexOffsetForWireFrame;
		//indexOffsetForWireFrame
		JsonIFCObject["indexOffsetForWireFrame"] = ifcObject->indexOffsetForWireFrame;
		*/
		if (ifcObject->noPrimitivesForFaces) {
			STRUCT_MATERIALS *materials = ifcObject->materials;
			while (materials) {
				//g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (int32_t)ifcObject->vertexOffsetForFaces, (int32_t)ifcObject->noVertices, (int32_t)materials->indexOffsetForFaces, (int32_t)materials->indexArrayPrimitives);
				Json::Value JsonMaterials;
				JsonMaterials["indexArrayOffset"] = (int32_t)materials->indexArrayOffset;
				JsonMaterials["indexArrayPrimitives"] = (int32_t)materials->indexArrayPrimitives;
				STRUCT_MATERIAL *material = materials->material;
				while (material) {
					Json::Value JsonMaterial;
					JsonMaterial["active"] = material->active;
					//ambient
					JsonMaterial["ambient_R"] = material->ambient.R;
					JsonMaterial["ambient_G"] = material->ambient.G;
					JsonMaterial["ambient_B"] = material->ambient.B;
					JsonMaterial["ambient_A"] = material->ambient.A;
					//diffuse
					JsonMaterial["diffuse_R"] = material->diffuse.R;
					JsonMaterial["diffuse_G"] = material->diffuse.G;
					JsonMaterial["diffusevB"] = material->diffuse.B;
					JsonMaterial["diffuse_A"] = material->diffuse.A;
					//specular
					JsonMaterial["specular_R"] = material->specular.R;
					JsonMaterial["specular_G"] = material->specular.G;
					JsonMaterial["specular_B"] = material->specular.B;
					JsonMaterial["specular_A"] = material->specular.A;
					//emissive
					JsonMaterial["emissive_R"] = material->emissive.R;
					JsonMaterial["emissive_G"] = material->emissive.G;
					JsonMaterial["emissive_B"] = material->emissive.B;
					JsonMaterial["emissive_A"] = material->emissive.A;
					//
					JsonMaterial["transparency"] = material->transparency;
					JsonMaterial["shininess"] = material->shininess;
					//
					JsonMaterials["material"].append(JsonMaterial);
					material = material->next;
				}
				JsonIFCObject["materials"].append(JsonMaterials);
				materials = materials->next;
			}
		}

		root.append(JsonIFCObject);
		if (num%30 == 0)
		{
			const std::string newFileName = fileName + std::to_string(num/30) + std::string(".json");
			std::ofstream ofs;
			ofs.open(newFileName.c_str(), std::ios_base::trunc | std::ios_base::out);
			std::string styledJsonFile = root.toStyledString();
			ofs << styledJsonFile;
			ofs.close();
			root.clear();
		}
		else if (ifcObject->next == NULL)
		{
			const std::string newFileName = fileName + std::to_string(num/30+1) + std::string(".json");
			std::ofstream ofs;
			ofs.open(newFileName.c_str(), std::ios_base::trunc | std::ios_base::out);
			std::string styledJsonFile = root.toStyledString();
			ofs << styledJsonFile;
			ofs.close();
			root.clear();
		}

		ifcObject = ifcObject->next;
		++num;
		//std::cout << "mum == " << num << std::endl;
	}

	/*
	//std::string json_file = writer.write(root);
	std::string styledJsonFile = root.toStyledString();
	std::ofstream ofs;
	ofs.open(fileName.c_str());
	ofs << styledJsonFile;
	*/

	CleanupIfcFile();

	//std::string out = root.toStyledString();
	//std::cout << out << std::endl;
}

void DumpIfcObjects2Json(const std::string& fileName)
{
	//output "["
	std::ofstream ofs;
	ofs.open(fileName.c_str(), std::ios_base::trunc | std::ios_base::out);
	ofs << "\[" << std::endl;
	ofs.close();

	//output ifcObject content
	ofs.open(fileName.c_str(), std::ios_base::app | std::ios_base::out);

	Json::Value root;
	Json::FastWriter writer;
	STRUCT__IFC__OBJECT	* ifcObject = ifcObjectsLinkedList;
	while (ifcObject) {
		Json::Value JsonIFCObject;
		//GUID
		const char* strGUID = wchar_t2char(ifcObject->globalId);
		JsonIFCObject["globalId"] = strGUID;
		delete strGUID;
		//Type
		const char* strifcType = wchar_t2char(ifcObject->ifcType);
		JsonIFCObject["ifcType"] = strifcType;
		delete strifcType;
		//Instance
		//JsonIFCObject["ifcInstance"] = ifcObject->ifcInstance;
		//Entity
		//JsonIFCObject["ifcEntity"] = ifcObject->ifcEntity;
		//noVertices
		JsonIFCObject["noVertices"] = ifcObject->noVertices;
		//vertices length = 6 * noVertices
		for (int i = 0; i < 6 * ifcObject->noVertices; i++)
		{
			JsonIFCObject["vertices"].append(ifcObject->vertices[i]);
		}
		//noPrimitivesForFaces
		JsonIFCObject["noPrimitivesForFaces"] = ifcObject->noPrimitivesForFaces;
		//indicesForFaces length == 3 * ifcObject->noPrimitivesForFaces
		for (int i = 0; i < 3 * ifcObject->noPrimitivesForFaces; i++)
		{
			JsonIFCObject["indicesForFaces"].append(ifcObject->indicesForFaces[i]);
		}

		//root.append(JsonIFCObject);
		std::string styledJsonIFCObject = JsonIFCObject.toStyledString();
		//std::ofstream ofs;
		//ofs.open(fileName.c_str(), std::ios_base::app | std::ios_base::out);
		ofs << styledJsonIFCObject;
		if (ifcObject->next != NULL)
			ofs << "\," << std::endl;

		ifcObject = ifcObject->next;
	}

	ofs << "\]" << std::endl;
	ofs.close();

	/*
	//std::string json_file = writer.write(root);
	std::string styledJsonFile = root.toStyledString();
	std::ofstream ofs;
	ofs.open(fileName.c_str());
	ofs << styledJsonFile;
	*/

	CleanupIfcFile();

	//std::string out = root.toStyledString();
	//std::cout << out << std::endl;
}
