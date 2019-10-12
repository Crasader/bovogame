/* $ @BovoSocketDelegate.h - author: Hsiang Chen - license: GPL */
#ifndef __BOVOMESSAGEDELEGATE_H__
#define __BOVOMESSAGEDELEGATE_H__

#include "BovoMessageCtrl.h"
#include "msg.pb.h"

class BovoMessageDelegate {
public:
  bool connect(const string& uname, const string& hostip, uint32_t port);
  void start();
  void disconnect();
  bool send(bovo_message::BovoMessage& msg);
  bool recv(bovo_message::BovoMessage& msg);
protected:
  virtual void onMessage(const bovo_message::BovoMessage& msg) = 0;
private:
  static void _process_msg(BovoMessageDelegate* bmd);
  BovoMessageCtrl client;
};

#endif
