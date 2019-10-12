#include "BovoMessageCtrl.h"

#define BUFF_SIZE 256

using namespace std;

bool BovoMessageCtrl::connect(const char* unam, const char* ip, uint32_t port)
{
  bool rev = false;
  
  if (BovoSocket::socket(0x11) != -1 && BovoSocket::connect(ip, port) != -1) {
    BovoMessage bm;
    bm.set_cmd(BovoMessage::JOIN);
    bm.set_name(unam);
    rev = send(bm) > 0;
  }

  return rev;
}

void BovoMessageCtrl::disconnect()
{
  BovoMessage bm;
  bm.set_cmd(BovoMessage::QUIT);
  send(bm);
  close();
}

bool BovoMessageCtrl::send(BovoMessage& msg)
{
  string str;

  if (msg.SerializeToString(&str)) {
    ssize_t len = msg.ByteSize();
    if (BovoSocket::send(str.data(), len) > 0)
      return true;
    /*
      printf("len:%d\n",len);
   printf("%02x %02x %02x %02x %02x %02x %02x %02x - %s\n", str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7], str.c_str());
    } */
  }

  return false;
}

bool BovoMessageCtrl::recv(BovoMessage& msg)
{
  char* ptr = new char[BUFF_SIZE * 2];
  ssize_t len;
  
  if (ptr == nullptr) return false;

  bool okay = false;

  if ((len = BovoSocket::recv(ptr, BUFF_SIZE * 2)) > 0) {
    string str(ptr, len);
    if (msg.ParseFromString(str))
      okay = true;
  }

  delete[] ptr;

  return okay;
}

// end
