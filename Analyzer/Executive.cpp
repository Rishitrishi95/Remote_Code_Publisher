/////////////////////////////////////////////////////////////////////
// Executive.cpp - Demonstrate Project2 Requirements               //
// ver 1.4                                                         //
//-----------------------------------------------------------------//
// Rishit Reddy Muthyala(c) copyright 2016                          //
// All rights granted provided this copyright notice is retained   //
//-----------------------------------------------------------------//
// Language:    C++, Visual Studio 2015                            //
// Platform:    Dell XPS 8900, Windows 10                          //
// Application: Project #2, CSE687 - Object Oriented Design, S2015 //
// Author:      Rishit Reddy Muthyala, Syracuse University,		    //
//           rmuthyal@syr.edu                                       //
// Maintainance History: Created on 22th May 2017                   //
// Ver 1.0: Updated on 2 May,2017									//
/////////////////////////////////////////////////////////////////////

#include "Executive.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <exception>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <shellapi.h>
#include "../Parser/Parser.h"
#include "../FileSystem/FileSystem.h"
#include "../FileMgr/FileMgr.h"
#include "../Parser/ActionsAndRules.h"
#include "../Parser/ConfigureParser.h"
#include "../AbstractSyntaxTree/AbstrSynTree.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include "../DepthAnalysis/TypeAnalysis.h"
#include "../DependencyAnalysis/DependAnal.h"
#include "../Graph/Graph.h"
#include "../PublishCode/PublishCode.h"
using Rslt = Logging::StaticLogger<0>;  // use for application results
using Demo = Logging::StaticLogger<1>;  // use for demonstrations of processing
using Dbug = Logging::StaticLogger<2>;  // use for debug output
using namespace Utilities;
using namespace Async;
using namespace Code_Analysis;
using namespace std;
using Path = std::string;
using File = std::string;
using Files = std::vector<File>;
using Pattern = std::string;
using Ext = std::string;
using FileMap = std::unordered_map<Pattern, Files>;
using namespace Code_Analysis;


