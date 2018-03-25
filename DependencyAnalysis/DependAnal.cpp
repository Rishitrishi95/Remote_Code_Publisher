/////////////////////////////////////////////////////////////////////
// DependAnal.cpp - Store the type information needed for          //
//                  dependency between the files                   //
// ver 1.0                                                         //
//-----------------------------------------------------------------//
// Rishit Reddy Muthyala (c) copyright 2015                        //
// All rights granted provided this copyright notice is retained   //
//-----------------------------------------------------------------//
// Language:    C++, Visual Studio 2015                            //
// Application: TYpe Based Dependency Analysis                     //
//              CSE687 - Object Oriented Design                    //
// Author:     Rishit Reddy Muthyala, Syracuse University          //
//             rmuthyal@syr.edu                                    //
// Maintainance History: Created on March 4th,2017                 //
/////////////////////////////////////////////////////////////////////

// This cpp file implements all the declarations in header files 
// of DependAnal.h

#include "DependAnal.h"
#include "../FileSystem/FileSystem.h"
#include "../Parser/Parser.h"
#include <vector>
#include <sstream>

using namespace Code_Analysis;

//----This method displays the Graph
void DependAnal::displayGraph()
{
	dependDisplay_.show(dependendencyGraph);
}


//----This method adds the dependencies to the graph
void DependAnal::addDependencyToGraph(const std::string &fileName1_, const std::string &fileName2_)
{
	static int i = 0;
	int index1 = -1;
	int index2 = -1;
	for (size_t j = 0; j < dependendencyGraph.size(); j++)
	{
		Vertex<std::string, std::string>& v1 = dependendencyGraph[j];
		if (v1.value() == fileName1_)
		{
			index1 = (int)j;
		}
		if (v1.value() == fileName2_)
		{
			index2 = (int)j;
		}
	}

	std::stringstream ss;
	ss.str("");
	ss << "Edge" << i;
	if ((index1 != -1) && (index2 != -1))
	{
		if (dependendencyGraph.addEdge(ss.str(), dependendencyGraph[index1], dependendencyGraph[index2]) == true)
		{
			i++;
		}
	}
}




bool DependAnal::Attach(const std::string& name)
{
	if (pToker == 0)
		return false;
	if (pIn != nullptr)
	{
		pIn->close();
		delete pIn;
	}
	pIn = new std::ifstream(name);
	if (!pIn->good())
		return false;

	char a, b, c;
	a = pIn->get();
	b = pIn->get();
	c = pIn->get();
	if (a != (char)0xEF || b != (char)0xBB || c != (char)0xBF)
		pIn->seekg(0);

	if (!pIn->good())
	{
		return true;
	}
	return pToker->attach(pIn);
}

//------This method initializes all the vertices
void DependAnal::InitializeVertices()
{
	static int i = 0;
	for (auto item : fileMap_)
	{
		for (auto file : item.second)
		{
			Vertex<std::string, std::string> vert(file, i);
			if (dependendencyGraph.addVertex(vert) == true)
			{
				i++;
			}
		}
	}

}


//-----This method fetched all the source files
void DependAnal::fetchSourcseFiles()
{
	AnalFileMgr fm(path_, fileMap_);
	for (auto patt : patterns_)
		fm.addPattern(patt);
	fm.search();
}


//------This method processes the code and get the arguments

bool DependAnal::ProcessCommandLine(int argc, char* argv[])
{
	if (argc < 2)
	{
		return false;
	}
	try {
		path_ = FileSystem::Path::getFullFileSpec(argv[1]);
		if (!FileSystem::Directory::exists(path_))
		{
			std::cout << "\n\n  path \"" << path_ << "\" does not exist\n\n";
			return false;
		}

		for (int i = 2; i < argc; ++i)
		{
			if (argv[i][0] == '/')
				options_.push_back(argv[i][1]);
			else
				patterns_.push_back(argv[i]);
		}
		if (patterns_.size() == 0)
		{
			return false;
		}
	}
	catch (std::exception& ex)
	{
		std::cout << "\n\n  Error in passing command line arguments";
		std::cout << "\n  " << ex.what() << "\n\n";
		return false;
	}

	return true;
}

//-----This method processes the source code and performs the depndency analyiis on the Type table
void DependAnal::processSourceCode()
{InitializeVertices();
	for (auto item : fileMap_)
	{
		for (auto file : item.second)
		{
			Attach(file);
			std::string fileName_;
			if (file.find_last_of("\\") < file.length())
				fileName_ = file.substr(file.find_last_of("\\") + 1);
			while (pParser->next())
			{
				std::vector<std::string>::iterator it = pSemi->begin();
				for (it; it != pSemi->end(); it++)
				{
					if (typeTable_.getTypeTable().find(*it) != typeTable_.getTypeTable().end())
					{
						if (file != typeTable_.getTypeTable().find(*it)->second.filename)
						{
							if (dependendencyMap[file].insert(typeTable_.getTypeTable().find(*it)->second.filename).second 
								!= false)
							{
								std::string fileName2_ = typeTable_.getTypeTable().find(*it)->second.filename;
								addDependencyToGraph(file, fileName2_);
								std::string str1 = file.substr(file.find_last_of("\\") + 1);
								std::string str2 = fileName2_.substr(fileName2_.find_last_of("\\") + 1);
								if ((str1.find_last_of(".cpp") < str1.length()) 
									&& (str2.find_last_of(".h") < str2.length()))
								{
									if (str1.substr(0, str1.find_last_of(".cpp") - 4).compare
									(str2.substr(0, str2.find_last_of(".h") - 2)) == 0)
									{dependendencyMap[fileName2_].insert(file);
										addDependencyToGraph(fileName2_, file);}
								}
								
							}
						}
					}
				}
			}
		}
	}
	Utilities::StringHelper::Title("Dependency Analysis is done.");
}

#ifdef TEST_DEPENDANAL
int main()
{
	DependAnal dan;
	dan.ProcessCommandLine(argc, argv);
	dan.fetchSourcseFiles();
	dan.processSourceCode();
	return 0;
}
#endif