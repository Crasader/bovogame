/*$ @BovoMessageCtrl.h - author: Hsiang Chen - license: GPL*/
#ifndef __BOVOMESSAGECTRL_H__
#define __BOVOMESSAGECTRL_H__

#include <string>

#include "msg.pb.h"
#include "BovoSocket.h"

using namespace bovo_message;

class BovoMessageCtrl : public BovoSocket {
public:
  bool connect(const char* unam, const char* ip, uint32_t port);
  void disconnect();
  bool send(BovoMessage& msg);
  bool recv(BovoMessage& msg);
};

#endif