int CodeAnalyze(int argc, char* argv[]) {
	CodeAnalysisExecutive codeExecutive;
	try {
		bool succeeded = codeExecutive.ProcessCommandLine(argc, argv);
		codeExecutive.setDisplayModes();
		//	codeExecutive.startLogger(std::cout);
		std::ostringstream tOut("CodeAnalysis for Repository");
		Utils::sTitle(tOut.str(), 3, 92, tOut, '=');
		cout << tOut.str();
		cout << "\n     " + codeExecutive.getAnalysisPath();
		cout << "\n     " + codeExecutive.systemTime();
		//Rslt::flush();
		codeExecutive.showCommandLineArguments(argc, argv);
		cout << "\n";
		codeExecutive.getSourceFiles();
		codeExecutive.processSourceCode(true);
		std::ostringstream out;
		cout << "\n  " << std::setw(10) << "searched" << std::setw(6) << codeExecutive.numDirs() << " dirs";
		cout << "\n  " << std::setw(10) << "processed" << std::setw(6) << codeExecutive.numFiles() << " files";
		//Rslt::write(out.str());
		//	codeExecutive.stopLogger();
		putline();
		TypeAnalysis ta;
		ta.doTypeAnalysis();
		ta.displayTypeTable();
		DependAnal dan;
		dan.ProcessCommandLine(argc, argv);
		dan.fetchSourcseFiles();
		dan.processSourceCode();
		CodeAnalysisExecutive::dependencyMapForUse = dan.returnDependenyTable();
		HTMLGenerator cp(dan.returnDependenyTable());
		cp.ProcessCommandLine(argc, argv);
		cp.getSourceFiles();
		cp.displayDependencies();
		cp.Demo2();
		cp.processSourceCode();
		std::string pathForIndex = argv[6];
		pathForIndex += "/Index.htm";
		pathForIndex = FileSystem::Path::getFullFileSpec(pathForIndex);
		std::wstring sPath(pathForIndex.begin(), pathForIndex.end());
		ShellExecute(NULL, L"open", sPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		std::string pathForIndexInIIS = argv[8];
		pathForIndexInIIS += "/Index.htm";
		std::wstring sPath1(pathForIndexInIIS.begin(), pathForIndexInIIS.end());
		ShellExecute(NULL, L"open", sPath1.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	catch (std::exception& except)
	{
		std::cout << "\n\n  caught exception in Executive::main: " + std::string(except.what()) + "\n\n";
		return 1;
	}
	return 0;
}
//----<This method is the final step before sending message to the client >----------

bool finalSendMessageToClient(HttpMessage &msg)
{
	try
	{
		SocketSystem ssytem;
		SocketConnecter sc2;
		while (!sc2.connect("localhost", 8081))
		{
			cout << "\n Server is currently waiting for connection" << endl;
			::Sleep(100);
		}
		std::string sendString = msg.toString();
		sc2.send(sendString.size(), (Socket::byte*)sendString.c_str());
		cout << "\n Server has sent get reply to the client." << endl;
	} catch (std::exception& exc)
	{
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << "\n  Exeception caught: " << exMsg << endl;
	}
	return true;

}

//----<This method processes get File Names from server message requested by the client >----------
void getMessage_getFileNames(HttpMessage &msg, int argc, char* argv[]) {
	string sendResult = "";
	HttpMessage SM;
	std::string SrvrEndPoint = "localhost::8080";
	std::string ClientEndPoint = "localhost::8081";
	SM.addAttribute(HttpMessage::attribute("GET", "Message"));
	SM.addAttribute(HttpMessage::Attribute("mode", "oneWay"));
	SM.addAttribute(HttpMessage::parseAttribute("toAddr:" + ClientEndPoint));
	SM.addAttribute(HttpMessage::parseAttribute("fromAddr:" + SrvrEndPoint));
	cout << "\nServer needs to process the GetFiles message: \n";
	string CategoryPath = argv[1];
	CategoryPath += "/" + msg.findValue("FileCategory");
	vector<std::string> getAllFilesList = FileSystem::Directory::getFiles(CategoryPath, "*.*");
	for (int i = 0; i < getAllFilesList.size(); i++)
	{
		cout << "File found " << getAllFilesList[i] << " \n";
		sendResult += getAllFilesList[i];
		if (i != getAllFilesList.size() - 1)
		{
			sendResult += ",";
		}
	}
	SM.addAttribute(HttpMessage::Attribute("messageType", "getFileNamesReply"));
	if (sendResult.size() > 0)
	{
		SM.addAttribute(HttpMessage::attribute("content-length", Converter<size_t>::toString(sendResult.size())));
		SM.addBody(sendResult);
	}
	getAllFilesList.clear();
	cout << "\nMessage to be sent is " + SM.toString();
	finalSendMessageToClient(SM);
}




//----<This method is runs when the client wants to analyze the code remotely >----------



//----<This method sends a requested file to the client >----------
bool sendingFileToClient(string FileName,string path, int argc, char* argv[]) {
	try {
		HttpMessage SMFromRepo;
		std::string ServerEndPoint = "localhost::8080";
		std::string ClientEndPoint = "localhost::8081";
		SMFromRepo.addAttribute(HttpMessage::attribute("GET", "Message"));
		SMFromRepo.addAttribute(HttpMessage::Attribute("mode", "oneWay"));
		SMFromRepo.addAttribute(HttpMessage::parseAttribute("toAddr:" + ClientEndPoint));
		SMFromRepo.addAttribute(HttpMessage::parseAttribute("fromAddr:" + ServerEndPoint));
		string FileHtmPath = path;
		FileHtmPath += '/' + FileName;
		FileSystem::File file(FileHtmPath);
		file.open(FileSystem::File::in, FileSystem::File::binary);
		if (!file.isGood())
		{	cout<<"\n\n  can't open file " + FileHtmPath;
			return false;}
		FileSystem::FileInfo fi(FileHtmPath);
		size_t fileSize = fi.size();
		std::string sizeString = Converter<size_t>::toString(fileSize);
		SocketSystem ss;SocketConnecter si;
		while (!si.connect("localhost", 8081))
		{	cout<<"\n Server waiting for connection";
			::Sleep(100);}
		SMFromRepo.addAttribute(HttpMessage::Attribute("messageType", "HtmFileContentReplyFromServer"));
		SMFromRepo.addAttribute(HttpMessage::Attribute("fileName", FileName));
		SMFromRepo.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		std::string sendString = SMFromRepo.toString();
		si.send(sendString.size(), (Socket::byte*)sendString.c_str());
		const size_t BlockSize = 2048;
		Socket::byte buffer[BlockSize];
		while (true)
		{	FileSystem::Block blk = file.getBlock(BlockSize);
			if (blk.size() == 0)
				break;
			for (size_t i = 0; i < blk.size(); ++i)
				buffer[i] = blk[i];
			si.send(blk.size(), buffer);
			if (!file.isGood())
				break;}
		file.close();
		cout<<"\n\n File : " + FileHtmPath + " content from the file is sent to the client successfully";
	}
	catch (std::exception& exc)
	{cout<<"\n  Exeception caught: ";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout<<exMsg;}
	return true;}

//----<This method sends all the dependent files to the Client along with the requestted files >----------
bool LazyLoadingExecutive(int argc, char* argv[],string FileName) {
	Utils::Title("Requirement 10 : Lazy loading is started. On downloading a file all of its depenedent files will be loaded");
	try {
		if (CodeAnalysisExecutive::dependencyMapForUse.size() <= 1) {
			CodeAnalyze(argc, argv);
		}
		unordered_map<std::string, std::set<std::string>>::iterator it = CodeAnalysisExecutive::dependencyMapForUse.begin();
		for (it; it != CodeAnalysisExecutive::dependencyMapForUse.end(); it++) {
			string fileStringFromItr = it->first;
			std::string fileName_ = fileStringFromItr.substr(fileStringFromItr.find_last_of("\\") + 1);
			if (fileName_.compare(FileName) == 0) {
				set<string> valueOfMap = it->second;
				for (std::string str : it->second)
				{

					std::string dependendencyFile = str.substr(str.find_last_of("\\") + 1);
					string FileHtmPath = argv[6];
					sendingFileToClient(FileHtmPath, dependendencyFile + ".htm", argc, argv);

				}
			}
		}
	}
	catch (std::exception& exc)
	{
		cout<<"\n  Exeception caught: ";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout<<exMsg;
	}
	return true;
}

//----<This method processes download Files from server message requested by the client >----------
void getMessage_downloadFileFromServer(HttpMessage &msg, int argc, char* argv[]) {

	string sendResult = "";
	HttpMessage sendMessage;
	std::string ServerEndPoint = "localhost::8080";
	std::string ClientEndPoint = "localhost::8081";
	sendMessage.addAttribute(HttpMessage::attribute("GET", "Message"));
	sendMessage.addAttribute(HttpMessage::Attribute("mode", "oneWay"));
	sendMessage.addAttribute(HttpMessage::parseAttribute("toAddr:" + ClientEndPoint));
	sendMessage.addAttribute(HttpMessage::parseAttribute("fromAddr:" + ServerEndPoint));
	size_t contentSize;
	try {
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
			contentSize = Converter<size_t>::toValue(sizeString);
		if (contentSize > 0) {
			Socket::byte* buffer = new Socket::byte[contentSize + 1];
			msg.getBody(buffer, contentSize);
			buffer[contentSize] = '\0';
			string getMsgBody = buffer;
			delete[] buffer;
			if (getMsgBody.find('#') != std::string::npos) {
				string Split = getMsgBody.substr(getMsgBody.find('#') + 1);
				if (Split.find('#') != std::string::npos) {
					string CategoryName = Split.substr(0, Split.find('#'));
					string FileName = Split.substr(Split.find('#') + 1);
					string FileHtmPath = argv[6];
					sendingFileToClient(FileHtmPath, FileName + ".htm", argc, argv);
					LazyLoadingExecutive( argc, argv, FileName);
				}
			}
		}
	}
	catch (std::exception& exc)
	{
		cout<<"\n  Exeception caught in the method deleteSingleFileOnServer: \n";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout<<exMsg;
	}
}
//----<This method handles delete file from client requests coming to the client >----------

void FileDeletionFromServer(HttpMessage &msg, int argc, char* argv[]) {
	cout << "\nServer needs to process the delete message request from the client: \n";
	size_t contentSize = 0;
	HttpMessage replyMessage;
	std::string SrvrEndPoint = "localhost::8080";
	std::string ClientEndPoint = "localhost::8081";
	replyMessage.addAttribute(HttpMessage::attribute("GET", "Message"));
	replyMessage.addAttribute(HttpMessage::Attribute("mode", "oneWay"));
	replyMessage.addAttribute(HttpMessage::parseAttribute("toAddr:" + ClientEndPoint));
	replyMessage.addAttribute(HttpMessage::parseAttribute("fromAddr:" + SrvrEndPoint));
	try {
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
			contentSize = Converter<size_t>::toValue(sizeString);
		if (contentSize > 0) {
			Socket::byte* buffer = new Socket::byte[contentSize + 1];
			msg.getBody(buffer, contentSize);
			buffer[contentSize] = '\0';
			string getMsgBody = buffer;
			delete[] buffer;
			if (getMsgBody.find('#') != std::string::npos) {
				string FirstSplit = getMsgBody.substr(getMsgBody.find('#') + 1);
				if (FirstSplit.find('#') != std::string::npos) {
					string CategoryName = FirstSplit.substr(0, FirstSplit.find('#'));
					string FileName = FirstSplit.substr(FirstSplit.find('#') + 1);
					string getDeletePath = argv[1];
					getDeletePath += '/' + CategoryName + '/' + FileName;
					cout << "\n FilePath to be deleted is :" + getDeletePath;
					string fullpath = FileSystem::Path::getFullFileSpec(getDeletePath);
					if (FileSystem::File::exists(fullpath)) {
						cout << "The file to be removed is: " << endl;
						cout << fullpath << endl;;
						FileSystem::File::remove(getDeletePath);
						cout << "\n  The file " + fullpath + " has been deleted from the Repoitory on Client request" << endl;
					}
					else {
						cout << "The requested path doesn't exist Change the path" << endl;
					}

				}
			}
		}
	}
	catch (std::exception& exc)
	{
		cout << "\n  Exeception caught in the method delete Single File On Server: \n";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << exMsg;
	}
}


//----<This method processes getCategories message requested by the client >----------

void getMessageGetCategories(HttpMessage &msg, int argc, char* argv[]) {

	string sendResult = "";
	HttpMessage sendMessage;
	std::string SrvrEndPoint = "localhost::8080";
	std::string ClientEndPoint = "localhost::8081";
	sendMessage.addAttribute(HttpMessage::attribute("GET", "Message"));
	sendMessage.addAttribute(HttpMessage::Attribute("mode", "oneWay"));
	sendMessage.addAttribute(HttpMessage::parseAttribute("toAddr:" + ClientEndPoint));
	sendMessage.addAttribute(HttpMessage::parseAttribute("fromAddr:" + SrvrEndPoint));
cout<<"\nServer needs to process the GetCategories message: \n";
	vector<std::string> categoriesList = FileSystem::Directory::getDirectories(argv[1]);
	for (int i = 0; i <categoriesList.size(); i++)
	{
		if ((categoriesList[i] != ".") && (categoriesList[i] != ".."))
		{
			sendResult += categoriesList[i];
			if (i != categoriesList.size() - 1)
			{
				sendResult += ",";
			}
		}
	}
	sendMessage.addAttribute(HttpMessage::Attribute("messageType", "getCategoriesReply"));
	if (sendResult.size() > 0)
	{
		sendMessage.addAttribute(HttpMessage::attribute("content-length", Converter<size_t>::toString(sendResult.size())));
		sendMessage.addBody(sendResult); 
	}
	categoriesList.clear();
	cout<<"\nMessage to be sent is " + sendMessage.toString();
	finalSendMessageToClient(sendMessage);
}
//----<This method is the entry point for all the Get Messages requests coming to the client >----------
bool MessageHandler_Get(HttpMessage &msg, int argc, char* argv[])
{
	string sendResult = "";
	HttpMessage sendMessage;
	std::string ServerEndPoint = "localhost::8080";
	std::string ClientEndPoint = "localhost::8081";
	sendMessage.addAttribute(HttpMessage::attribute("GET", "Message"));
	sendMessage.addAttribute(HttpMessage::Attribute("mode", "oneWay"));
	sendMessage.addAttribute(HttpMessage::parseAttribute("toAddr:" + ClientEndPoint));
	sendMessage.addAttribute(HttpMessage::parseAttribute("fromAddr:" + ServerEndPoint));
	if (msg.findValue("messageType") != "")
	{

		cout << "Repository recieved a new GET Message from the Client: " << msg.findValue("messageType") << endl;
		if (msg.findValue("messageType").compare("getCategories") == 0)
		{
			getMessageGetCategories(msg, argc, argv);
		}
		else if (msg.findValue("messageType").compare("getFileNamesFromServer") == 0)
		{
			getMessage_getFileNames(msg, argc, argv);
		}
		else if (msg.findValue("messageType").compare("downloadFileFromServer") == 0) {
			getMessage_downloadFileFromServer(msg, argc, argv);
		}
		else if (msg.findValue("messageType").compare("downloadJsAndCssFromServer") == 0) {
			string FileHtmPath = "../../";
			string jsfile = "myjsscript.js";
			string cssfile = "mystylesheet.css";
			sendingFileToClient(jsfile, FileHtmPath, argc, argv);
			sendingFileToClient(cssfile, FileHtmPath, argc, argv);
		}
	}
	return true;
}



// ----< this defines processing to frame messages for sending from the Client >------------------

HttpMessage ClientHandler::frameMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;
	while (true)
	{
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1)
		{
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
			msg.addAttribute(attrib);
		}
		else
			break;
	}
	if (msg.attributes().size() == 0)
	{
		connectionClosed_ = true;
		return msg;
	}
	if (msg.attributes()[0].first == "POST")
		msg = frameMessageForPost(msg, socket);
	else if (msg.attributes()[0].first == "GET") {
		std::string getMessageType = msg.findValue("messageType");
		size_t numBytes = 0;
		size_t pos = msg.findAttribute("content-length");
		if (pos < msg.attributes().size())
		{
			numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
			Socket::byte* buffer = new Socket::byte[numBytes + 1];
			socket.recv(numBytes, buffer);
			buffer[numBytes] = '\0';
			std::string msgBody(buffer);
			msg.addBody(msgBody);
			delete[] buffer;
		}
	}
	return msg;
}




//----<This method is the entry point for all the Post Messages requests coming from the client >----------
bool processMessage_Post(HttpMessage &msg, int argc, char* argv[]) {
	cout<<"\nServer needs to process the Post message request from the client: \n"<<endl;
	if (msg.findValue("messageType") != "")
	{
		
	 if (msg.findValue("messageType").compare("deleteFilesMessage") == 0) {
			cout << "\n Delete file from server message request from the client needs to be processed by server: \n";
			FileDeletionFromServer(msg, argc, argv);
		}
		else if (msg.findValue("messageType").compare("runCodeAnalyzer") == 0) {
			cout << "\n run code analyzer message request from client needs to be processed by server: \n";
			CodeAnalyze(argc, argv);
		}
	}
	return true;
}

//----<This method is builds the Post Message which is sent from the Repository >----------
HttpMessage ClientHandler::frameMessageForPost(HttpMessage& msg, Socket& socket) {

	std::string filename = msg.findValue("file");
	if (filename != "")
	{
		size_t contentSize;
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
			contentSize = Converter<size_t>::toValue(sizeString);
		else
			return msg;

		std::string getDir = msg.findValue("directory");

		readFile(filename, contentSize, getDir, socket);
	}

	if (filename != "")
	{
		msg.removeAttribute("content-length");
		std::string bodyString = "<file>" + filename + "</file>";
		std::string sizeString = Converter<size_t>::toString(bodyString.size());
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		msg.addBody(bodyString);
	}
	else
	{
		size_t numBytes = 0;
		size_t pos = msg.findAttribute("content-length");
		if (pos < msg.attributes().size())
		{
			numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
			Socket::byte* buffer = new Socket::byte[numBytes + 1];
			socket.recv(numBytes, buffer);
			buffer[numBytes] = '\0';
			std::string msgBody(buffer);
			msg.addBody(msgBody);
			delete[] buffer;
		}
	}

	return msg;
}




//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string& filename, size_t fileSize, 
	                         const std::string& getDir, Socket& socket)
{
	//std::string fqname = "../TestFiles_Client_Server/" + filename + ".snt";
  	std::string checkDirPath = path_+"/"+getDir; 
	if (FileSystem::Directory::exists(checkDirPath) == false)
	{
		FileSystem::Directory::create(checkDirPath);
	}
	std::string fqname = path_ + "/" + getDir + "/" + filename;
	cout<<" Trying to read File: " + fqname + " \n";
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		cout<<"\n\n  can't open file " + fqname;
		return false;
	}

	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	size_t bytesToRead;
	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		FileSystem::Block blk;
		for (size_t i = 0; i < bytesToRead; ++i)
			blk.push_back(buffer[i]);

		file.putBlock(blk);
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
	file.close();
	return true;
}


