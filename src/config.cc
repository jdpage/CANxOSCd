#include "config.hh"
#include "listeners/stn11xx.hh"
#include "listeners/stdin.hh"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

using namespace std;

namespace CANxOSC {

    Config::Config(std::string & config_file) {
        // create a Lua interpreter to read the file
        _config_file = config_file;
        _L = luaL_newstate();
        luaL_openlibs(_L);

        // load and run the file
        if (luaL_loadfile(_L, config_file.c_str()) ||
                lua_pcall(_L, 0, 0, 0))
            throw runtime_error("could not load config \"" + config_file + "\"");
    }

    Config::~Config() {
        lua_close(_L);
    }

    unique_ptr<IListener> Config::create_listener() {
        // get the device config table
        lua_getglobal(_L, "device");
        if (!lua_istable(_L, -1))
            throw runtime_error("device should be a table");

        // get the name so we know what properties to look for
        lua_getfield(_L, -1, "kind");
        if (!lua_isstring(_L, -1))
            throw runtime_error("device.kind should be a string");
        string device_kind = lua_tostring(_L, -1);
        lua_pop(_L, 1); // remove device.kind from the stack

        if (device_kind == "stdin") {
            // stdin just reads from stdin
            // it's really boring
            lua_pop(_L, 1); // remove device from stack

            // return the listener
            return make_unique<Listeners::StdIn>();
        } else if (device_kind == "stn11xx") {
            // stn11xx has a TTY and an optional transcript file

            // get device.tty
            lua_getfield(_L, -1, "tty");
            if (!lua_isstring(_L, -1))
                throw runtime_error("device.tty should be a string");
            string device_tty = lua_tostring(_L, -1);
            lua_pop(_L, 1); // remove device.tty from the stack

            // get device.transcript_file
            lua_getfield(_L, -1, "transcript_file");
            string transcript_file;
            if (lua_isnil(_L, -1)) {
                transcript_file = "/dev/null"; // no transcript
            } else if (!lua_isstring(_L, -1)) {
                throw runtime_error("device.transcript_file should be nil or string");
            } else {
                transcript_file = lua_tostring(_L, -1);
            }
            lua_pop(_L, 2); //remove device.transcript_file, device from stack

            // return the stn11xx object
            return make_unique<Listeners::STN11xx>(device_tty, transcript_file);
        } else {
            throw runtime_error("unknown device kind \"" + device_kind + "\"");
        }
    }

    unique_ptr<Translator> Config::create_translator() {
        return make_unique<Translator>(_config_file);
    }

}
