#include "luaApi.h"
#include "unistd.h"
#include "string.h"
#include "Znet.h"
#include "iostream"
void LuaApi::Register(lua_State *luaState)
{
    static luaL_Reg libs[] {
        {"NewService",NewService},
        {"Write",Write}
    };
    luaL_newlib(luaState,libs);
    lua_setglobal(luaState,"znet");


}

int LuaApi::NewService(lua_State *luaState)
{
    int num = lua_gettop(luaState);
    size_t len = 0;
    const char* type = lua_tolstring(luaState,1,&len);
    char* newType = new char[len+1];
    newType[len] = '\0';
    memcpy(newType,type,len);
    auto t = std::make_shared<std::string>(newType);
    uint32_t id = Znet::instance->NewService(t);
    lua_pushinteger(luaState,id);
    return 1;
}

int LuaApi::KillService(lua_State *luaState)
{
    int id = lua_tointeger(luaState,1);
    Znet::instance->KillService(id);
    return 1;
}

int LuaApi::Send(lua_State *luaState)
{
    return 0;
}

int LuaApi::Listen(lua_State *luaState)
{
    return 0;
}

int LuaApi::CloseConn(lua_State *luaState)
{
    return 0;
}

int LuaApi::Write(lua_State *luaState)
{
    int num = lua_gettop(luaState);
    std::cout<< "num is:"<<num<<std::endl;
    for(int i=0;i<num;i++){
        int value = lua_tointeger(luaState,i+1);
        std::cout<<"value is:"<<value<<std::endl;
    }
    return 0;
}