//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
	
	while (true)
	{
		HttpMessage msg = frameMessage(socket);
	
		if (connectionClosed_ || msg.bodyString() == "quit")
		{
			cout<<"\n\n  clienthandler thread is terminating \n";
			break;
		}
		msgQ_.enQ(msg);
	}
}
//----< override of FileMgr::dir(...) to save current dir >----------

void AnalFileMgr::dir(const Dir& d)
{
	d_ = d;
	++numDirs_;
}
//----< override of FileMgr::done(), not currently used >------------

void AnalFileMgr::done()
{
}
//----< returns number of matched files from search >----------------

size_t AnalFileMgr::numFiles()
{
	return numFiles_;
}
//----< initialize application specific FileMgr >--------------------
/*
* - Accepts CodeAnalysisExecutive's path and fileMap by reference
*/
AnalFileMgr::AnalFileMgr(const Path& path, FileMap& fileMap)
	: FileMgr(path), fileMap_(fileMap), numFiles_(0), numDirs_(0) {}

//----< override of FileMgr::file(...) to store found files >------

void AnalFileMgr::file(const File& f)
{
	File fqf = d_ + "\\" + f;
	Ext ext = FileSystem::Path::getExt(fqf);
	Pattern p = "*." + ext;
	fileMap_[p].push_back(fqf);
	++numFiles_;
}

