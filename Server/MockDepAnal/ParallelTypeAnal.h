#pragma once
/////////////////////////////////////////////////////////////////////////
// ParallelTypeAnal.h - This package builds a mock of all of pass 1    //
//                                                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016           //
/////////////////////////////////////////////////////////////////////////
/*
 * No real type analysis is done here, but this mock sets up all the 
 * event handling and data flow needed, using a ThreadPool and Task class
 * to manage a lot of concurrency in a relatively easy way.
 *
 * This package depends on the packages:
 * - ParallelTypeAnal - ParallelTypeAnalysis, TypeAnal, TableMerge, 
 *                      and EventHandler classes
 * - IFileMgr         - IFileMgr interface
 * - FileMgr          - FileMgr, AnalFileEventHandler, AnalDirEventHandler, 
 *                      and AnalDoneEventHandler classes
 * - Task             - Task class
 * - ThreadPool       - ThreadPool class
 * - Logger           - Logger and StaticLogger<size_t>
 * - BlockingQueue    - BlockingQueue<T> used by the three previous packages
 * - Utilities        - Converter<T> has toString and toValue methods, 
 *                      and several other helper classes
*/
#include <string>
#include <iostream>
#include <sstream>
#include <future>
#include <atomic>
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../FileMgr/IFileMgr.h"
#include "../FileMgr/FileMgr.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include "../Tasks/Task.h"

using Show = StaticLogger<0>;
using DebugLog = StaticLogger<1>;

/////////////////////////////////////////////////////////////////////
// TypeAnal class
// - Responsible for analyzing types and functions in a single file.
// - For complete solution this becomes the package TypeAnal
//
class TypeAnal
{
public:
  using File = std::string;
  using Table = std::string;  // simple for demo

  TypeAnal(BlockingQueue<Table>& tableQ) : tableQ_(tableQ) {}
  void doTypeAnal(const File& filename);

private:
  BlockingQueue<Table>& tableQ_;
};

/////////////////////////////////////////////////////////////////////
// TableMerge class
// - Responsible for merging type tables as they arrive in table queue
// - For complete solution this becomes package TableMerge
//
class TableMerge;

class MergeDoneEventHandler : public FileManager::IDoneEventHandler
{
public:
  MergeDoneEventHandler(TableMerge* pTM) : pTM_(pTM) {}
  void execute(size_t numFilesProcessed);
private:
  TableMerge* pTM_;
};

class TableMerge
{
public:
  TableMerge(BlockingQueue<TypeAnal::Table>& tableQ);
  void registerForDone(FileManager::IFileMgr* pFileMgr);
  TypeAnal::Table merge();
  void done(size_t numFilesProcessed);
  size_t wait();
private:
  void showProgress(std::atomic<size_t>& numFilesProcessed, size_t numTablesProcessed);
  BlockingQueue<TypeAnal::Table>& tableQ_;
  TypeAnal::Table merged_;
  size_t numTablesCreated = 0;
  FileManager::IFileMgr* pFileMgr_;
  MergeDoneEventHandler dnEvt_;
  std::atomic<size_t> numFilesProcessed_;  // shared by FileMgr and TableMerge
  size_t numTablesProcessed_ = 0;
  std::future<size_t> future_;
  std::promise<size_t> promise_;
};

/////////////////////////////////////////////////////////////////////
// Analysis Event Handlers

class ParallelTypeAnalysis;

class AnalFileEventHandler : public FileManager::IFileEventHandler
{
public:
  AnalFileEventHandler(ParallelTypeAnalysis* pExec) : pExec_(pExec) {}
  void execute(const std::string& file);
private:
  ParallelTypeAnalysis* pExec_;
};

class AnalDirEventHandler : public FileManager::IDirEventHandler
{
public:
  AnalDirEventHandler(ParallelTypeAnalysis* pExec) : pExec_(pExec) {}
  void execute(const std::string& dir);
private:
  ParallelTypeAnalysis* pExec_;
};

class AnalDoneEventHandler : public FileManager::IDoneEventHandler
{
public:
  AnalDoneEventHandler(ParallelTypeAnalysis* pExec) : pExec_(pExec) {}
  void execute(size_t numFilesProcessed);
private:
  ParallelTypeAnalysis* pExec_;
};

/////////////////////////////////////////////////////////////////////
// ParallelTypeAnalysis class
// - Responsible for creating FileManager, TypeAnal, and TableMerge
// - Runs FileMgr::search(), TypeAnal::doTypeAnal(), and TableMerge::merge()
//   on ThreadPool threads, using the Task class
//
class ParallelTypeAnalysis
{
public:
  using File = std::string;
  using Path = std::string;
  using Pattern = std::string;
  using CallObj = std::function<bool()>;

  ParallelTypeAnalysis(const Path& path);
  void addPattern(const Pattern& pattern);
  void start();
  void stop();
  void wait();
  void file(const File& file);
  void dir(const Path& path);
  void done(size_t numFilesProcessed);
private:
  TypeAnal ta_;
  TableMerge tm_;
  BlockingQueue<TypeAnal::Table> tableQueue_;
  Path path_;
  Task<8> task_;
  FileManager::IFileMgr* pFileMgr_;
  AnalFileEventHandler fevt_;
  AnalDirEventHandler devt_;
  AnalDoneEventHandler dnEvt_;
  bool fileSearchDone = false;
  CallObj tmco_;  // table merge
  CallObj taco_;  // type anal
  CallObj fsco_;  // file search
};
