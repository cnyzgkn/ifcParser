
// ifcviewer.cpp : Defines the class behaviors for the application.
//

#include "IFCEngineInteract.h"
#include "IFCGeomInteract.h"
#include "IFCJsonInteract.h"
#include "ifcengine/include/engdef.h"
#include <string>
#include <iostream>

#ifdef _WINDOWS
#include <direct.h> 
#elif defined(_LINUX)
#include <unistd.h>
#endif

extern char *ifcFileName, *ifcSchemaName_IFC2x3, *ifcSchemaName_IFC4, *xmlSettings_IFC2x3, *xmlSettings_IFC4;

int main(int argc, char** argv) 
{
	if ( argc != 2 ) {
#ifdef _WINDOWS
		std::cout << "usage: IfcParser C::\\filename.ifc" << std::endl;
#elif defined(_LINUX)
		std::cout << "usage: IfcParser /home/etc/filename.ifc" << std::endl;
#endif
		return 1;
	}

	char *buffer = NULL;
	if ((buffer = getcwd(NULL, 0)) == NULL)
	{
		std::cout << "get current directory error" << std::endl;
		return 0;
	}

#ifdef _WINDOWS
	const std::string expData("\\expData");
	const std::string IFC2X3exp("\\IFC2X3_TC1.exp");
	const std::string IFC4exp("\\IFC4.exp");
	const std::string IFC2X3Setting("\\IFC2X3-Settings.xml");
	const std::string IFC4Setting("\\IFC4-Settings.xml");
#elif defined(_LINUX)
	const std::string expData("/expData");
	const std::string IFC2X3exp("/IFC2X3_TC1.exp");
	const std::string IFC4exp("/IFC4.exp");
	const std::string IFC2X3Setting("/IFC2X3-Settings.xml");
	const std::string IFC4Setting("/IFC4-Settings.xml");
#endif
	
	const std::string strCurrentFolderPath(std::string(buffer) + expData);
	free(buffer);

	const std::string strifcSchemaName_IFC2x3 = strCurrentFolderPath + IFC2X3exp;
	ifcSchemaName_IFC2x3 = new char[strifcSchemaName_IFC2x3.size() + 1];
	strncpy(ifcSchemaName_IFC2x3, strifcSchemaName_IFC2x3.c_str(), strifcSchemaName_IFC2x3.size() + 1);
	std::cout << "IFC2x3 schema " << std::string(ifcSchemaName_IFC2x3) << std::endl;

	const std::string strifcSchemaName_IFC4 = strCurrentFolderPath + IFC4exp;
	ifcSchemaName_IFC4 = new char[strifcSchemaName_IFC4.size() + 1];
	strncpy(ifcSchemaName_IFC4, strifcSchemaName_IFC4.c_str(), strifcSchemaName_IFC4.size() + 1);
	std::cout << "IFC4 schema " << std::string(ifcSchemaName_IFC4) << std::endl;

	const std::string strxmlSettings_IFC2x3 = strCurrentFolderPath + IFC2X3Setting;
	xmlSettings_IFC2x3 = new char[strxmlSettings_IFC2x3.size() + 1];
	strncpy(xmlSettings_IFC2x3, strxmlSettings_IFC2x3.c_str(), strxmlSettings_IFC2x3.size() + 1);
	std::cout << "IFC2x3 xml " << std::string(xmlSettings_IFC2x3) << std::endl;

	const std::string strxmlSettings_IFC4 = strCurrentFolderPath + IFC4Setting;
	xmlSettings_IFC4 = new char[strxmlSettings_IFC4.size() + 1];
	strncpy(xmlSettings_IFC4, strxmlSettings_IFC4.c_str(), strxmlSettings_IFC4.size() + 1);
	std::cout << "IFC4 xml " << std::string(xmlSettings_IFC4) << std::endl;

	const std::string strFileFullPath(argv[1]);
	ifcFileName = new char[strFileFullPath.size() + 1];
	strncpy(ifcFileName, strFileFullPath.c_str(), strFileFullPath .size() + 1);
	std::cout << "Parsing file " << argv[1] << std::endl;

	ParseIfcFile();

	FillMaterials2IfcObjects();

	delete[] ifcSchemaName_IFC2x3;
	delete[] ifcSchemaName_IFC4;
	delete[] xmlSettings_IFC2x3;
	delete[] xmlSettings_IFC4;
	delete[] ifcFileName;

	//DumpIfcObjects2Json("result.json");
	DumpIfcObjects2SeperateJson("result");

	return 1;
}




