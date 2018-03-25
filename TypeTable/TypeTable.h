#ifndef TYPETABLE_H
#define TYPETABLE_H
/////////////////////////////////////////////////////////////////////
// TypeTable.h - Store the type information needed for dependency  //
//               Analysis between the files                        //
// ver 1.2                                                         //
//-----------------------------------------------------------------//
// Rishit Reddy Muthyala(c) copyright 2015                         //
// All rights granted provided this copyright notice is retained   //
//-----------------------------------------------------------------//
// Language:    C++, Visual Studio 2015                            //
// Application: TYpe Based Dependency Analysis                     //
//              CSE687 - Object Oriented Design                    //
// Author:      Rishit Reddy Muthyala, Syracuse University          //
//              rmuthyal@syr.edu                                   //
/////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>


namespace Code_Analysis
{
	using namespace std;
	struct DataStruct {
		string name;
		string type;
		string filename;
	};

	using TypeInfo = std::string;
	using FileInfo = std::string;
	using TypeMap = std::unordered_map<TypeInfo, DataStruct>;
	using TyeTableIterator = std::pair<TypeInfo, FileInfo>;
	class TypeTable
	{
	public:
		//----< All the method below perform various simmple operations on Typetable >--------------
		TypeTable() {}
		~TypeTable() { typeTable_.clear(); }
		void TableDisplay();
		void FeddTypeTable(TypeInfo ti, DataStruct s1);
		TypeMap& getTypeTable() { return typeTable_; }
	private:
		TypeMap typeTable_;
	};
}
#endif
