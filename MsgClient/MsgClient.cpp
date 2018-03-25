/////////////////////////////////////////////////////////////////////////
// MsgClient.cpp - Demonstrates simple one-way HTTP messaging          //
//                                                                     //
//Rishit Reddy Muthyala, CSE687 - Object Oriented Design, Spring 2016  //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
* This package implements a client that sends HTTP style messages and
* files to a server that simply displays messages and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/
/*
* Required Files:
*   MsgClient.cpp, MsgServer.cpp
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp
*   Utilities.h, Utilities.cpp
*/
#include "MsgClient.h"
#include "../FileSystem/FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <shellapi.h>
#include <Windows.h>

//----<This method is the entry point for all the Get Messages requests coming to the client >----------

HttpMessage ClientHandler::readingMessageForGetFunc(HttpMessage& msg, Socket& socket) {
	std::string getMessage = msg.findValue("messageType");
	Show::write("\n");
	if (getMessage.compare("getCategoriesReply") == 0)
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
	else if (getMessage.compare("getFileNamesReply") == 0)
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
	else if (getMessage.compare("HtmFileContentReplyFromServer") == 0) {
		acceptFileClickedFromServer(msg, socket);
	}

	return msg;

}

//-----This method process the file coming from the Repository and builds a file at the Client end
bool ClientHandler::acceptFileClickedFromServer(HttpMessage &msg, Socket& socket) {
	try {
		::SetConsoleTitle("Client");
		const size_t BlockSize = 2048;
		Socket::byte buffer[BlockSize];
		std::string fileStoragePath;
		if (msg.findValue("fileName").find(".htm") != std::string::npos)
			fileStoragePath = "../../../clientRepository/CoreFileRepoClient/" + msg.findValue("fileName");
		else
			fileStoragePath = "../../../clientRepository/" + msg.findValue("fileName");
		if (FileSystem::File::exists(fileStoragePath))
		{
			cout << "File already exist: " << fileStoragePath << endl<<"Deleting the file...";
			FileSystem::File::remove(fileStoragePath);
		}
		FileSystem::File file(fileStoragePath);
		file.open(FileSystem::File::out, FileSystem::File::binary);
		if (file.isGood())
			cout << "File is good in client end" << endl;
		size_t bytesToRead;
		string getFileSize = msg.findValue("content-length");
		size_t fileSize = Converter<size_t>::toValue(getFileSize);
		while (true)
		{
			if (fileSize > BlockSize)
				bytesToRead = BlockSize;
			else
				bytesToRead = fileSize;
			socket.recv(bytesToRead, buffer);
			FileSystem::Block blk;
			for (size_t i = 0; i < bytesToRead; ++i) {
				blk.push_back(buffer[i]);
			}
			file.putBlock(blk);
			if (fileSize < BlockSize)
				break;
			fileSize -= BlockSize;
		}
		file.close();
		cout << "The File : " + msg.findValue("fileName") + " is downloaded successfully on the client side successfully" << endl;
	}
	catch (std::exception& except){
		cout << "Exception caught in acceptFileFromServer of client " <<  std::string(except.what()) <<  "\n\n";
	}
	return true;
}