//----< returns number of dirs searched >----------------------------

size_t AnalFileMgr::numDirs()
{
	return numDirs_;
}


//----< This method processes the command line arguments >---------


bool ClientHandler::ProcessingCommandLine(int argc, char * argv[])
{
	
	try {
		path_ = FileSystem::Path::getFullFileSpec(argv[1]);
		if (!FileSystem::Directory::exists(path_))
		{
			std::cout << "\n\n  path \"" << path_ << "\" does not exist\n\n";
			return false;
		}

	}
	catch (std::exception& ex)
	{
		Utils::Title("\n command line argument parsing error:");
		std::cout << "\n  " << ex.what() << "\n\n";
		return false;
	}
	
	return true;
}



/////////////////////////////////////////////////////////////////////
// CodeAnalysisExecutive class
// - 
using Path = std::string;
using Pattern = std::string;
using Patterns = std::vector<Pattern>;
using File = std::string;
using Files = std::vector<File>;
using Ext = std::string;
using FileMap = std::unordered_map<Pattern, Files>;
using ASTNodes = std::vector<ASTNode*>;
//----< usage message >----------------------------------------------

void showUsage()
{
	std::ostringstream out;
	out << "\n  Usage:";
	out << "\n  Command Line Arguments are:";
	out << "\n  - 1st: path to subdirectory containing files to analyze";
	out << "\n  - remaining non-option arguments are file patterns, e.g., *.h and/or *.cpp, etc.";
	out << "\n  - must have at least one file pattern to specify what to process";
	out << "\n  - option arguments have the format \"\"/x\" , where x is one of the options:";
	out << "\n    - m : display function metrics";
	out << "\n    - s : display file sizes";
	out << "\n    - a : display Abstract Syntax Tree";
	out << "\n    - r : set logger to display results";
	out << "\n    - d : set logger to display demo outputs";
	out << "\n    - b : set logger to display debug outputs";
	out << "\n    - f : write all logs to logfile.txt";
	out << "\n  A metrics summary is always shown, independent of any options used or not used";
	out << "\n\n";
	std::cout << out.str();

}
using FileToNodeCollection = std::vector<std::pair<File, ASTNode*>>;

