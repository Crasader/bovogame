/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include <cstdlib>

#include "BovoScene.h"
#include "BovoLayer.h"
#include "SimpleAudioEngine.h"
#include "extensions/cocos-ext.h"

USING_NS_CC;

using namespace cocos2d::extension;
using namespace std;

#define FONT_PATH "fonts/arial.ttf"
#define FONT_SIZE 10

#ifndef MIN
  #define MIN(x,y) (x < y ? x : y)
#endif

Scene* BovoScene::createScene()
{
    return BovoScene::create();
}

// on "init" you need to initialize your instance
bool BovoScene::init()
{
    //////////////////////////////
    if ( !Scene::init() ) {
      return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    if ((page = BovoLayer::create()) != nullptr) {
      Size page_size;
      if (initComponents(page, visibleSize, origin, page_size)) {
        if (page->initLayer(origin, page_size, opponent, message)) {
          this->addChild(page);
          return true;
        }
      }
    }

    return false;
}

bool BovoScene::initComponents(Layer* page, const Size& size, const Vec2& point, Size& page_size)
{
  Size but_siz;

  auto quit_item = MenuItemImage::create("CloseNormal.png", "CloseSelected.png", CC_CALLBACK_1(BovoScene::menuQuitCallback, this));
  if (quit_item == nullptr) {
    problemLoading("CloseNormal.png and CloseSelected.png");
  } else {
    but_siz = quit_item->getContentSize();
    if (but_siz.width <= 0 || but_siz.height <= 0) return false;
    quit_item->setPosition(Vec2(point.x + size.width  - but_siz.width / 2, point.y + but_siz.height / 2));
    auto quit_menu = Menu::create(quit_item, NULL);
    if (quit_menu != nullptr) {
      quit_menu->setPosition(Vec2::ZERO);
      page->addChild(quit_menu);
    }
  }

  auto connect_item = MenuItemImage::create("ConnectNormal.png", "ConnectSelected.png", CC_CALLBACK_1(BovoScene::menuConnectCallback, this));
  if (connect_item == nullptr) {
    problemLoading("ConnecNormal.png and ConnectSelected.png");
  } else {
    auto siz = connect_item->getContentSize();
    if (siz.width <= 0 || siz.height <= 0) return false;
    connect_item->setPosition(Vec2(point.x + size.width - but_siz.width - siz.width / 2 - 2, point.y + siz.height / 2));
    auto connect_menu = connectbut = Menu::create(connect_item, NULL);
    if (connect_menu != nullptr) {
      connect_menu->setPosition(Vec2::ZERO);
      page->addChild(connect_menu);
      but_siz.width += siz.width + 4;
    }
  }

  Size lab_siz;

  auto ipp_label = Label::createWithTTF("User@Server:", FONT_PATH, FONT_SIZE);
  if (ipp_label == nullptr) {
    problemLoading(FONT_PATH);
    return false;
  } else {
    lab_siz = ipp_label->getContentSize();
    ipp_label->setPosition(Vec2(point.x + size.width - lab_siz.height / 2, point.y + but_siz.height + lab_siz.width / 2 + 6));
    ipp_label->setRotation(-90.0);
    ipp_label->setTextColor(Color4B(0, 255, 0, 255));
    ipp_label->setHorizontalAlignment(TextHAlignment::LEFT);
    page->addChild(ipp_label);
  }

  if ((useratserver = TextFieldTTF::textFieldWithPlaceHolder("Input User@Server in here", Size(size.height - but_siz.height - lab_siz.width - 8, lab_siz.height), TextHAlignment::CENTER, FONT_PATH, FONT_SIZE)) == nullptr) {
    problemLoading(FONT_PATH);
    return false;
  } else {
    auto siz = useratserver->getContentSize();
    useratserver->setPosition(Vec2(point.x + size.width - lab_siz.height / 2, point.y + but_siz.height + lab_siz.width / 2 + siz.width / 2 + 8));
    useratserver->setRotation(-90.0);
    useratserver->setString("user@127.0.0.1:8000");
    useratserver->attachWithIME();
    page->addChild(useratserver);
  }

  Size siz;

  auto vs_label = Label::createWithTTF("Opponent:", FONT_PATH, FONT_SIZE);
  if (vs_label == nullptr) {
    problemLoading(FONT_PATH);
    return false;
  } else {
    siz = vs_label->getContentSize();
    vs_label->setPosition(Vec2(point.x + size.width - lab_siz.height - siz.height / 2 - 2, point.y + but_siz.height + siz.width / 2 + 6));
    vs_label->setRotation(-90.0);
    vs_label->setTextColor(Color4B(0, 255, 0, 255));
    page->addChild(vs_label);
  }

  if ((opponent = Label::createWithTTF("...", FONT_PATH, FONT_SIZE)) == nullptr) {
    problemLoading(FONT_PATH);
    return false;
  } else {
    Size sz = opponent->getContentSize();
    opponent->setPosition(Vec2(point.x + size.width - lab_siz.height - sz.height / 2 - 4, point.y + but_siz.height + siz.width + sz.width / 2 + 8));
    opponent->setRotation(-90.0);
    opponent->setTextColor(Color4B(128, 128, 128, 255));
    page->addChild(opponent);
  }

  Size xy = {0, 0};

  if ((message = Label::createWithTTF("Copyright (C) 2019 Hsiang Chen", FONT_PATH, FONT_SIZE)) == nullptr) {
    problemLoading(FONT_PATH);
    return false;
  } else {
    auto sz2 = message->getContentSize();
    message->setPosition(Vec2(xy.width = point.x + size.width - lab_siz.height - siz.height - sz2.height / 2 - 12, point.y + but_siz.height + sz2.width / 2 + 6));
    message->setRotation(-90.0);
    page->addChild(message);
  }

  page_size.width = MIN(xy.width, size.width - but_siz.width - 2);
  page_size.height = size.height;

  return true;
}

void BovoScene::menuQuitCallback(Ref* pSender)
{
    page->disconnect(); // disconnect from game server

    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void BovoScene::menuConnectCallback(Ref* pSender)
{
  if (page->connect(useratserver->getString())) {
    useratserver->detachWithIME();
    connectbut->setEnabled(false);
  }
}

// end
