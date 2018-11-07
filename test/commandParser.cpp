#include "acutest.h"
#include "PiTermLite.h"
#include <string>

void parser_caret(void)
{
  int bytesReceived = 3;

  char* buffer = new char [bytesReceived];
  buffer[0] = '^';
  buffer[1] = 'a';
  buffer[2] = 0;

  int bytesToSend = parseCommandLineInput(buffer);

  TEST_CHECK_(buffer[0] == 0x01, "parser produced %d instead of %d",
      buffer[0], 0x01);
  TEST_CHECK_(buffer[1] == '\r', "parser produced %d instead of %d",
      buffer[1], '\r');
  TEST_CHECK_(bytesToSend == 2, 
      "wrong number of bytesToSend, got %d, expected %d", 
      bytesToSend, 2);
}

void parser_at(void) 
{
  int bytesReceived = 3;

  char* buffer = new char [bytesReceived];
  buffer[0] = '@';
  buffer[1] = 'a';
  buffer[2] = 0;

  int bytesToSend = parseCommandLineInput(buffer);

  TEST_CHECK_(buffer[0] == 0x00, "parser produced %d instead of %d",
      buffer[0], 0x00);
  TEST_CHECK_(buffer[1] == 'a', "parser produced %d instead of %d",
      buffer[1], 'a');
  TEST_CHECK_(buffer[2] == '\r', "parser produced %d instead of %d",
      buffer[2], '\r');
  TEST_CHECK_(bytesToSend == 3, 
      "wrong number of bytesToSend, got %d, expected %d", 
      bytesToSend, 3);
}

void parser_passthrough(void) 
{
  int bytesReceived = 3;

  char* buffer = new char [bytesReceived];
  buffer[0] = 'l';
  buffer[1] = '1';
  buffer[2] = 0;

  int bytesToSend = parseCommandLineInput(buffer);

  TEST_CHECK_(buffer[0] == 'l', "parser produced %d instead of %d",
      buffer[0], 0x01);
  TEST_CHECK_(buffer[1] == '1', "parser produced %d instead of %d",
      buffer[1], '1');
  TEST_CHECK_(buffer[2] == '\r', "parser produced %d instead of %d",
      buffer[1], '\r');
  TEST_CHECK_(bytesToSend == 3, 
      "wrong number of bytesToSend, got %d, expected %d", 
      bytesToSend, 3);
}

TEST_LIST = {
  {"caret", parser_caret},
  {"at sign", parser_at},
  {"passthrough", parser_passthrough}
};
