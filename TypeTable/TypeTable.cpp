#pragma once
/////////////////////////////////////////////////////////////////////////
// TypeTable.cpp - Provides a container class that stores type         //
//                 information needed for dependency analysis.         //
// ver 1.0                                                             //
// Language:    C++, Visual Studio 2015                                //
// Application: Support for DepAnal, CSE687 - Object Oriented Design   //
// Author:      Rishit reddy Muthyala, Syracuse University,            //
/////////////////////////////////////////////////////////////////////////

#include "TypeTable.h"
#include "../Utilities/Utilities.h"
#include <iomanip>

using namespace Code_Analysis;
using namespace Utilities;

//---This method is used to display the TimeTable 
void TypeTable::TableDisplay()
{
	Utilities::StringHelper::Title("Type Table Info");

	std::cout << "\n";

	string name1;
	DataStruct s1;
	cout << "------------------------------------------------------------------------------------" << endl;
	cout << setw(40) << setfill(' ') << "Name" << setw(40) << setfill(' ') << "DataType" << setw(40) << setfill(' ') << "FileName" << endl;
	cout << "------------------------------------------------------------------------------------" << endl;
	for (std::unordered_map<string, DataStruct>::iterator it = typeTable_.begin(); it != typeTable_.end(); ++it) {
		name1 = it->first;
		s1 = it->second;
		cout << setw(40) << setfill(' ') << name1 << setw(40) << setfill(' ') << s1.type << setw(40) << setfill(' ') <<
			s1.filename << endl;
	}
}
//----This method is used to feed the Type table with the Struct
void TypeTable::FeddTypeTable(TypeInfo ti, DataStruct fi)
{
	if (typeTable_.find(ti) == typeTable_.end())
	{
		typeTable_[ti] = fi;
	}
}




#ifdef TEST_TYPETABLE
int main()
{
	return 0;
}
#endif