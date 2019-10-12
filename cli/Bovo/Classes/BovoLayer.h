// $@ BovoLayer.h

#ifndef __BOVOLAYER_H__
#define __BOVOLAYER_H__

#include <map>
#include <string>

#include "cocos2d.h"
#include "BovoSocket.h"
#include "BovoMessageDelegate.h"

#define MATRIX_ROW 16
#define MATRIX_COL 18

struct UiSize {
  uint32_t x, y;
};

class BovoLayer : public cocos2d::Layer, public BovoMessageDelegate {
public:
  BovoLayer(cocos2d::Layer* lc);
  ~BovoLayer();

  static BovoLayer* create();

  bool initLayer( const cocos2d::Vec2& ori, const cocos2d::Size& siz, \
                  cocos2d::Label* oppo, cocos2d::Label* mesg );
  bool connect(const string& uiaddr);
  void disconnect();

  virtual void onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* unused_event);
  virtual void onTouchesMoved(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* unused_event);
  virtual void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* unused_event);
  virtual void onTouchesCancelled(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* unused_event);
protected:
  virtual void onMessage(const bovo_message::BovoMessage& msg);
private:
  uint32_t normToInt(const UiSize& norm);
  bool setCell(const UiSize& norm, uint32_t markstyle);

  void calOriginCoordinate(cocos2d::Vec2* ori);
  bool getCoordinateSize(cocos2d::Size* to);
  bool getCellSize(cocos2d::Sprite* spt, cocos2d::Size* to);

  bool getCoordinateByNorm(const UiSize& from, cocos2d::Vec2& to);
  bool getNormByPixelPoint(const cocos2d::Vec2& from, UiSize& to);

  cocos2d::Layer* lc;
  cocos2d::Size size, coor, cell;
  cocos2d::Vec2 origin;

  bool busy, connected;

  std::map<uint32_t, cocos2d::Sprite*> matrix;

  cocos2d::Label* oppo,* mesg;
  
  uint32_t markstyle;
  std::string username;

  cocos2d::Sprite* spt_self,* spt_oppo;
};

#endif // __BOVOLAYER_H__
