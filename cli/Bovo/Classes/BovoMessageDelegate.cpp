/*$ @BovoMessageDelegate.cpp - author: Hsiang Chen - license: GPL*/
#include <thread>

#include "BovoMessageDelegate.h"

using namespace std;
using namespace bovo_message;

bool BovoMessageDelegate::connect(const string& uname, const string& hostip, uint32_t port)
{
  return client.connect(uname.c_str(), hostip.c_str(), port);
}

void BovoMessageDelegate::start()
{
  thread obj(_process_msg, this);
  obj.detach(); // hand out thread
}

void BovoMessageDelegate::disconnect()
{
  client.disconnect();
}

bool BovoMessageDelegate::send(BovoMessage& msg)
{
  return client.send(msg);
}

bool BovoMessageDelegate::recv(BovoMessage& msg)
{
  return client.recv(msg);
}

void BovoMessageDelegate::_process_msg(BovoMessageDelegate* bmd)
{
  if (bmd == nullptr) return;

  for ( ; ; ) {
    BovoMessage bm;
    bm.Clear();
    if (bmd->client.recv(bm)) {
      bmd->onMessage(bm);
      if (bm.cmd() == BovoMessage::QUIT) break;
    }
  }
}

// end
