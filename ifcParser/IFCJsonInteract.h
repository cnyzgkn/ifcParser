#ifndef IFCJSONINTERACT_H
#define IFCJSONINTERACT_H

#include "jsoncpp/json/json.h"
#include "generic.h"

extern	STRUCT__IFC__OBJECT	* ifcObjectsLinkedList;

void DumpIfcObjects2Json(const std::string& fileName);

#endif
