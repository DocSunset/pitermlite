#include "PiTermLite.h"
#include <thread>
#include <chrono>
#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
const char* version = "version 0.1";

// polhemus tracker names
const char* trackerLongNames[NUM_SUPP_TRKS] =
{
  "high_speed_liberty",
  "liberty",
  "high_speed_patriot",
  "patriot",
  "fastrak",
  "fastrak3"
};

const char* trackerShortNames[NUM_SUPP_TRKS] =
{
  "hsl",
  "l",
  "hsp",
  "p",
  "f",
  "f3"
};

// usb vid/pids for Polehemus trackers
const USB_PARAMS usbTrkParams[NUM_SUPP_TRKS] =
{
  {0x0f44,0xff20,0x04,0x88},  // Lib HS
  {0x0f44,0xff12,0x02,0x82},  // Lib
  {0X0f44,0xef20,0x04,0x88},  // Patriot HS
  {0x0f44,0xef12,0x02,0x82},  // Patriot
  {0x0f44,0x0002,0x02,0x82},	// Fastrak
  {0x0f44,0x0003,0x02,0x84}   // Fastrak 3
};

bool argExists(char** begin, char** end, const std::string& arg)
{
  return std::find(begin, end, arg) != end;
}

bool parseArgs(int argc, char** argv, ConnectionInfo* info)
{
  char** begin = argv;
  char** end = argv+argc;
  if (argc == 2)
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
    if (argExists(begin, end, "-h"))
    {
      printHelp(argv);
      return false;
    }
    else
    {
      info->connectionType = RS232_CNX;
      info->port = argv[1];
      return true;
    }
  }
  else
  {
    // if execution reaches this point, args were invalid
    printHelp(argv);
    return false;
  }
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
    tracker.WriteTrkData((void *)'\r', 1);
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
        std::cout << localBuffer << std::flush;
    }
  }
}

char alphaToCtrlCode(const char& alpha) { return tolower(alpha - 0x60); }

int parseCommandLineInput(char* buffer)
{
  int read = 0;
  int write = 0;
  for (/*read, write*/; buffer[read] != 0x00; ++read, ++write)
  {
    switch (buffer[read])
    {
    case '^':
      if (!(buffer[read+1] != 0x00 && isalpha(buffer[read+1]))) break;
      ++read;
      buffer[write] = alphaToCtrlCode(buffer[read]);
      break;
    case '@':
      buffer[write] = 0x00;
      break;
    default:
      buffer[write] = buffer[read];
      break;
    }
  }
  buffer[write] = '\r';
  ++write;
  return write;
}

void readFromCommandLine(PiTracker& tracker, const bool& running)
{
  char localBuffer[BUFFERSIZE];
  int bytesToSend = 0;
  while(running) 
  {
    std::cin.getline(localBuffer, BUFFERSIZE);
    bytesToSend = parseCommandLineInput(localBuffer);
    tracker.WriteTrkData((void*)localBuffer, bytesToSend); 
  }
}
