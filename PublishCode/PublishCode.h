#pragma once
#ifndef PUBLISHCODE_H
#define PUBLISHCODE_H
#pragma once
/////////////////////////////////////////////////////////////////////
// HTMLGenerater.h - Generates the html files for all files matching
//					the patterns which is given as input		//                                //
// ver 1.0                                                        //
//-----------------------------------------------------------------//
// Rishit Reddy Muthyala (c) copyright 2017                        //
// All rights granted provided this copyright notice is retained   //
//-----------------------------------------------------------------//
// Language:    C++, Visual Studio 2015                            //
// Application: TYpe Based Dependency Analysis                     //
//              CSE687 - Object Oriented Design                    //
// Author:      Rishit Reddy Muthyala, Syracuse University          //
//              rmuthyal@syr.edu                                   //
/////////////////////////////////////////////////////////////////////

//* Operations:
//* This package is used to generate html files for all the matching file patterns found in the Project folder
//* Also, A file called DependencyFile.html is created which lists all the files present in the Repository

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <iostream>

namespace Code_Analysis
{
	class HTMLGenerator
	{
	public:
		using Path = std::string;
		using Pattern = std::string;
		using Patterns = std::vector<Pattern>;
		using File = std::string;
		using Files = std::vector<File>;
		using Ext = std::string;
		using Options = std::vector<char>;
		using FileMap = std::unordered_map<Pattern, Files>;
		using DependMap = std::unordered_map<std::string, std::set<std::string>>;

		HTMLGenerator(DependMap& depndencymap_);

		bool ProcessCommandLine(int argc, char* argv[]);
		//----< Gets al the source files which are required for processing >--------------
		void getSourceFiles();
		//----< Process the sources code and builds the dependency graph >--------------
		void processSourceCode();

		void copyFiletoHtm(std::string file);
		void copyFiletoHtmToIIS(std::string file);
		void displayDependencies();
		void flushFilesInRepository(std::vector<std::string> filesList);
		void Demo2();
		void reqDemo();
		void CreateIIXIndexFile();
		void copyContentToHtmlFile(std::string oldfile, std::string newfilename);
	private:
		Path path_;
		Patterns patterns_;
		Options options_;
		FileMap fileMap_;
		DependMap &dependMap_;
		Path storePath_;
		Path IISPathDepositPath_;
	};
}
#endif /* PUBLISHCODE_H */