//Initializng the static map
std::unordered_map<std::string, std::set<std::string>> CodeAnalysisExecutive::dependencyMapForUse = { {} };
//----< initialize parser, get access to repository >----------------

CodeAnalysisExecutive::CodeAnalysisExecutive()
{
  pParser_ = configure_.Build();
  if (pParser_ == nullptr)
  {
    throw std::exception("couldn't create parser");
  }
  pRepo_ = Repository::getInstance();
}

//----< cleanup >----------------------------------------------------

CodeAnalysisExecutive::~CodeAnalysisExecutive()
{
  /*
   *  Nothing to do:
   *  - pParser_ and pRepo_ point to objects that the configure_
   *    destructor will delete.
   *  - The AbstractSynTree object will cleanup its contents when
   *    it goes out of scope by deleting the root of the AST node
   *    tree.  
   *  - Each node deletes its children, so, again, nothing more 
   *    to do.
   *  - This is here to present these comments and to make this
   *    base destructor virtual.
   */
}

//----< show command line arguments >--------------------------------

void CodeAnalysisExecutive::showCommandLineArguments(int argc, char* argv[])
{
  std::ostringstream out;
  out << "\n     Path: \"" << FileSystem::Path::getFullFileSpec(argv[1]) << "\"\n     Args: ";
  for (int i = 2; i < argc - 1; ++i)
    out << argv[i] << ", ";
  out << argv[argc - 1];
  cout<<out.str();

}
//----< handle command line arguments >------------------------------
/*
* Arguments are:
* - path: possibly relative path to folder containing all analyzed code,
*   e.g., may be anywhere in the directory tree rooted at that path
* - patterns: one or more file patterns of the form *.h, *.cpp, and *.cs
* - options: /m (show metrics), /s (show file sizes), and /a (show AST)
*/
bool CodeAnalysisExecutive::ProcessCommandLine(int argc, char* argv[])
{
  if (argc < 2)
  {
    showUsage();
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
      showUsage();
      return false;
    }
  }
  catch (std::exception& ex)
  {
	Utils::Title("\ncommand line argument parsing error:");
    std::cout << "\n  " << ex.what() << "\n\n";
    return false;
  }
  return true;
}
//----< returns path entered on command line >-------------------

std::string CodeAnalysisExecutive::getAnalysisPath()
{
  return path_;
}
//----< returns reference to FileMap >---------------------------
/*
 * Supports quickly finding all the files found with a give pattern
 */
FileMap& CodeAnalysisExecutive::getFileMap()
{
  return fileMap_;
}
//----< searches path for files matching specified patterns >----
/*
 * - Searches entire diretory tree rooted at path_, evaluated 
 *   from a command line argument.
 * - Saves found files in FileMap.
 */
void CodeAnalysisExecutive::getSourceFiles()
{
  AnalFileMgr fm(path_, fileMap_);
  for (auto patt : patterns_)
    fm.addPattern(patt);
  fm.search();
  numFiles_ = fm.numFiles();
  numDirs_ = fm.numDirs();
}

std::string CodeAnalysisExecutive::systemTime()
{
	time_t sysTime = time(&sysTime);
	char buffer[27];
	ctime_s(buffer, 27, &sysTime);
	buffer[24] = '\0';
	std::string temp(buffer);
	return temp;
}

//----< helper: is text a substring of str? >--------------------

bool contains(const std::string& str, const std::string& text)
{
  if (str.find(text) < str.length())
    return true;
  return false;
}
//----< retrieve from fileMap all files matching *.h >-----------

std::vector<File>& CodeAnalysisExecutive::cppHeaderFiles()
{
  cppHeaderFiles_.clear();
  for (auto item : fileMap_)
  {
    if (contains(item.first, "*.h"))
    {
      for (auto file : item.second)
        cppHeaderFiles_.push_back(file);
    }
  }
  return cppHeaderFiles_;
}

//----< retrieve from fileMap all files matching *.cpp >---------

std::vector<File>& CodeAnalysisExecutive::cppImplemFiles()
{
  cppImplemFiles_.clear();
  for (auto item : fileMap_)
  {
    if (contains(item.first, "*.cpp"))
    {
      for (auto file : item.second)
        cppImplemFiles_.push_back(file);
    }
  }
  return cppImplemFiles_;
}
//----< retrieve from fileMap all files matching *.cs >----------

std::vector<File>& CodeAnalysisExecutive::csharpFiles()
{
  csharpFiles_.clear();
  for (auto item : fileMap_)
  {
    if (contains(item.first, "*.cs"))
    {
      for (auto file : item.second)
        csharpFiles_.push_back(file);
    }
  }
  return csharpFiles_;
}
//----< report number of Source Lines Of Code (SLOCs) >----------

CodeAnalysisExecutive::Slocs CodeAnalysisExecutive::fileSLOCs(const File& file) 
{ 
  return slocMap_[file];
}
//----< report number of files processed >-----------------------

size_t CodeAnalysisExecutive::numFiles()
{
  return numFiles_;
}
//----< report number of directories searched >------------------

size_t CodeAnalysisExecutive::numDirs()
{
  return numDirs_;
}
//----< show processing activity >-------------------------------

void CodeAnalysisExecutive::showActivity(const File& file)
{
  std::function<std::string(std::string, size_t)> trunc = [](std::string in, size_t count)
  {
    return in.substr(0, count);
  };

 // if (Rslt::running())
  {
    std::cout << std::left << "\r     Processing file: " << std::setw(80) << trunc(file, 80);
  }
}

void CodeAnalysisExecutive::clearActivity()
{
 // if (Rslt::running())
  {
    std::cout << std::left << "\r                      " << std::setw(80) << std::string(80,' ');
  }
}
//----< parses code and saves results in AbstrSynTree >--------------
/*
* - Processes C++ header files first to build AST with nodes for
*   all public classes and structs.
* - Then processes C++ implementation files.  Each member function
*   is relocated to its class scope node, not the local scope.
* - Therefore, this ordering is important.
* - C# code has all member functions inline, so we don't need to
*   do any relocation of nodes in the AST.  Therefore, that analysis
*   can be done at any time.
* - If you bore down into the analysis code in ActionsAndRules.h you
*   will find some gymnastics to handle template syntax.  That can
*   get somewhat complicated, so there may be some latent bugs there.
*   I don't know of any at this time.
*/
void CodeAnalysisExecutive::setLanguage(const File& file)
{
  std::string ext = FileSystem::Path::getExt(file);
  if (ext == "h" || ext == "cpp")
    pRepo_->language() = Language::Cpp;
  else if (ext == "cs")
    pRepo_->language() = Language::CSharp;
}

