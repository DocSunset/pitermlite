#include "acutest.h"
#include "PiTermLite.h"
#include <string>

void parseArgs_validname(void)
{
  std::string argv0 = "pitermlite";
  std::string argv1 = "hsl";
  char** argv = new char *[2];
  argv[0] = new char[argv0.length() + 1];
  argv[1] = new char[argv1.length() + 1];
  strcpy(argv[0], argv0.c_str());
  strcpy(argv[1], argv1.c_str());

  ConnectionInfo info;
  bool argsuccess = parseArgs(2, argv, &info);

  TEST_CHECK_(argsuccess, "parseArgs failed unexpectedly");
  TEST_CHECK_(info.connectionType == USB_CNX, 
      "parseArgs parsed wrong connection type, got %d, expected %d", 
      info.connectionType, USB_CNX);
  TEST_CHECK_(info.usbParams.pid == 0xff20, 
      "parseArgs got incorrect usbParams. Got vid %d, expected %d",
      info.usbParams.vid, 0xff20);
}

void parseArgs_validport(void)
{
  std::string argv0 = "pitermlite";
  std::string argv1 = "/dev/serialwhatever";
  char** argv = new char *[2];
  argv[0] = new char[argv0.length() + 1];
  argv[1] = new char[argv1.length() + 1];
  strcpy(argv[0], argv0.c_str());
  strcpy(argv[1], argv1.c_str());

  ConnectionInfo info;
  bool argsuccess = parseArgs(2, argv, &info);

  TEST_CHECK_(argsuccess, "parseArgs failed unexpectedly");
  TEST_CHECK_(info.connectionType == RS232_CNX,
      "parseArgs parsed wrong connection type, got %d, expected %d", 
      info.connectionType, RS232_CNX);
  TEST_CHECK_(std::string(info.port) == argv1,
      "parseArgs parsed port incorrectly, got %s instead of %s",
      info.port, argv1.c_str());
}

TEST_LIST = {
  {"valid name", parseArgs_validname},
  {"valid port", parseArgs_validport}
};