//----<This method is the entry point for all the Post Messages requests coming to the client >----------
HttpMessage ClientHandler::readMessageForPost(HttpMessage& msg, Socket& socket) {

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

		readingFile(filename, contentSize, getDir, socket);
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




//----< this defines processing to frame messages >------------------
//----< Used the professor's code directly >------------------

HttpMessage ClientHandler::readingMessage(Socket& socket)
{
	HttpMessage msg;
	connectionClosed_ = false;
	try {
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
			msg = readMessageForPost(msg, socket);
		
		else if (msg.attributes()[0].first == "GET") 
			msg = readingMessageForGetFunc(msg, socket);
		cout << "\n The message received from the server is\n " << msg.toString() << "\n";
	}
	catch (std::exception& except)
	{ 
		cout << "Exception caught in readMessage of client " << std::string(except.what()) << "\n\n";
	}
	return msg;
}

//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readingFile(const std::string& filename, size_t fileSize,
	const std::string& getDir, Socket& socket)
{
	//std::string fqname = "../TestFiles_Client_Server/" + filename + ".snt";
	std::string checkDirPath = path_ + "/" + getDir;
	if (FileSystem::Directory::exists(checkDirPath) == false)
	{
		FileSystem::Directory::create(checkDirPath);
	}
	std::string fqname = path_ + "/" + getDir + "/" + filename;
//	Show::write(" Trying to readFile: " + fqname + " \n");
	cout << "Trying to readFile: " << fqname << " \n";
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		cout << "\n\n can't open file " << fqname << endl;
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
		HttpMessage msg = readingMessage(socket);
		if (connectionClosed_ || msg.bodyString() == "quit")
		{
		//	Show::write("\n\n  clienthandler thread is terminating");
			cout << "\n\n clientHandler thread is terminationg " << endl;
			break;
		}
		msgQ_.enQ(msg);
	}
}


/////////////////////////////////////////////////////////////////////
// ClientCounter creates a sequential number for each client
//
class ClientCounter
{
public:
	ClientCounter() { ++clientCount; }
	size_t count() { return clientCount; }
private:
	static size_t clientCount;
};

size_t ClientCounter::clientCount = 0;




// --< start  CLient to listen for connections and push them to the Blocking Queue >-----------
void MsgClient::startingClient(BQueue &sharedQueue)
{
	Utils::Title("Requirement5 : Client Program is started. Select the files from the browse files button and uplaod them to the Repository.The files are uploaded to the Repository srver based on the project name as category.Click on View categories to view the categories in the Repository.On clicking the download button files are downloaded into clientRepository folder");
	std::thread t2([&]() {
		try {
			SocketSystem ss;
			SocketListener sl(8081, Socket::IP6);
			BlockingQueue<HttpMessage> msgQ;
			ClientHandler cp(msgQ);
			cp.ProcessingCommandLine();
			cout << "\n Client is enabled to listen on a socket for new messages...\n";
			sl.start(cp);
			while (true)
			{
				HttpMessage msg = msgQ.deQ();
				size_t numBytes = 0;
				size_t pos = msg.findAttribute("content-length");
				string getMessageType = msg.findValue("messageType");
				if ((pos < msg.attributes().size()) && (msg.findValue("messageType").compare("HtmFileContentReplyFromServer") != 0))
				{
					numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
					Socket::byte* buffer = new Socket::byte[numBytes + 1];
					msg.getBody(buffer, numBytes);
					buffer[numBytes] = '\0';
					cout << "\nMessage being enqueued to sender channel is " << msg.findValue("messageType") << "$" << buffer << endl;
					sharedQueue.enQ(msg.findValue("messageType")+ "$" + buffer);
					delete[] buffer;
				}

			}
		}
		catch (std::exception& exc) {
			std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
			cout << "\n  Exception caught: " << exMsg << endl;
		}
	});
	t2.detach();
}

bool ClientHandler::ProcessingCommandLine()
{
	
	return true;

}