void CodeAnalysisExecutive::processSourceCode(bool showProc)
{
  for (auto file : cppHeaderFiles())
  {
    if (showProc)
      showActivity(file);
    pRepo_->package() = FileSystem::Path::getName(file);

    if (!configure_.Attach(file))
    {
      std::ostringstream out;
      out << "\n  could not open file " << file << "\n";
     cout<<out.str();
     // Rslt::flush();
      continue;
    }
   
	cout << "\n\n  opening file \"" << pRepo_->package() << "\"" << endl;
    pRepo_->language() = Language::Cpp;
    pRepo_->currentPath() = file;
    while (pParser_->next())
    {
      pParser_->parse();
    }

    Slocs slocs = pRepo_->Toker()->currentLineCount();
    slocMap_[pRepo_->package()] = slocs;
  }
  for (auto file : cppImplemFiles())
  {
    if (showProc)
      showActivity(file);
    pRepo_->package() = FileSystem::Path::getName(file);

    if (!configure_.Attach(file))
    {
      std::ostringstream out;
      out << "\n  could not open file " << file << "\n";
	  cout << out.str();
//      Rslt::write(out.str());
//      Rslt::flush();
      continue;
    }
    
	cout << "\n\n  opening file \"" << pRepo_->package() << "\"" << endl;

    pRepo_->language() = Language::Cpp;
    pRepo_->currentPath() = file;
    while (pParser_->next())
      pParser_->parse();

    Slocs slocs = pRepo_->Toker()->currentLineCount();
    slocMap_[pRepo_->package()] = slocs;
  }
  for (auto file : csharpFiles())
  {
    if (showProc)
      showActivity(file);
    pRepo_->package() = FileSystem::Path::getName(file);

    if (!configure_.Attach(file))
    {
      std::ostringstream out;
      out << "\n  could not open file " << file << "\n";
	  cout << out.str();
    //  Rslt::write(out.str());
      continue;
    }
    // parse file

    //if (!Rslt::running())
    //  Demo::write("\n\n  opening file \"" + pRepo_->package() + "\"");
    //if (!Demo::running() && !Rslt::running())
    //  Dbug::write("\n\n  opening file \"" + pRepo_->package() + "\"");
	cout << "\n\n  opening file \"" << pRepo_->package() << "\"" << endl;

    pRepo_->language() = Language::CSharp;
    pRepo_->currentPath() = file;
    while (pParser_->next())
      pParser_->parse();

    Slocs slocs = pRepo_->Toker()->currentLineCount();
    slocMap_[pRepo_->package()] = slocs;
  }
  if (showProc)
    clearActivity();
  std::ostringstream out;
  out << std::left << "\r  " << std::setw(77) << " ";
  cout << out.str();
//  Rslt::write(out.str());
}
//----< evaluate complexities of each AST node >---------------------

void CodeAnalysisExecutive::complexityAnalysis()
{
  ASTNode* pGlobalScope = pRepo_->getGlobalScope();
  Code_Analysis::complexityEval(pGlobalScope);
}
//----< comparison functor for sorting FileToNodeCollection >----
/*
* - supports stable sort on extension values
* - displayMetrics(...) uses to organize metrics display
*/
struct CompExts
{
  bool operator()(const std::pair<File, ASTNode*>& first, const std::pair<File, ASTNode*>& second)
  {
    return FileSystem::Path::getExt(first.first) > FileSystem::Path::getExt(second.first);
  }
};
//----< comparison functor for sorting FileToNodeCollection >----
/*
* - supports stable sort on name values
* - displayMetrics(...) uses these functions to organize metrics display
*/
static void removeExt(std::string& name)
{
  size_t extStartIndex = name.find_last_of('.');
  name = name.substr(0, extStartIndex);
}

struct CompNames
{
  bool operator()(const std::pair<File, ASTNode*>& first, const std::pair<File, ASTNode*>& second)
  {
    std::string fnm = FileSystem::Path::getName(first.first);
    removeExt(fnm);
    std::string snm = FileSystem::Path::getName(second.first);
    removeExt(snm);
    return fnm < snm;
  }
};
//----< display header line for displayMmetrics() >------------------

void CodeAnalysisExecutive::displayHeader()
{
  std::ostringstream out;
  out << std::right;
  out << "\n ";
  out << std::setw(25) << "file name";
  out << std::setw(12) << "type";
  out << std::setw(35) << "name";
  out << std::setw(8) << "line";
  out << std::setw(8) << "size";
  out << std::setw(8) << "cplx";
  out << std::right;
  out << "\n  ";
  out << std::setw(25) << "-----------------------";
  out << std::setw(12) << "----------";
  out << std::setw(35) << "---------------------------------";
  out << std::setw(8) << "------";
  out << std::setw(8) << "------";
  out << std::setw(8) << "------";
  cout << out.str();
 // Rslt::write(out.str());
}
//----< display single line for displayMetrics() >-------------------

void CodeAnalysisExecutive::displayMetricsLine(const File& file, ASTNode* pNode)
{
  std::function<std::string(std::string, size_t)> trunc = [](std::string in, size_t count)
  {
    return in.substr(0, count);
  };
  std::ostringstream out;
  out << std::right;
  out << "\n ";
  out << std::setw(25) << trunc(file, 23);
  out << std::setw(12) << pNode->type_;
  out << std::setw(35) << trunc(pNode->name_, 33);
  out << std::setw(8) << pNode->startLineCount_;
  out << std::setw(8) << pNode->endLineCount_ - pNode->startLineCount_ + 1;
  size_t debug1 = pNode->startLineCount_;
  size_t debug2 = pNode->endLineCount_;
  out << std::setw(8) << pNode->complexity_;
  cout << out.str();
  //Rslt::write(out.str());
}
//----< display lines containing public data declaration >-----------

