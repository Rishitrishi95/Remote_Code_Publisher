/////////////////////////////////////////////////////////////////////
//  HTMLGenerater.cpp -- Implements the methods in HTMLGenerater.h  //
//  ver 1.4                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Dell XPS 8900, Windows 10                       //
//  Application:   Used to perform Dependency Analysis				//
//  Author:        Rishit Redddy Muthyala,
//			    	Syracuse University								//
/////////////////////////////////////////////////////////////////////

#include <vector>
#include <sstream>
#include "PublishCode.h"
#include "../FileSystem/FileSystem.h"
#include "../Analyzer/Executive.h"

using namespace Code_Analysis;
using namespace std;


//---This method flushes all the files in the Repository
void HTMLGenerator::flushFilesInRepository(std::vector<std::string> filesList)
{
	try
	{
		for (int i = 0; i < filesList.size(); i++)
		{
			string removePath = storePath_ + "/" + filesList[i];
			if (FileSystem::File::exists(removePath)) {
				FileSystem::File::remove(removePath);
	
			}
		}
	}
	catch (exception e) {
		cout << "Exception caught while trying to delete the file\n";
	}
}



HTMLGenerator::HTMLGenerator(DependMap& depndencymap_) : dependMap_(depndencymap_)
{}

//This is a method to cut the method less than 50 lines
void HTMLGenerator::reqDemo()
{

	
	std::string dependencyhtmfile = storePath_ + "/" + "index.htm";


	if (FileSystem::File::exists(dependencyhtmfile))
	{
		FileSystem::File::remove(dependencyhtmfile);
	}
	
	ofstream ofs(dependencyhtmfile);
	for (auto item : fileMap_)
	{
		for (auto file : item.second)
		{
			string onlyFileName1 = file.substr(file.find_last_of("\\") + 1);
			for (std::string items : dependMap_[file])
			{
				string onlyFileName2 = items.substr(items.find_last_of("\\") + 1);
				string addhref1 = "<a href=" + onlyFileName1 + ".htm>" + onlyFileName1 + "</a>";
				string addhref2 = "<a href=" + onlyFileName2 + ".htm>" + onlyFileName2 + "</a>";
				ofs << addhref1 << "  --------  " << addhref2;
				ofs << "<br/><br/>";
			}
		}
	}
	ofs.close();
	
}


//-----This method creates the Index file in IIS folder
void HTMLGenerator::CreateIIXIndexFile()
{


	std::string cpath = IISPathDepositPath_;
	
	std::string dependencyhtmfile = cpath + "/" + "index.htm";
	

	if (FileSystem::File::exists(dependencyhtmfile))
	{
		FileSystem::File::remove(dependencyhtmfile);
	}
	
	ofstream ofs(dependencyhtmfile);
	for (auto item : fileMap_)
	{
		for (auto file : item.second)
		{
			string onlyFileName1 = file.substr(file.find_last_of("\\") + 1);
			for (std::string items : dependMap_[file])
			{
				string onlyFileName2 = items.substr(items.find_last_of("\\") + 1);
				string addhref1 = "<a href=" + onlyFileName1 + ".htm>" + onlyFileName1 + "</a>";
				string addhref2 = "<a href=" + onlyFileName2 + ".htm>" + onlyFileName2 + "</a>";
				ofs << addhref1 << "  --------  " << addhref2;
				ofs << "<br/><br/>";
			}
		}
	}
	ofs.close();

}

void HTMLGenerator::Demo2()
{
	
	ifstream ifs("../../Addhtmlheader.txt");
	std::string line;
	char ch;
	stringstream ss;
	ss << "\n";
	while (ifs.get(ch))
		ss << ch;
	cout << ss.str() << "\n";
	ss.str("");
	ifs.close();
	
	ifstream ifs2("../../myjsscript.js");
	ss << "\n";
	while (ifs2.get(ch))
		ss << ch;
	cout << ss.str() << "\n";
	ss.str("");
	ifs2.close();
	

	ifstream ifs3("../../mystylesheet.css");
	ss << "\n";
	while (ifs3.get(ch))
		ss << ch;
	cout << ss.str() << "\n";
	ifs3.close();
	
}

//----This method processses the command line arguments
bool HTMLGenerator::ProcessCommandLine(int argc, char* argv[])
{
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
		string convPathtoString(argv[6]);
		string convPathtoStringFotIIS(argv[7]);
		storePath_ = convPathtoString;
		IISPathDepositPath_ = convPathtoStringFotIIS;
		cout << IISPathDepositPath_ << endl;
		if (!FileSystem::Directory::exists(convPathtoString))
		{
			FileSystem::Directory::create(convPathtoString);
			cout << "\nCreated the destination directory " << convPathtoString << " to store html files";
		}
		else
		{
			flushFilesInRepository(FileSystem::Directory::getFiles(convPathtoString, "*.*"));
			FileSystem::Directory::remove(convPathtoString);
			cout << "\nRemoved the existing directory " << convPathtoString << "\n";
			FileSystem::Directory::create(convPathtoString);
		}
	}
	catch (std::exception& ex)
	{
		std::cout << "\n  " << ex.what() << "\n\n";
		return false;
	}
	return true;
}