//---This method is called at the start of the mock channel so the js and css files are downloaded at the start of the client
void MsgClient::downloadOfScriptsOnStartUpOfClient()
{
	Utils::Title("Client Requirements:On Client start js and css files are downloaded into the Client Repository");
	ClientCounter counter;
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	try
	{
		SocketSystem ssytem;
		SocketConnecter socketi;
		while (!socketi.connect("localhost", 8080))
		{
			cout << "\n MsgClient::downloadJsAndCssFromServer:: Client waiting to connect to the server \n";
			::Sleep(100);
		}
		HttpMessage msg = makeMessageToSend(2, "downloadJsAndCssFromServer", "localhost::8080");
		msg.addAttribute(HttpMessage::Attribute("messageType", "downloadJsAndCssFromServer"));
		sendMessageFromClient(msg, socketi);
		cout << " MsgClient::Message has been sent to the Repository server from the Client" << endl;
	}
	catch (std::exception& exc)
	{
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << "\n  Exeception caught in the method downloadJsAndCssFromServer: \n" << exMsg;
	}
}
//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
HttpMessage MsgClient::makeMessageToSend(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage message;
	HttpMessage::Attribute attribut;
	EndPoint myEndPoint = "localhost::8081";  // ToDo: make this a member of the sender
											 // given to its constructor.
	switch (n)
	{
	case 1:
		message.clear();
		message.addAttribute(HttpMessage::attribute("POST", "Message"));
		message.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		message.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		message.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));

		message.addBody(body);
		if (body.size() > 0)
		{
			attribut = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			message.addAttribute(attribut);

		}
		break;
	case 2:
		message.clear();
		message.addAttribute(HttpMessage::attribute("GET", "Message"));
		message.addAttribute(HttpMessage::Attribute("mode", "Duplex"));
		message.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		message.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));
		message.addBody(body);
		if (body.size() > 0)
		{
			attribut = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			message.addAttribute(attribut);
		}
		break;
	default:
		message.clear();
		message.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return message;
}

//----This message handles the request for code analysis from the Client side
void MsgClient::requestCodeAnaysisExecutive() {
	ClientCounter ccounter;
	size_t myCount = ccounter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	cout << " \n MsgClient::executeSingleMessage::A request for codeAnalyzer has been recieved from wpf by client \n";
	cout << "Starting HttpMessage client " << myCountString << "on thread " << Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id());
	try
	{
		SocketSystem ssytem;
		SocketConnecter socketi;
		while (!socketi.connect("localhost", 8080))
		{
			//	Show::write("\n MsgClient::sendCodeAnalysisToolRequest:: Client waiting to connect to the server \n");
			cout << "\n MsgClient::sendCodeAnalysisToolRequest:: Client waiting to connect to the server \n";
			::Sleep(100);
		}
		HttpMessage msg = makeMessageToSend(1, "RunCodeAnalysis", "localhost::8080");
		msg.addAttribute(HttpMessage::Attribute("messageType", "runCodeAnalysisOnRepository"));
		sendMessageFromClient(msg, socketi);
		cout << " MsgClient::sendCodeAnalysisToolRequest::Message has been sent to the Repository server from the Client" << endl;
	}
	catch (std::exception& exc)
	{
		//Show::write("\n  Exeception caught in the method sendCodeAnalysisToolRequest: \n");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		//Show::write(exMsg);
		cout << "\n  Exeception caught in the method sendCodeAnalysisToolRequest: \n" << exMsg;
	}
}



//----< send message using socket >----------------------------------

void MsgClient::sendMessageFromClient(HttpMessage& msg, Socket& socket)
{

	cout << "\n MsgClient::sendMessage : The message being sent from client end is: " << msg.toString() << "\n\n";
	std::string msgString = msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}
//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool MsgClient::sendFileFromClient(const std::string& fullFilePath, Socket& socket)
{
	std::string fqname = fullFilePath;
	FileSystem::FileInfo fi(fqname);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	FileSystem::File file(fqname);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
	{
		Show::write("File is not good to be read");
		cout << "File is not in good state to be read \n";
		return false;
	}
	/* Get only filename from file path */
	size_t found = fullFilePath.find_last_of("\\");
	std::string filename = fullFilePath.substr(found + 1);
	std::string getPath = fullFilePath.substr(0, found);
	std::string getDirectory = getPath.substr(getPath.find_last_of("\\") + 1);
	HttpMessage msg = makeMessageToSend(1, "", "localhost::8080");
	msg.addAttribute(HttpMessage::Attribute("file", filename));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
	msg.addAttribute(HttpMessage::Attribute("directory", getDirectory));
	sendMessageFromClient(msg, socket);
	
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	while (true)
	{
		FileSystem::Block blk = file.getBlock(BlockSize);
		if (blk.size() == 0)
			break;
		for (size_t i = 0; i < blk.size(); ++i)
			buffer[i] = blk[i];
		socket.send(blk.size(), buffer);
		if (!file.isGood())
			break;
	}
	cout << "\n The file " << filename << " has been sent from the client end \n";
	file.close();
	return true;
}


