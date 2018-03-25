/////////////////////////////////////////////////////////////////////////
// AnalysisExecutive - This package runs a mock of all of pass 1       //
// ver 1.1                                                             //
//                                                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016           //
/////////////////////////////////////////////////////////////////////////
/*
* No real type or dependency analysis is done here, but this mock sets up
* all the event handling and data flow needed, using a ThreadPool and
* Task class to manage a lot of concurrency in a relatively easy way.
*
* This solution uses packages:
* - AnalysisExecutive - Root of the analysis packages with the project's
*                       entry point.
* - ParallelTypeAnal  - ParallelTypeAnalysis, TypeAnal, TableMerge,
*                       and EventHandler classes
* - IFileMgr          - IFileMgr interface
* - FileMgr           - FileMgr, AnalFileEventHandler, AnalDirEventHandler,
*                       and AnalDoneEventHandler classes
* - Task              - Task class
* - ThreadPool        - ThreadPool class
* - Logger            - Logger and StaticLogger<size_t>
* - BlockingQueue     - BlockingQueue<T> used by the three previous packages
* - Utilities         - Converter<T> has toString and toValue methods,
*                       and several other helper classes
*/
/*
 * Maintenance History:
 * --------------------
 * Ver 1.1 : 19 Apr 2016
 * - added spin lock to wait for event propagation in ParllelTypeAnal's TableMerge::Merge()
 * ver 1.0 : 15 Apr 2016
 * - first release
 */
#include "ParallelTypeAnal.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <vector>
#include <iostream>

using namespace Utilities;
using Utils = StringHelper;

class AnalysisExecutive
{
public:
  using Path = std::string;
  using Pattern = std::string;
  using Patterns = std::vector<Pattern>;

  bool parseCommandLine(int argc, char* argv[]);
  void doAnalysis();
private:
  Path path_;
  Patterns patterns_;
};

bool AnalysisExecutive::parseCommandLine(int argc, char* argv[])
{
  std::cout << "\n  commandline: ";
  for (int i = 1; i < argc; ++i)
    std::cout << argv[i] << " ";

  if (argc < 2)
  {
    std::cout << "\n\n  Expected path and patterns on command line";
    return false;
  }
  path_ = argv[1];
  for (int i = 2; i < argc; ++i)
    patterns_.push_back(argv[i]);
  return true;
}

void AnalysisExecutive::doAnalysis()
{
  Show::write("\n  Pass #1 starting");

  std::string msg = "\n  numFilesProcessed will be displayed as zero until FileMgr";
  msg += "\n  notifies TableMerge of the number of files processed\n";
  Show::write(msg);
  ParallelTypeAnalysis exec(path_);
  for (auto pattern : patterns_)
    exec.addPattern(pattern);
  exec.start();
  exec.wait();  // wait for table merge to complete
  Show::write("\n  Pass #1 - Type Analysis complete\n");
  
  // do Pass #2 here

  exec.stop();  // shut down thread pool
  Show::stop();
  DebugLog::stop();
}

#ifdef TEST_ANALYSIS

int main(int argc, char* argv[])
{
  Utils::Title("Testing MockDepAnal");

  Show::attach(&std::cout);
  Show::start();
  DebugLog::attach(&std::cout);

  AnalysisExecutive exec;
  if(!exec.parseCommandLine(argc, argv))
    return 1;
  exec.doAnalysis();
}
#endif
