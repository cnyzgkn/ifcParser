#include "IFCJsonInteract.h"
#include "IFCEngineInteract.h"
#include <iostream>
#include <fstream>
#include <string>
#include <locale>

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
			if (ifcObject->indicesForFaces[i] > 1000000 && ifcObject->indicesForFaces[i] < -1000000)
				std::cout << "ID == " << ifcObject->globalId << " type == " << ifcObject->ifcType << std::endl;
		}
		/*
		//vertexOffsetForFaces
		JsonIFCObject["vertexOffsetForFaces"] = ifcObject->vertexOffsetForFaces;
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