std::string CodeAnalysisExecutive::showData(const Scanner::ITokCollection* pTc)
{
  std::string semiExpStr;
  for (size_t i=0; i<pTc->length(); ++i)
    semiExpStr += (*pTc)[i] + " ";
  return semiExpStr;
}

//------Displays the Data lines for logging
void CodeAnalysisExecutive::displayDataLines(ASTNode* pNode, bool isSummary)
{
  for (auto datum : pNode->decl_)
  {
    if (pNode->parentType_ == "namespace" || pNode->parentType_ == "class" || pNode->parentType_ == "struct")
    {
      if (pNode->type_ == "function" || pNode->parentType_ == "function")
        continue;
      if (datum.access_ == Access::publ && datum.declType_ == DeclType::dataDecl)
      {
        std::ostringstream out;
        out << std::right;
        out << "\n ";
        out << std::setw(25) << "public data:" << " ";
        if (isSummary)
        {
          out << datum.package_ << " : " << datum.line_ << " - "
            << pNode->type_ << " " << pNode->name_ << "\n " << std::setw(15) << " ";
        }
        out << showData(datum.pTc);
		cout << out.str();
      //  Rslt::write(out.str());
      }
    }
  }
}
//----<  helper for displayMetrics() >-------------------------------
/*
* - Breaking this out as a separate function allows application to
*   display metrics for a subset of the Abstract Syntax Tree
*/
void CodeAnalysisExecutive::displayMetrics(ASTNode* root)
{
  flushLogger();
  std::ostringstream out;
  out << "Code Metrics - Start Line, Size (lines/code), and Complexity (number of scopes)";
  Utils::sTitle(out.str(), 3, 92, out, '=');
  out << "\n";
  cout << out.str();
 // Rslt::write(out.str());

  std::function<void(ASTNode* pNode)> co = [&](ASTNode* pNode) {
    if (
      pNode->type_ == "namespace" ||
      pNode->type_ == "function" ||
      pNode->type_ == "class" ||
      pNode->type_ == "interface" ||
      pNode->type_ == "struct" ||
      pNode->type_ == "lambda"
      )
      fileNodes_.push_back(std::pair<File, ASTNode*>(pNode->package_, pNode));
  };
  ASTWalkNoIndent(root, co);
  std::stable_sort(fileNodes_.begin(), fileNodes_.end(), CompExts());
  std::stable_sort(fileNodes_.begin(), fileNodes_.end(), CompNames());

  displayHeader();

  std::string prevFile;
  for (auto item : fileNodes_)
  {
    if (item.first != prevFile)
    {
		cout << endl;
     // Rslt::write("\n");
      displayHeader();
    }
    displayMetricsLine(item.first, item.second);
    displayDataLines(item.second);
    prevFile = item.first;
  }
  cout << endl;
 // Rslt::write("\n");
}
//----< display metrics results of code analysis >---------------

void CodeAnalysisExecutive::displayMetrics()
{
  ASTNode* pGlobalScope = pRepo_->getGlobalScope();
  displayMetrics(pGlobalScope);
}
//----< walk tree of element nodes >---------------------------------

template<typename element>
void TreeWalk(element* pItem, bool details = false)
{
  static std::string path;
  if (path != pItem->path_ && details == true)
  {
    path = pItem->path_;
	cout << endl << path;
   // Rslt::write("\n" + path);
  }
  static size_t indentLevel = 0;
  std::ostringstream out;
  out << "\n  " << std::string(2 * indentLevel, ' ') << pItem->show();
  cout << out.str();
  //Rslt::write(out.str());
  auto iter = pItem->children_.begin();
  ++indentLevel;
  while (iter != pItem->children_.end())
  {
    TreeWalk(*iter);
    ++iter;
  }
  --indentLevel;
}
//----< display the AbstrSynTree build in processSourceCode() >------

void CodeAnalysisExecutive::displayAST()
{
  flushLogger();
  ASTNode* pGlobalScope = pRepo_->getGlobalScope();
  Utils::title("Abstract Syntax Tree");
  TreeWalk(pGlobalScope);
  cout << endl;
 // Rslt::write("\n");
}
//----< show functions with metrics exceeding specified limits >-----

void CodeAnalysisExecutive::displayMetricSummary(size_t sMax, size_t cMax)
{
  flushLogger();
  std::ostringstream out;
  Utils::sTitle("Functions Exceeding Metric Limits and Public Data", 3, 92, out, '=');
  cout << out.str();
  //Rslt::write(out.str());
  displayHeader();

  if (fileNodes_.size() == 0)  // only build fileNodes_ if displayMetrics hasn't been called
  {
    std::function<void(ASTNode* pNode)> co = [&](ASTNode* pNode) {
      fileNodes_.push_back(std::pair<File, ASTNode*>(pNode->package_, pNode));
    };
    ASTNode* pGlobalNamespace = pRepo_->getGlobalScope();
    ASTWalkNoIndent(pGlobalNamespace, co);
    std::stable_sort(fileNodes_.begin(), fileNodes_.end(), CompExts());
    std::stable_sort(fileNodes_.begin(), fileNodes_.end(), CompNames());
  }
  for (auto item : fileNodes_)
  {
    if (item.second->type_ == "function")
    {
      size_t size = item.second->endLineCount_ - item.second->startLineCount_ + 1;
      size_t cmpl = item.second->complexity_;
      if (size > sMax || cmpl > cMax)
        displayMetricsLine(item.first, item.second);
    }
  }
  cout << endl;
  //Rslt::write("\n");
  for (auto item : fileNodes_)
  {
    displayDataLines(item.second, true);
  }
  cout << endl;
 // Rslt::write("\n");
}
//----< comparison functor for sorting SLOC display >----------------

struct compFiles
{
private:
  std::string ChangeFirstCharOfExt(const std::string& fileName) const
  {
    std::string temp = fileName;
    size_t pos = temp.find_last_of('.');
    if (pos < temp.size() - 1)
      if (temp[pos + 1] == 'h')
        temp[pos + 1] = 'a';
    return temp;
  }
public:
  bool operator()(const std::string& fileName1, const std::string& fileName2) const
  {
    return ChangeFirstCharOfExt(fileName1) < ChangeFirstCharOfExt(fileName2);
  }
};
//----< show sizes of all the files processed >----------------------

