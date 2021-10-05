#include "PiTermLite.h"
#include <thread>
#include <iostream>
#include <signal.h>

bool runflag = true;

void siginthandler(int sig) { runflag = false; }

int main(int argc, char** argv)
{
  signal(SIGINT, siginthandler);

  ConnectionInfo connectionInfo;
  bool argsuccess = parseArgs(argc, argv, &connectionInfo);
  if (!argsuccess)
  {
    // parseArgs prints its own helpful message
    return 1;
  }

  PiTracker tracker;
  bool connectionsuccess = connectTracker(tracker, connectionInfo);
  if (!connectionsuccess)
  {
    std::cerr << "Error connecting to tracker." << std::endl;
    return 1;
  }

  PingPong buffer; 
  bool initbuffersuccess = buffer.InitPingPong(BUFFERSIZE) == 0;
  if (!initbuffersuccess)
  {
    std::cerr << "Error initializing memory for buffers" << std::endl;
    return 1;
  }

  std::cout << "Successfully connected to tracker\n" << std::flush;

  std::cin.tie(nullptr); // so that cin won't try to flush cout
  bool running = runflag; // local copy is made so that it can be captured

  std::thread readThread([&tracker, &buffer, &running]
  {
    waitForResponseFromTracker(tracker);
    readFromTracker(tracker, buffer, running);
  });
  
  std::thread outputThread([&buffer, &running]
  {
    writeToCommandLine(buffer, running);
  });

  std::thread commandThread([&tracker, &running]
  {
    readFromCommandLine(tracker, running);
  });
  
  while (running)
  {
    running = runflag;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  readThread.join();
  outputThread.join();
  commandThread.join();
  
  return 0;
}
