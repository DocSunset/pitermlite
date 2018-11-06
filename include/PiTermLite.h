#ifndef PITERMLITE_H
#define PITERMLITE_H

#include "PiTracker.h"
#include "PingPong.h"
#define BUFFERSIZE 1000
extern const char* version;

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

// polhemus tracker names
extern const char* trackerLongNames[NUM_SUPP_TRKS];
extern const char* trackerShortNames[NUM_SUPP_TRKS];

// usb vid/pids for Polehemus trackers
extern const USB_PARAMS usbTrkParams[NUM_SUPP_TRKS];

struct ConnectionInfo
{
  int connectionType;
  USB_PARAMS usbParams;
  char* port;
};

bool parseArgs(int argc, char** argv, ConnectionInfo* info);
void printHelp(char** argv);
bool connectTracker(PiTracker& tracker, const ConnectionInfo& connectionInfo);
void waitForResponseFromTracker(PiTracker& tracker);
void readFromTracker(PiTracker& tracker, PingPong& buffer, const bool& running);
void writeToCommandLine(PingPong& buffer, const bool& running);
int parseCommandLineInput(char* buffer);
void readFromCommandLine(PiTracker& tracker, const bool& running);

#endif