void CodeAnalysisExecutive::displaySlocs()
{
  flushLogger();
  Utils::sTitle("File Size - Source Lines of Code", 3, 92);
  size_t slocCount = 0;
  std::map<std::string, size_t, compFiles> fileColl;
  for (auto item : fileMap_)
  {
    for (auto file : item.second)
    {
      File fileName = FileSystem::Path::getName(file);
      fileColl[file] = slocMap_[fileName];
    }
  }
  for (auto fitem : fileColl)
  {
    std::ostringstream out;
    out << "\n  " << std::setw(8) << fitem.second << " : " << fitem.first;
	cout << out.str();
    //Rslt::write(out.str());
    slocCount += fitem.second;
  }
  std::ostringstream out;
  out << "\n\n      Total line count = " << slocCount << "\n";
  cout << out.str() << endl;
  /*Rslt::write(out.str());
  Rslt::write("\n");*/
}
//----< display analysis info based on command line options >--------

void CodeAnalysisExecutive::dispatchOptionalDisplays()
{
  for (auto opt : options_)
  {
    switch (opt)
    {
    case 'm':
      displayMetrics();
      Rslt::start();
      break;
    case 'a':
      displayAST();
      Rslt::start();
      break;
    case 's':
      displaySlocs();
      Rslt::start();
      break;
    default:
      break;
    }
  }
}
//----< display analysis info based on command line options >--------

void CodeAnalysisExecutive::setDisplayModes()
{
  for (auto opt : options_)
  {
    switch (opt)
    {
    case 'r':
      Rslt::start();
      break;
    case 'd':
      Demo::start();
      break;
    case 'b':
      Dbug::start();
      break;
    case 'f':
      setLogFile("logFile.txt");
      break;
    default:
      if (opt != 'a' && opt != 'b' && opt != 'd' && opt != 'f' && opt != 'm' && opt != 'r' && opt != 's')
      {
        std::cout << "\n\n  unknown option " << opt << "\n\n";
      }
    }
  }
}
//----< helper functions for managing application's logging >--------

void CodeAnalysisExecutive::startLogger(std::ostream& out)
{
  Rslt::attach(&out);
  Demo::attach(&out);
  Dbug::attach(&out);

  // will start Demo and Dbug if  have options /d and /b

  setDisplayModes();
}


//------Flush Logger
void CodeAnalysisExecutive::flushLogger()
{
  Rslt::flush();
  Demo::flush();
  Dbug::flush();
}

//-----Stops Logger
void CodeAnalysisExecutive::stopLogger()
{
  Rslt::flush();
  Demo::flush();
  Dbug::flush();
  Rslt::stop();
  Demo::stop();
  Dbug::stop();
}



//-----This method starts a server at the startup so that Repository can listen to the Messages

void serverStart(int argc, char* argv[]) {
	//Rslt::attach(&std::cout);
	//Rslt::start();
	cout << "\n Server has been started at the Repository end" << endl;
	BlockingQueue<HttpMessage> msgQ;
	try
	{
		SocketSystem ss;
		SocketListener AB(8080, Socket::IP6);
		ClientHandler ch(msgQ);
		ch.ProcessingCommandLine(argc, argv);
		AB.start(ch);
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
			cout << "\n\n  Server received a new message with the contents:\n  " << msg.toString();
			//Rslt::write("  ");
			//Rslt::write(msg.toString());
			if (msg.attributes().size() > 0)
			{
				if (msg.attributes()[0].first == "GET")
				{
					MessageHandler_Get(msg, argc, argv);
				}
				else if (msg.attributes()[0].first == "POST")
				{
					processMessage_Post(msg, argc, argv);
				}
			}
		//	Rslt::flush();
		}
	}
	catch (std::exception& exc)
	{
	//	Rslt::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
	//	Rslt::write(exMsg);
		cout << "\n  Exeception caught: " << exMsg;
	}
}
//----< open file stream for logging >-------------------------------
/*
*  - must come after CodeAnalysisExecutive::processCommandLine()
*  - must come before starting any of the loggers
*/
void CodeAnalysisExecutive::setLogFile(const File& file)
{
  std::string path = getAnalysisPath();
  path += "\\" + file;
  pLogStrm_ = new std::ofstream(path);
  if (pLogStrm_->good())
  {
    Rslt::attach(pLogStrm_);
    Demo::attach(pLogStrm_);
    Dbug::attach(pLogStrm_);
  }
  else
    Rslt::write("\n  couldn't open logFile.txt for writing");
}

#include <fstream>




void receiveThread(int argc, char* argv[]) {
	try {
		serverStart(argc, argv);
	}
	catch (std::exception& ex) {
		cout << "Exception caught in receiver thread" << endl;
		cout << ex.what() << endl;
	}
}
void requirementsPrint() {
	Utils::Title("Requirement9 : include an automated unit test suite that demonstrates you meet all the requirements of this project4 including the transmission of files. Repository and client console implements this requirement");
	Utils::Title("Requirement1 : Windows Presentation Foundation (WPF) to provide a required client Graphical User Interface (GUI) is used and Visual Studio 2015 and its C++ Windows console projects are used for the rest of projects");
	Utils::Title("Requirement2 : using the C++ standard library's streams for all console I/O and new and delete for all heap-based memory management.");
	Utils::Title("Requirement3 :A Repository program that provides functionality to publish, as linked web pages, the contents of a set of C++ source code files. Please click on Analyze button to anaylyze the code and generate html files");
	Utils::Title("Requirement4 :TypeAnalysis,Dependency Analysis and publishing of pages is done as in Project3");
	Utils::Title("Requirement6 : All the communication happens throgh sockets. Please check the code ");
	Utils::Title("Requirement7 : All the messages being passed from Repository and client(Vice versa) are http style messages ");
	Utils::Title("Requirement8 : File upload and download is happening in bytes. Streams are estlabished with initial exchange of messsages. ");
	Utils::Title("Requirement10 : IIS is setup. Please read the readme file how to give inputs to run html files on IIS server");
}

int main(int argc, char* argv[])
{	

	::SetConsoleTitle(L"Repository");
	requirementsPrint();
  std::thread initialThreadForReceivingMessages(receiveThread, argc, argv); 
  initialThreadForReceivingMessages.join();
  return 0;
}