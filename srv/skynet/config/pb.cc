/* @ $pb.cc - author: Hsiang Chen - license: GPL*/
#include <string>
#include <cstdlib>
#include <lua.hpp>

#include "msg.pb.h"

using namespace std;
using namespace bovo_message;

struct BMUD {
  BovoMessage* bm;
};

static int pb_init(lua_State* state)
{
  BMUD* p = (BMUD*) lua_newuserdata(state, sizeof(*p));
  if (p != nullptr) {
    p->bm = new BovoMessage();
    lua_pushlightuserdata(state, p);
  } else
    lua_pushnil(state);
  return 1;
}

// userdata, ...
static int pb_free(lua_State* state)
{
  int n = lua_gettop(state), i;

  for (i = 1; i <= n; i++) {
    if (lua_islightuserdata(state, i)) {
      BMUD* p = (BMUD*) lua_touserdata(state, i);
      if (p != nullptr && p->bm != nullptr) {
        delete p->bm;
        p->bm = nullptr;
      }
    }
  }

  return 0;
}

static int pb_get(lua_State* state)
{
  int n = lua_gettop(state);
  bool okay = false;

  if (n == 2) {
    if (lua_islightuserdata(state, 1) && lua_isstring(state, 2)) {
      BMUD* p = (BMUD*) lua_touserdata(state, 1);
      if (p != nullptr) {
        BovoMessage* bm = p->bm;
        string name = lua_tostring(state, 2);
        okay = true;
        if (name == "magic" && bm->has_magic()) lua_pushinteger(state, bm->magic());
        else if (name == "cmd" && bm->has_cmd()) lua_pushinteger(state, (int) bm->cmd());
        else if (name == "status" && bm->has_status()) lua_pushinteger(state, (int) bm->status());
        else if (name == "name" && bm->has_name()) lua_pushstring(state, bm->name().c_str());
        else if (name == "address" && bm->has_address()) lua_pushstring(state, bm->address().c_str());
        else if (name == "markstyle" && bm->has_markstyle()) lua_pushinteger(state, bm->markstyle());
        else if (name == "x" && bm->has_x()) lua_pushinteger(state, bm->x());
        else if (name == "y" && bm->has_y()) lua_pushinteger(state, bm->y());
        else okay = false;
      }
    }
  }

  if (! okay)
    lua_pushnil(state);

  return 1;
}

static int pb_set(lua_State* state)
{
  int n = lua_gettop(state);
  bool okay = false;

  if (n == 3) {
    if (lua_islightuserdata(state, 1) && lua_isstring(state, 2)) {
      BMUD* p = (BMUD*) lua_touserdata(state, 1);
      if (p != nullptr) {
        BovoMessage* bm = p->bm;
        string name = lua_tostring(state, 2);
        okay = true;
        if (name == "magic") bm->set_magic(lua_tointeger(state, 3));
        else if (name == "cmd") bm->set_cmd((BovoMessage::CmdType) lua_tointeger(state, 3));
        else if (name == "status") bm->set_status((BovoMessage::StatusType) lua_tointeger(state, 3));
        else if (name == "name") bm->set_name(lua_tostring(state, 3));
        else if (name == "address") bm->set_address(lua_tostring(state, 3));
        else if (name == "markstyle") bm->set_markstyle(lua_tointeger(state, 3));
        else if (name == "x") bm->set_x(lua_tointeger(state, 3));
        else if (name == "y") bm->set_y(lua_tointeger(state, 3));
        else okay = false;
      }
    }
  }

  if (okay)
    lua_pushboolean(state, 1);
  else
    lua_pushboolean(state, 0);

  return 1;
}

static int pb_clear(lua_State* state)
{
  int n = lua_gettop(state);
  
  if (n == 2) {
    if (lua_islightuserdata(state, 1) && lua_isstring(state, 2)) {
      BMUD* p = (BMUD*) lua_touserdata(state, 1);
      if (p != nullptr) {
        BovoMessage* bm = p->bm;
        string name = lua_tostring(state, 2);
        if (name == "magic") bm->clear_magic();
        else if (name == "cmd") bm->clear_cmd();
        else if (name == "status") bm->clear_status();
        else if (name == "name") bm->clear_name();
        else if (name == "address") bm->clear_address();
        else if (name == "markstyle") bm->clear_markstyle();
        else if (name == "x") bm->clear_x();
        else if (name == "y") bm->clear_y();
      }
    }
  }

  return 0;
}

// userdata -> string, length
static int pb_serialize(lua_State* state)
{
  int n = lua_gettop(state);
  bool okay = false;

  if (n == 1 && lua_islightuserdata(state, 1)) {
    BMUD* p = (BMUD*) lua_touserdata(state, 1);
    if (p != nullptr) {
      BovoMessage* bm = p->bm;
      string str;
      if (bm->SerializeToString(&str)) {
        lua_pushlstring(state, str.c_str(), str.size());
        lua_pushinteger(state, str.size());
        okay = true;
      }
    }
  }

  if (! okay) {
    lua_pushnil(state);
    lua_pushinteger(state, 0);
  }

  return 2;
}

// userdata (uninitialized) , pointer, length -> userdata (initialized)
static int pb_parse(lua_State* state)
{
  int n = lua_gettop(state);
  bool okay = false;

  if (n == 3 && lua_islightuserdata(state, 1) && lua_isstring(state, 2) && lua_isinteger(state, 3)) {
    BMUD* p = (BMUD*) lua_touserdata(state, 1);
    if (p != nullptr) {
      BovoMessage* bm = p->bm;
      char* p = (char*) lua_tostring(state, 2);
      int n = lua_tointeger(state, 3);
      string str(p, n);
      if (bm->ParseFromString(str)) {
        lua_pushlightuserdata(state, bm);
        okay = true;
      }
    }
  }

  if (! okay)
    lua_pushnil(state);

  return 1;
}

extern "C" int luaopen_libpb(lua_State* state)
{
  luaL_Reg li[] = {
    {"init", pb_init},
    {"free", pb_free},
    {"get", pb_get},
    {"set", pb_set},
    {"clear", pb_clear},
    {"serialize", pb_serialize},
    {"parse", pb_parse},
    {nullptr, nullptr}
  };
  luaL_newlib(state, li);
  return 1;
}