void HTMLGenerator::getSourceFiles()
{
	AnalFileMgr fm(path_, fileMap_);
	for (auto patt : patterns_)
		fm.addPattern(patt);
	fm.search();
}


//-------This method creates the header part of each html file
void HTMLGenerator::copyContentToHtmlFile(std::string oldfile, std::string newfilename)
{
	static int counter = 0;
	ofstream ofs(newfilename, ios_base::out | ios_base::app);
	char ch;
	fstream fin(oldfile, fstream::in);
	ofs << "\n";
	while (fin >> noskipws >> ch)
	{
		if (ch == '<')
		{
			ofs << "&lt;";
			continue;
		}
		if (ch == '>')
		{
			ofs << "&gt;";
			continue;
		}
		ofs << ch;
		if (ch == '{')
		{
			ofs << "\n";
			string divid = "simplediv" + std::to_string(counter);
			string buttonid = "myButton" + std::to_string(counter);
			counter++;
			ofs << "<input onclick=\"expand_collapse(" << divid << ".id, this.id)\" type=\"button\" value=\" - \" id=\"" << buttonid << "\" />";
			ofs << "<div id=\"" << divid << "\" style=\"display:block; \">";
		}
		if (ch == '}')
		{
			ofs << "</div>";
		}
	}
	ofs << "</pre>\n</body>\n</html>\n";
	ofs.close();
}

void HTMLGenerator::copyFiletoHtm(std::string file)
{
	if (!FileSystem::File::exists(file))
	{
		cout << "File: " << file << "to be copied to HTML page is not present" << endl;
	}
	std::string fileName_ = file.substr(file.find_last_of("\\") + 1);
	std::string newFilename_ = storePath_;

	newFilename_ += "/";
	newFilename_ += fileName_ + ".htm";
	ofstream ofs(newFilename_);


	ifstream ifs("../../boilerplate.txt");
	if (!ifs.is_open())
	{
		cout << "Couldnot open boilerplate.txt file\n";
		return;
	}
	std::string line;
	char ch;
	while (ifs.get(ch))
	{
		ofs << ch;
	
	}

	ifs.close();
	ofs << "<hr />";
	ofs << "<div class=\"indent\" >\n" << "<h4>Dependencies:</h4>\n";
	for (std::string items : dependMap_[file])
	{
		string onlyFileName = items.substr(items.find_last_of("\\") + 1);
		string addhref = "<a href=" + onlyFileName + ".htm>" + onlyFileName + "</a>";
		ofs << addhref << "\n";
	}
	ofs << "</div>\n  <hr />\n <pre>";
	ofs.close();
	copyContentToHtmlFile(file, newFilename_);
}


void HTMLGenerator::copyFiletoHtmToIIS(std::string file)
{
	if (!FileSystem::File::exists(file))
	{
		cout << "File: " << file << "to be copied to HTML page is not present" << endl;
	}
	std::string fileName_ = file.substr(file.find_last_of("\\") + 1);
	std::string newFilename_ = IISPathDepositPath_;

	newFilename_ += "/";
	newFilename_ += fileName_ + ".htm";
	ofstream ofs(newFilename_);
	cout << "Created a new HTML file: " << newFilename_ << "\n";

	ifstream ifs("../../boilerplate.txt");
	if (!ifs.is_open())
	{
		cout << "Couldnot open Addhtmlheader.txt file\n";
		return;
	}
	std::string line;
	char ch;
	while (ifs.get(ch))
	{
		ofs << ch;
		
	}

	ifs.close();
	ofs << "<hr />";
	ofs << "<div class=\"indent\" >\n" << "<h4>Dependencies:</h4>\n";
	for (std::string items : dependMap_[file])
	{
		string onlyFileName = items.substr(items.find_last_of("\\") + 1);
		string addhref = "<a href=" + onlyFileName + ".htm>" + onlyFileName + "</a>";
		ofs << addhref << "\n";
	}
	ofs << "</div>\n  <hr />\n <pre>";
	ofs.close();
	copyContentToHtmlFile(file, newFilename_);

}



void HTMLGenerator::processSourceCode()
{
	reqDemo();
	CreateIIXIndexFile();
	
	cout << "\n";
	for (auto item : fileMap_)
	{
		for (auto file : item.second)
		{
			
			copyFiletoHtm(file);
			copyFiletoHtmToIIS(file);
			//std::string newHtmFile = file + ".htm";
		}
	}

	Utils::Title("All the html files are generated and stored in the folder specifies by the user and also in the IIS physical directroy");

}


//----This method dsiplays the Dependency of the Table
void HTMLGenerator::displayDependencies()
{
	unordered_map<std::string, std::set<std::string>>::iterator it = dependMap_.begin();
	cout << "\nSize of the Dependency Table is " << dependMap_.size() << "\n";
	for (it; it != dependMap_.end(); it++)
	{
		cout << "\n The file is : " << it->first << " is \n";
		cout << "Its dependencies are: " << endl;
		for (std::string str : it->second)
		{
			cout << std::setw(8) << " " << str << "\n";
		}
	}
}


#ifdef TEST_PUBLISHCODE
int main()
{

}
#endif