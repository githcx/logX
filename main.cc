#include "blocking_queue.hpp"
#include "object_pool.hpp"

#include <string>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <ctime>
#include <unistd.h>
using namespace std;

#define BUF_SIZE (10 * 1024 * 1024)

class LogBuf {
public:
  LogBuf() : capacity(BUF_SIZE), len(0) {
    bufTotal++;
  }
  ~LogBuf() {
    bufTotal--;
  }

  bool Write(const string& s) {
    if (len + s.length() > capacity) {
      return true;
    }

    memcpy(buf + len, s.c_str(), s.length());
    len += s.length();

    return false;
  }

  LogBuf* Reset() {
    len = 0;
    return this;
  }

// private:
  static int bufTotal;
  int  serial; 
  int  id;
  int  capacity;
  int  len;
  char buf[BUF_SIZE];
};

int LogBuf::bufTotal = 0;

void testLogX() {
    
  Pool<LogBuf> bufPool;

  string content(BUF_SIZE/10, 'a');
  BlockQueue<LogBuf*> channel;

  // log mem writer
  thread memWriter( [&](){
    LogBuf* buffer = NULL;
    int id = 1;
    while(true) {
      if (buffer == NULL) {
        buffer = bufPool.Get();
        printf("get buffer #%d\n", buffer->serial);
        buffer->id = id;
        id++;
      }

      if (buffer->Write(content)) {
        channel.PushBack(buffer);
        printf("completed buffer #%d size = %d\n", buffer->id, buffer->len);
        buffer = NULL;
        continue;
      }
      usleep(100 * 1000);
    }
  });

  // log file writer
  thread fileWriter( [&](){
    LogBuf* buffer = NULL;

    while(true) {
      LogBuf* buffer = NULL;
      channel.PopFront(buffer);
      printf("received buffer#%d with size = %d\n", buffer->id, buffer->len);
      printf("flush buffer #%d to disk size = %d\n", buffer->id, buffer->len);

      bufPool.Put(buffer->Reset());
    }
  });

  while(true) {
    printf("buf total = %d\n", LogBuf::bufTotal);
    sleep(1);
  }

  memWriter.join();
  fileWriter.join();
}

int main() {
  testLogX();
  return 0;
}
