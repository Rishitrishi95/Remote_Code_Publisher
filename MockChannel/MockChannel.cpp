///////////////////////////////////////////////////////////////////////////////////////
// MockChannel.cpp - Demo for CSE687 Project #4, Spring 2015                         //
// - build as static library showing how C++\CLI client can use native code channel  //
// - MockChannel reads from sendQ and writes to recvQ                                //
//                                                                                   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2015                         //
///////////////////////////////////////////////////////////////////////////////////////

#define IN_DLL
#include "MockChannel.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../MsgClient/MsgClient.h"
#include <string>
#include <thread>
#include <iostream>
using namespace Async;
using BQueue = BlockingQueue<Message>;

/////////////////////////////////////////////////////////////////////////////
// Sendr class
// - accepts messages from client for consumption by MockChannel
//
class Sendr : public ISendr
{
public:
	void postMessage(const Message& msg);
	BQueue& queue();
private:
	BQueue sendQ_;
};

void Sendr::postMessage(const Message& msg)
{
	sendQ_.enQ(msg);
}

BQueue& Sendr::queue() { return sendQ_; }

/////////////////////////////////////////////////////////////////////////////
// Recvr class
// - accepts messages from MockChanel for consumption by client
//
class Recvr : public IRecvr
{
public:
	Message getMessage();
	BQueue& queue();
private:
	BQueue recvQ_;
};

Message Recvr::getMessage()
{
	return recvQ_.deQ();
}

BQueue& Recvr::queue()
{
	return recvQ_;
}
/////////////////////////////////////////////////////////////////////////////
// MockChannel class
// - reads messages from Sendr and writes messages to Recvr
//
class MockChannel : public IMockChannel
{
public:
	MockChannel(ISendr* pSendr, IRecvr* pRecvr);
	void start();
	void stop();
	void receiverFromClientToMockChannel();
private:
	std::thread thread_;
	//std::thread threadmock_;
	ISendr* pISendr_;
	IRecvr* pIRecvr_;
	bool stop_ = false;
};

//----< pass pointers to Sender and Receiver >-------------------------------

MockChannel::MockChannel(ISendr* pSendr, IRecvr* pRecvr) : pISendr_(pSendr), pIRecvr_(pRecvr) {}

//----< creates thread to read from sendQ and echo back to the recvQ >-------


void MockChannel::start()
{

	receiverFromClientToMockChannel();
	thread_ = std::thread(
		[&] {
		Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
		Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);
		BQueue& sendQ = pSendr->queue();
		BQueue& recvQ = pRecvr->queue();
		if (pSendr == nullptr || pRecvr == nullptr)
		{	std::cout << "\n  Failed to start Mock Channel\n\n";
			return;}
		MsgClient msgClient;
		while (!stop_)
		{
			Message msg = sendQ.deQ();  
			std::cout << "\n  MockChannel:: A message has been dequed from the shim : \n" << endl<<msg;
			if (msg.find("FileUploadToRepo") != std::string::npos) {
				int pos = msg.find("FileUploadToRepo");
				Message mshbeingsenttoclient = msg.substr(16);

				msgClient.processUploadFromClient(mshbeingsenttoclient);
			}
			else if (msg.find("getcategories") != std::string::npos)
	
				
				msgClient.processGetCategoriesFromClient();
			 else if (msg.find("fetchFileNames") != std::string::npos) 
				
				msgClient.processGetAllFilesFromRepositoryForAParticluarCategorySpecified(msg);
			 else if (msg.find("downloadFileFromServer") != std::string::npos) 
				
				msgClient.processFileDownloadRequest(msg);
			 else if (msg.find("deleteFilesMessage") != std::string::npos) 
			
				msgClient.processDeleteRequestFromClient(msg);
			 else if(msg.find("RunCodeAnalyzerOnServer") != std::string::npos) {
				msgClient.processRunCodeAnalysisFromClient(msg);
			} else {
				recvQ.enQ(msg);
			}
		}
		std::cout << "\n  Mock channel stopping\n\n";});}
//----< signal server thread to stop >---------------------------------------

void MockChannel::stop() { stop_ = true; }

//-----This method opens the socket at client end and downloads js and css files from the Repository
void MockChannel::receiverFromClientToMockChannel()
{
	std::thread theradInitClient([&]() {
		try {
			Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
			if (pSendr != nullptr)
			{	BQueue& sharedQueue = pSendr->queue();
				MsgClient msgClient;
				msgClient.startingClient(sharedQueue);
				msgClient.downloadOfScriptsOnStartUpOfClient();
			}
		}
		catch (std::exception& ex) {
			cout << "Exception caught in MockChannel::receiverFromClientToMockChannel thread" << endl;
			cout << ex.what() << endl;
		}
	});
	theradInitClient.detach();
}

//----< factory functions >--------------------------------------------------

ISendr* ObjectFactory::createSendr() { return new Sendr; }

IRecvr* ObjectFactory::createRecvr() { return new Recvr; }

IMockChannel* ObjectFactory::createMockChannel(ISendr* pISendr, IRecvr* pIRecvr)
{
	return new MockChannel(pISendr, pIRecvr);
}

#ifdef TEST_MOCKCHANNEL

//----< test stub >----------------------------------------------------------

int main()
{
	ObjectFactory objFact;
	ISendr* pSendr = objFact.createSendr();
	IRecvr* pRecvr = objFact.createRecvr();
	IMockChannel* pMockChannel = objFact.createMockChannel(pSendr, pRecvr);
	pMockChannel->start();
	pSendr->postMessage("Hello World");
	pSendr->postMessage("CSE687 - Object Oriented Design");
	Message msg = pRecvr->getMessage();
	std::cout << "\n  received message = \"" << msg << "\"";
	msg = pRecvr->getMessage();
	std::cout << "\n  received message = \"" << msg << "\"";
	pSendr->postMessage("stopping");
	msg = pRecvr->getMessage();
	std::cout << "\n  received message = \"" << msg << "\"";
	pMockChannel->stop();
	pSendr->postMessage("quit");
	std::cin.get();
}
#endif