//---- This method process the Run code anayzer request coming from the Client GUI
void MsgClient::processRunCodeAnalysisFromClient(string requestMsg)
{
	ClientCounter counter;
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	cout << " \n MsgClient::executeSingleFileDownloadMessage:: A request for downloadfile has been recieved from wpf by client \n";
	cout <<
		"Starting HttpMessage client" + myCountString +
		" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id()) << endl;
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080))
		{
			cout << "\n MsgClient::executeRunCodeAnalyzerOnServer:: Client waiting to connect to the server \n" << endl;
			::Sleep(100);
		}
		HttpMessage msg = makeMessageToSend(1, requestMsg, "localhost::8080");
		msg.addAttribute(HttpMessage::Attribute("messageType", "runCodeAnalyzer"));
		cout << endl;
		sendMessageFromClient(msg, si);
		cout << " MsgClient::executeRunCodeAnalyzerOnServer::Message has been sent to the Repository server from the Client" << endl;
	}
	catch (std::exception& exc)
	{
		cout << "\n  Exeception caught in the method executeRunCodeAnalyzerOnServer: \n";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << exMsg << endl;
	}
}


//------Thie method processes the Delete Request coming from the Client GUI
void MsgClient::processDeleteRequestFromClient(string splitMsg) {
	ClientCounter counter;
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	cout << " \n MsgClient::executeSingleFileDeleteRequest:: A request for deletFile has been recieved from wpf by client \n";
	cout << "Starting HttpMessage client " << myCountString << "on thread " << Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id());
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080))
		{
			cout << "\n MsgClient::executeSingleFileDeleteRequest:: Client waiting to connect to the server \n";
			::Sleep(100);
		}
		HttpMessage msg = makeMessageToSend(1, splitMsg, "localhost::8080");
		msg.addAttribute(HttpMessage::Attribute("messageType", "deleteFilesMessage"));
		sendMessageFromClient(msg, si);
	}
	catch (std::exception& exc)
	{
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << "\n  Exeception caught in the method executeSingleMessage: \n" << exMsg;
	}
}


///-----This method processes file download request coming from the Client GUI
void MsgClient::processFileDownloadRequest(string splitMsg) {
	ClientCounter counter;
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	cout << " \n MsgClient::processFileDownloadRequest:: A request for downloadfile has been recieved from wpf by client" << endl;
	cout <<
		"Starting HttpMessage client" + myCountString +
		" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id()) << endl;

	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080))
		{
			cout<<"\n MsgClient::processFileDownloadRequest:: Client waiting to connect to the server \n"<<endl;
			::Sleep(100);
		}
		HttpMessage msg = makeMessageToSend(2, splitMsg, "localhost::8080");
		msg.addAttribute(HttpMessage::Attribute("messageType", "downloadFileFromServer"));
		cout << endl;
		sendMessageFromClient(msg, si);
		cout << " MsgClient::processFileDownloadRequest::Message has been sent to the Repository server from the Client" << endl;
	}
	catch (std::exception& exc)
	{
		cout<<"\n  Exeception caught in the method processFileDownloadRequest: \n";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << exMsg << endl;
	}

}


