#include "acutest.h"
#include "PiTermLite.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>

bool compareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
  std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);

  if (f1.fail() || f2.fail()) {
    return false; //file problem
  }

  if (f1.tellg() != f2.tellg()) {
    return false; //size mismatch
  }

  //seek back to beginning and use std::equal to compare contents
  f1.seekg(0, std::ifstream::beg);
  f2.seekg(0, std::ifstream::beg);
  return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()));
}

inline bool fileExists (const std::string& name) {
  std::ifstream f(name.c_str());
  return f.good();
}

void writethread(void)
{

  std::ofstream out("test/writetestout.txt");
  std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
  std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

  std::ifstream in("test/writetestin.txt");

  PingPong buffer;
  bool initbuffersuccess = buffer.InitPingPong(1000) == 0;
  TEST_CHECK_(initbuffersuccess, "Failed to initialize pp buffer");

  bool running = true;
  std::thread outputThread([&buffer, &running]
  {
    writeToCommandLine(buffer, running);
  });

  char localBuffer[1000];
  while (in.getline(localBuffer,999)) {
    int bytesReceived = in.gcount();
    int bytesWritten = 0;
    do {
      bytesWritten = buffer.WritePP((BYTE*)localBuffer, bytesReceived);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (bytesWritten == 0);
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
  running = false;
  outputThread.join();

  std::cout.rdbuf(coutbuf);
  out.close();

  TEST_CHECK_(
    compareFiles("test/writetestexpect.txt", "test/writetestout.txt"),
    "Write test files are not equivalent.");
}

TEST_LIST = {
  {"write thread", writethread}
};
