// $@ BovoLayer.cpp

#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "BovoLayer.h"

USING_NS_CC;

using namespace std;
using namespace bovo_message;

BovoLayer::BovoLayer(Layer* lc) : busy(true), connected(false), oppo(nullptr), mesg(nullptr), spt_self(nullptr), spt_oppo(nullptr)
{
  this->lc = lc;
  this->setTouchEnabled(true);
  this->spt_self = Sprite::create("MarkCircle.png");
  this->spt_oppo = Sprite::create("MarkCross.png");
}

BovoLayer::~BovoLayer()
{
  disconnect();
}

BovoLayer* BovoLayer::create()
{
  return new BovoLayer(Layer::create());
}

bool BovoLayer::initLayer(const Vec2& ori, const Size& siz, Label* oppo, Label* mesg)
{
  origin = ori;
  size = siz;

  this->oppo = oppo;
  this->mesg = mesg;

  getCellSize(nullptr, nullptr);
  getCoordinateSize(nullptr);

  if (size.width < coor.width) return false;
  if (size.height < coor.height) return false;

  calOriginCoordinate(nullptr);

  auto board = DrawNode::create(1.0);

  if (board != nullptr) {
    board->drawSolidRect(origin, Vec2(origin.x + coor.width, origin.y + coor.height), Color4F(1, 1, 1, 1));

    uint32_t i;

    for (i = origin.y; i < origin.y + coor.height; i += cell.height) {
      board->drawLine(Vec2(origin.x, i), Vec2(origin.x + coor.width, i), Color4F(0, 0, 0, 1));
    }

    for (i = origin.x; i < origin.x + coor.width; i += cell.width) {
      board->drawLine(Vec2(i, origin.y), Vec2(i, origin.y + coor.height), Color4F(0, 0, 0, 1));
    }

    this->addChild(board);

    return true;
  }

  return false;
}

bool BovoLayer::connect(const string& uiaddr)
{
  auto len = uiaddr.size();

  if (len <= 0) return false;

  busy = true;

  disconnect();

  char* ptr = new char[len];

  if (ptr != nullptr) {
    strcpy(ptr, uiaddr.c_str());
    //
    // username@hostip:port
    char* p = std::strchr(ptr, '@');
    if (p < ptr + len) {
      string uname = string(ptr, p - ptr);
      string hostip = strtok(p + 1, ":");
      string port = strtok(nullptr, ":");
      if (uname.size() > 0 && hostip.size() > 0 && port.size() > 0) {
        connected = BovoMessageDelegate::connect(username = uname, hostip, std::atoi(port.c_str()));
      }
    }
    delete[] ptr;
  }

  if (connected)
    start();

  busy = false;

  return connected;
}

void BovoLayer::disconnect()
{
  if (connected) {
    BovoMessageDelegate::disconnect();
    connected = false;
    busy = true;
  }
}

////////////////////////////////////////////////////

void BovoLayer::onTouchesCancelled(const vector<Touch*>& touches, Event* unused_event) {}
void BovoLayer::onTouchesBegan(const vector<Touch*>& touches, Event* unused_event) {}
void BovoLayer::onTouchesMoved(const vector<Touch*>& touches, Event* unused_event) {}
void BovoLayer::onTouchesEnded(const vector<Touch*>& touches, Event* unused_event)
{
  if ((busy && connected) || !connected) {
    return;
  }

  for (auto& touch : touches) {
    if (touch == nullptr) break;
    
    Vec2 loc = touch->getLocation();  UiSize norm;
    
    if (getNormByPixelPoint(loc, norm)) {
      if (norm.x < MATRIX_COL && norm.y < MATRIX_ROW) {
        uint32_t idx = normToInt(norm);
        if (matrix.find(idx) == matrix.end()) {
          BovoMessage bm;
          ////
          bm.Clear();
          bm.set_cmd(BovoMessage::PUT);
          bm.set_markstyle(markstyle);
          bm.set_status(BovoMessage::OKAY);
          bm.set_name(username);
          bm.set_y(norm.x);
          bm.set_x(norm.y);
          ////
          if (BovoMessageDelegate::send(bm) > 0)
            busy = true;
        }
      }
    }
  }
}

