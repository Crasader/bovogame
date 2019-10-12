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

#ifndef __BOVOSCENE_H__
#define __BOVOSCENE_H__

#include "cocos2d.h"
#include "BovoLayer.h"
#include "Utils.h"

class BovoScene : public cocos2d::Scene, public Utils
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuQuitCallback(cocos2d::Ref* pSender);
    void menuConnectCallback(cocos2d::Ref* pSender);

    bool initComponents(cocos2d::Layer* page, const cocos2d::Size& size, const cocos2d::Vec2& point, cocos2d::Size& page_size);
    
    // implement the "static create()" method manually
    CREATE_FUNC(BovoScene);
private:
    cocos2d::TextFieldTTF* useratserver;
    cocos2d::Label* opponent,* message;
    cocos2d::Menu* connectbut;
    
    BovoLayer* page;
};

#endif // __BOVOSCENE_H__
