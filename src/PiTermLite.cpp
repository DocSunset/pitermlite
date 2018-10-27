#include "PiTermLite.h"
#include <thread>
#include <chrono>
#include <string>
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
  bool initbuffersuccess = buffer.InitPingPong(BUFFERSIZE);
  if (!initbuffersuccess)
  {
    std::cerr << "Error initializing memory for buffers" << std::endl;
    return 1;
  }

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
  
  while (running) running = runflag;

  readThread.join();
  outputThread.join();
  commandThread.join();
  
  return 0;
}

void printHelp(char ** argv)
{
  std::cout << "Usage: " << argv[0] << " <tracker name>\n"
            << "or     " << argv[0] << " <serial port file>\n"
            << "A simple command line interface for Polhemus trackers\n"
            << "\n"
            << "To connect over RS232, specify the /dev file for the port\n"
            << "To connect over USB, specify one of the following names:\n";
  
  int longestname = 0;
  for (int i = 0; i < NUM_SUPP_TRKS; ++i)
  {
    auto len = std::strlen(trackerLongNames[i]);
    if (len > longestname) longestname = len;
  }
  for (int i = 0; i < NUM_SUPP_TRKS; ++i)
  {
    std::cout << "    " << trackerLongNames[i];
    for (int s = 0; s < longestname - std::strlen(trackerLongNames[i]); ++s)
        std::cout << " ";
    std::cout << "    or    " << trackerShortNames[i] << "\n";
  }
  std::cout << "\n" << version << "\n"
            << "Written by Travis West at IDMIL, McGill University, Montreal QC Canada\n"
            << "Based on Tracker Terminal (PiTerm) by James C. Farr of Polhemus Inc.\n";
  std::cout << std::flush;
}

bool argExists(char** begin, char** end, const std::string& arg)
{
  return std::find(begin, end, arg) != end;
}

bool parseArgs(int argc, char** argv, ConnectionInfo* info)
{
  char** begin = argv;
  char** end = argv+argc;
  if (argExists(begin, end, "-h"))
  {
    printHelp(argv);
    return false;
  }
  else if (argc == 2)
  {
    info->connectionType = RS232_CNX;
    info->port = argv[1];
    return true;
  }
  else
  {
    for (int i = 0; i < NUM_SUPP_TRKS; ++i)
    {
      if (argExists(begin, end, trackerLongNames[i]) 
          || argExists(begin, end, trackerShortNames[i]))
      {
        info->connectionType = USB_CNX;
        info->usbParams = usbTrkParams[i];
        return true;
      }
    }
  }
  // if execution reaches this point, no valid args were found
  printHelp(argv);
  return false;
}

bool connectTracker(PiTracker& tracker, const ConnectionInfo& connectionInfo)
{
  int returncode;
  if (connectionInfo.connectionType == USB_CNX)
    returncode = tracker.UsbConnect(connectionInfo.usbParams);
  else
    returncode = tracker.Rs232Connect(connectionInfo.port);
  bool connectionSuccess = returncode == 0;
  return connectionSuccess;
}

void waitForResponseFromTracker(PiTracker& tracker)
{
  BYTE localBuffer[BUFFERSIZE];
  int bytesReceived = 0;
  do { // keep sending an invalid command until a response is received
    tracker.WriteTrkData((void *)"\r", 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    bytesReceived = tracker.ReadTrkData(localBuffer, BUFFERSIZE);
    } while (bytesReceived == 0);
}

void readFromTracker(PiTracker& tracker, PingPong& buffer, const bool& running)
{
  BYTE localBuffer[BUFFERSIZE];
  int bytesReceived = 0;
  int bytesWritten;
  while (running) 
  {
    // read from tracker
    bytesReceived = tracker.ReadTrkData(localBuffer, BUFFERSIZE);
    // write to ping pong buffer
    if (0 < bytesReceived && bytesReceived < BUFFERSIZE)
    {
      localBuffer[bytesReceived] = 0x00; // null terminate localBuffer
      do {
        bytesWritten = buffer.WritePP(localBuffer, bytesReceived);
        std::this_thread::sleep_for(std::chrono::seconds(1));
      } while (bytesWritten == 0);
    }
  }
}

void writeToCommandLine(PingPong& buffer, const bool& running)
{
  BYTE localBuffer[BUFFERSIZE];
  int bytesReceived = 0;
  while (running) 
  {
    bytesReceived = buffer.ReadPP(localBuffer);
    if (bytesReceived)
    {
      for (int i = 0; i < bytesReceived; ++i) 
        std::cout << localBuffer[i];
      std::cout << std::flush;
    }
  }
}

void parseCommandLineInput(char* inputBuffer, int bytesReceived, BYTE* parsedBuffer)
{
    for (int i = 0; i < bytesReceived; ++i)
    {
      switch (inputBuffer[i])
      {
      case '^':
        if (isalpha(inputBuffer[i+1]))
        {
          parsedBuffer[i] = (BYTE)(tolower(inputBuffer[i+1]) - 0x60);
          ++i;
        }
        --bytesReceived;
        break;
      case '@':
        parsedBuffer[i] = 0x00;
        break;
      default:
        parsedBuffer[i] = (BYTE)inputBuffer[i];
        break;
      }
    }
    parsedBuffer[bytesReceived] = '\r';
}

void readFromCommandLine(PiTracker& tracker, const bool& running)
{
  char localBuffer[BUFFERSIZE];
  int bytesReceived = 0;
  BYTE parsedBuffer[BUFFERSIZE];
  while(running) 
  {
    std::cin.getline(localBuffer, BUFFERSIZE);
    bytesReceived = std::cin.gcount();
    parseCommandLineInput(localBuffer, bytesReceived, parsedBuffer);
    tracker.WriteTrkData(parsedBuffer,bytesReceived); 
  }
}