////////////////////////////////////////////////////

void BovoLayer::onMessage(const BovoMessage& msg)
{
  switch (msg.cmd()) {
    case BovoMessage::STYLE:
      if (markstyle == 0) {
        markstyle = msg.markstyle();
      }
      if (msg.has_name() && oppo != nullptr) {
        oppo->setString(msg.name());
      }
      break;
    case BovoMessage::PUT:
      UiSize norm;
      norm.x = msg.x();
      norm.y = msg.y();
      setCell(norm, msg.markstyle());
      busy = false;
      break;
    case BovoMessage::NONE:
      if (msg.status() == BovoMessage::OKAY) {
        UiSize norm;
        norm.x = msg.x();
        norm.y = msg.y();
        setCell(norm, markstyle);
        busy = false;
      }
      break;
    case BovoMessage::QUIT:
      if (msg.status() == BovoMessage::WON || msg.status() == BovoMessage::LOST) {
        UiSize norm;
        norm.x = msg.x();
        norm.y = msg.y();
        setCell(norm, msg.markstyle());
        if (mesg != nullptr) {
          if (msg.status() == BovoMessage::WON)
            mesg->setString("You won!");
          else
            mesg->setString("You lost!");
        }
      } else {
        if (mesg != nullptr)
          mesg->setString("Offline");
      }
      disconnect();
      busy = true;
      break;
  }
}

////////////////////////////////////////////////////

bool BovoLayer::setCell(const UiSize& norm, uint32_t markstyle)
{
  uint32_t idx = normToInt(norm);

  auto node = matrix.find(idx);

  if (node != matrix.end()) return false;

  Vec2 coor;

  if (getCoordinateByNorm(norm, coor)) {
    auto sp = Sprite::create(markstyle != 1 ? "MarkCircle.png" : "MarkCross.png");
    if (sp != nullptr) {
      sp->setPosition(coor);
      this->addChild(sp);
      matrix.insert(make_pair(idx, sp));
      return true;
    }
  }

  return false;
}

uint32_t BovoLayer::normToInt(const UiSize& norm)
{
  return ((norm.x & 0xff) << 8) | (norm.y & 0xff);
}

void BovoLayer::calOriginCoordinate(Vec2* ori)
{
  origin.x += size.width / 2 - coor.width / 2; 
  origin.y += size.height / 2 - coor.height / 2;

  if (ori != nullptr)
    *ori = origin;
}

bool BovoLayer::getCoordinateSize(Size* to)
{
  if (cell.width <= 0 || cell.height <= 0)
    getCellSize(nullptr, nullptr);

  coor.width = cell.width * MATRIX_COL;
  coor.height = cell.height * MATRIX_ROW;

  if (to != nullptr)
    *to = coor;

  return true;
}

bool BovoLayer::getCellSize(Sprite* spt, Size* to)
{
  if (spt == nullptr)
    spt = spt_self;
  if (spt != nullptr) {
    cell = spt->getContentSize();
    if (to != nullptr)
      *to = cell;
  } else return false;

  return true;
}


bool BovoLayer::getCoordinateByNorm(const UiSize& from, Vec2& to)
{
  if (from.x > MATRIX_COL) return false;
  if (from.y > MATRIX_ROW) return false;

  to.x = origin.x + from.x * cell.width + cell.width / 2;
  to.y = origin.y + from.y * cell.height + cell.height / 2;

  return true;
}

bool BovoLayer::getNormByPixelPoint(const Vec2& from, UiSize& to)
{
  if (from.x > origin.x + coor.width) return false;
  if (from.x < origin.x) return false;
  if (from.y > origin.y + coor.height) return false;
  if (from.y < origin.y) return false;

  to.x = (uint32_t) (from.x - origin.x) / cell.width;
  to.y = (uint32_t) (from.y - origin.y) / cell.height;

  return true;
}

// end
