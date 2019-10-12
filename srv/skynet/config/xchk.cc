/* @ $xchk.cc - author: Hsiang Chen - license: GPL*/
#include <string>
#include <cstring>
#include <lua.hpp>

//#include <iostream>

using namespace std;

#define MATRIX_ROW 16
#define MATRIX_COL 18

struct UserData {
  uint32_t x, y;
  uint8_t matrix[MATRIX_ROW][MATRIX_COL];
};

static int xchk_init(lua_State* state)
{
  UserData* ud = (UserData*) lua_newuserdata(state, sizeof(UserData));
  if (ud != nullptr) 
    lua_pushlightuserdata(state, ud);
  else
    lua_pushnil(state);
  return 1;
}

// userdata, x, y, mark
static int xchk_set(lua_State* state)
{
  int n = lua_gettop(state);

  if (n == 4) {
    UserData* ud = nullptr;
    uint32_t x = -1, y = -1, mark = 0;
    
    if (lua_islightuserdata(state, 1)) ud = (UserData*) lua_touserdata(state, 1);
    if (lua_isinteger(state, 2)) x = lua_tointeger(state, 2);
    if (lua_isinteger(state, 3)) y = lua_tointeger(state, 3);
    if (lua_isinteger(state, 4)) mark = lua_tointeger(state, 4);

    if (ud != nullptr && x >= 0 && x < MATRIX_ROW && y >= 0 && y < MATRIX_COL) {
      ud->x = x; ud->y = y;
      ud->matrix[x][y] = mark & 0xff;
    }
  }

  return 0;
}

/*
 0+----------
  |    N
  | W  +  E
  |    S
  +----------*/
bool xchk_check_point(UserData* ud, int x, int y, uint8_t mark, uint32_t& rec_length, uint8_t di)
{
  if (ud == nullptr) return false;
//cout << "matrix: " << x << "," << y << ":" << (uint32_t)ud->matrix[x][y] << " ? mark=" << (uint32_t)mark << endl;

  if (x >= MATRIX_ROW) return false;
  if (x < 0) return false;
  if (y >= MATRIX_COL) return false;
  if (y < 0) return false;

  if (ud->matrix[x][y] != mark) return false;
  else rec_length++;

  if (rec_length >= 5) return true;

  int xx, yy;

  // direction: N
  xx = x - 1;
  if (xx >= 0 && di == 1)
    return xchk_check_point(ud, xx, y, mark, rec_length, di);
  
  // direction: NE
  xx = x - 1;
  yy = y + 1;
  if (xx >= 0 && yy < MATRIX_COL && di == 2)
    return xchk_check_point(ud, xx, yy, mark, rec_length, di);

  // direction: E
  yy = y + 1;
  if (yy < MATRIX_COL && di == 3)
    return xchk_check_point(ud, x, yy, mark, rec_length, di);

  // direction: ES
  xx = x + 1;
  yy = y + 1;
  if (xx < MATRIX_ROW && yy < MATRIX_COL && di == 4)
    return xchk_check_point(ud, xx, yy, mark, rec_length, di);

  // direction: S
  xx = x + 1;
  if (xx < MATRIX_ROW && di == 5)
    return xchk_check_point(ud, xx, y, mark, rec_length, di);

  // direction: SW
  xx = x + 1;
  yy = y - 1;
  if (xx < MATRIX_ROW && yy >= 0 && di == 6)
    return xchk_check_point(ud, xx, yy, mark, rec_length, di);

  // direction: W
  yy = y - 1;
  if (yy >= 0 && di == 7)
    return xchk_check_point(ud, x, yy, mark, rec_length, di);

  // direction: WN
  xx = x - 1;
  yy = y - 1;
  if (xx >= 0 && yy >= 0 && di == 8)
    return xchk_check_point(ud, xx, yy, mark, rec_length, di);
  
  return false;
}

// userdata, x, y, mark -> true/false (checked or not)
static int xchk_check(lua_State* state)
{
  int n = lua_gettop(state);
  bool okay = false;

  if (n == 4) {
    UserData* ud = nullptr;
    uint32_t x = -1, y = -1, mark = 0;

    if (lua_islightuserdata(state, 1)) ud = (UserData*) lua_touserdata(state, 1);
    if (lua_isinteger(state, 2)) x = lua_tointeger(state, 2);
    if (lua_isinteger(state, 3)) y = lua_tointeger(state, 3);
    if (lua_isinteger(state, 4)) mark = lua_tointeger(state, 4);

    uint32_t rec_length;
    uint8_t di, i;

    for (i = 0, di = 1; i < 8; i++) {
      rec_length = 0;
      if (xchk_check_point(ud, x, y, mark, rec_length, di + i)) {
        okay = true;
        break;
      }
    }
  }

  if (okay)
    lua_pushboolean(state, 1);
  else
    lua_pushboolean(state, 0);

  return 1;
}

// userdata, ...
static int xchk_clear(lua_State* state)
{
  int n = lua_gettop(state), i;

  for (i = 1; i <= n; i++) {
    if (lua_isuserdata(state, i)) {
      UserData* ud = (UserData*) lua_touserdata(state, i);
      if (ud != nullptr) {
        ud->x = ud->y = 0;
        memset(&ud->matrix, 0, sizeof(ud->matrix));
      }
    }
  }

  return 0;
}

extern "C" int luaopen_libxchk(lua_State* state)
{
  luaL_Reg li[] = {
    {"init", xchk_init},
    {"set", xchk_set},
    {"check", xchk_check},
    {"clear", xchk_clear},
    {nullptr, nullptr}
  };
  luaL_newlib(state, li);
  return 1;
}
