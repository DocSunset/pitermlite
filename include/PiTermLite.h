#ifndef PITERMLITE_H
#define PITERMLITE_H

#include "PiTracker.h"
#include "PingPong.h"
#define BUFFERSIZE 1000
const char* version = "version 0.1";

enum TrackerType
{
  TRKR_LIB_HS, 
  TRKR_LIB, 
  TRKR_PAT_HS, 
  TRKR_PAT, 
  TRKR_FT, 
  TRKR_FT3, 
  NUM_SUPP_TRKS
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

struct ConnectionInfo
{
  int connectionType;
  USB_PARAMS usbParams;
  char* port;
};

void printHelp(char** argv);
bool parseArgs(int argc, char** argv, ConnectionInfo* info);
bool connectTracker(PiTracker& tracker, const ConnectionInfo& connectionInfo);
void waitForResponseFromTracker(PiTracker& tracker);
void readFromTracker(PiTracker& tracker, PingPong& buffer, const bool& running);
void writeToCommandLine(PingPong& buffer, const bool& running);
void parseCommandLineInput(char* inputBuffer, int bytesReceived, BYTE* parsedBuffer);
void readFromCommandLine(PiTracker& tracker, const bool& running);

#endif
