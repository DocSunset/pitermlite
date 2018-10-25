#include "PiTermLite.h"
#include <thread>
#include <iostream>

bool runflag = false;
// signal handler to set running to false on ctrl-c

int main(int argc, char** argv)
{
  ConnectionInfo connectionInfo = parseArgs(argc, argv);

  PiTracker tracker;
  bool connectionSuccess = connectTracker(tracker, connectionInfo);
  
  if (!connectionSuccess)
  {
    std::cout << "Error connecting to tracker." << std::endl;
    return 1;
  }

  PingPong buffer; 
  buffer.InitPingPong(BUFFERSIZE);
  buffer.ClearBuffers();
  std::cin.tie(nullptr); // so that cin won't try to flush cout
  runflag = true;
  bool running = runflag; // local copy is made so that it can be captured

  std::thread readThread([&tracker, &buffer, &running]
  {
  while (running) {
    // read from tracker
    // write to buffer
  }});
  
  std::thread outputThread([&buffer, &running]
  {
  while (running) {
    // read from buffer
    // write to stdcout
  }});
  
  while(running) {
    // read from stdcin
    // parse it
    // send it to tracker
    running = runflag; // update local copy of runflag in case of ctrl-c
  }
  
  readThread.join();
  outputThread.join();
  
  return 0;
}

ConnectionInfo parseArgs(int argc, char** argv)
{
  return ConnectionInfo();
}

bool connectTracker(PiTracker& tracker, const ConnectionInfo& connectionInfo)
{
  bool connectionSuccess = false;
  if (connectionInfo.connectionType == USB_CNX)
    connectionSuccess = tracker.UsbConnect(connectionInfo.usbParams);
  else
    connectionSuccess = tracker.Rs232Connect(connectionInfo.port);
  return connectionSuccess;
}