//----This method processes the fetching all the files based on the category from the Repository
void MsgClient::processGetAllFilesFromRepositoryForAParticluarCategorySpecified(string msg)
{

	std::string filecategorySelectedByClient = msg.substr(msg.find('#') + 1);
	cout << "\n MsgClient::processGetAllFilesFromRepoForAParticluarCategory::filecategory selected by the user is: " << filecategorySelectedByClient << endl;
	cout << endl;
	cout << " MsgClient::processGetAllFilesFromRepoForAParticluarCategory::A request for getFiles has been recieved from wpf by client" << endl;
	cout << endl;
	ClientCounter ccounter;
	size_t myCount = ccounter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	cout <<
		"Starting HttpMessage client" + myCountString +
		" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id()) << endl;
	cout << endl;
	cout << "\nA request for getfiles has been recieved from wpf by client" << endl;
	cout << endl;
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080))
		{
			cout << "\n MsgClient::processGetAllFilesFromRepoForAParticluarCategory:: Client waiting to connect to the server \n" << endl;
			::Sleep(100);
		}
		HttpMessage msg = makeMessageToSend(2, "getFilesRequest", "localhost::8080");
		msg.addAttribute(HttpMessage::Attribute("messageType", "getFileNamesFromServer"));
		msg.addAttribute(HttpMessage::Attribute("FileCategory", filecategorySelectedByClient));
		cout << endl;
		sendMessageFromClient(msg, si);
		cout << " MsgClient::processGetAllFilesFromRepoForAParticluarCategory::Message has been sent to the Repository server from the Client" << endl;
	}
	catch (std::exception& exc)
	{
		cout << "\n  Exeception caught in the method processGetAllFilesFromRepoForAParticluarCategory: \n";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << exMsg << endl;
	}

}




//-------This method process get all categories from the Repository on Client GUI request

void MsgClient::processGetCategoriesFromClient()
{
	try {
		cout << endl;
		cout << " MsgClient::processGetCategoriesMessageFromClient::A request for getcategories has been recieved from wpf by client" << endl;
		cout << endl;
		ClientCounter counter;
		size_t myCount = counter.count();
		std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
		cout <<
			"Starting HttpMessage client" + myCountString +
			" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id()) << endl;
		cout << endl;
		cout << endl;
		cout << "\nA request for processGetCategoriesMessageFromClient has been recieved from wpf by client" << endl;
		cout << endl;

		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080))
		{
			cout << "\n MsgClient::executeGetCategoriesMessage:: Client waiting to connect to the server \n" << endl;
			::Sleep(100);
		}
		HttpMessage msg = makeMessageToSend(2, "getcategoriesrequest", "localhost::8080");
		msg.addAttribute(HttpMessage::Attribute("messageType", "getCategories"));
		cout << endl;
		sendMessageFromClient(msg, si);
		cout << " MsgClient::processGetCategoriesMessageFromClient::Message being sent to the Repository server from the Client" << endl;
	}
	catch (std::exception& exc)
	{

		cout << "\n  Exeception caught in the method processGetCategoriesMessageFromClient: \n";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << exMsg << endl;
	}
}



//------This method processes the upload file request coming from the Client GUI

void MsgClient::processUploadFromClient(string filepath)
{
	ClientCounter counter;
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
cout<<" \n MsgClient::processUploadMessageFromClient:: A request for uploadfile has been recieved from wpf by client \n"<<endl;
	cout <<
		"Starting HttpMessage client" + myCountString +
		" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id()) << endl;

	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080))
		{
			cout << "\n MsgClient::processUploadMessageFromClient:: Client waiting to connect to the server \n" << endl;
			::Sleep(100);
		}
		sendFileFromClient(filepath, si);
	}
	catch (std::exception& exc)
	{
		cout << "\n  Exeception caught in the method processUploadMessageFromClient: \n";
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		cout << exMsg << endl;
	}
}



#ifdef  TEST_MESSAGE_CLIENT
int main(int argc, char* argv[])
{
	::SetConsoleTitle("Clients Running on Threads");

	Show::title("Demonstrating two HttpMessage Clients each running on a child thread");

	MsgClient c1;
	std::thread t1(
		[&]() { c1.execute(100, 20); } // 20 messages 100 millisec apart
	);

	std::thread t2([&]() { c1.startReceiver(argc, argv); });

	//MsgClient c2;
	//std::thread t2(
	//	[&]() { c2.execute(120, 20); } // 20 messages 120 millisec apart
	//);
	t1.join();
	t2.join();
	//t2.join();
	getchar();
}
#endif //  TEST_MESSAGE_CLIENT