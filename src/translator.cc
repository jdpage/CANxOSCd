#include "translator.hh"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lo/lo.h"

using namespace std;

namespace CANxOSC {

    Translator::Translator(const std::string & config_file) {
        // create a lua interpreter
        _L = luaL_newstate();
        luaL_openlibs(_L);

        // load and execute the file
        if (luaL_loadfile(_L, config_file.c_str()) ||
                lua_pcall(_L, 0, 0, 0))
            throw runtime_error("could not load config \"" + config_file + "\"");

        // configure self
        configure();
    }

    Translator::~Translator() {
        lua_close(_L);
        lo_address_free(_address);
    }

    void Translator::configure() {
        // get the translation info
        lua_getglobal(_L, "translation");
        if (!lua_istable(_L, -1))
            throw runtime_error("translation should be a table");

        // get prefix
        lua_getfield(_L, -1, "prefix");
        if (lua_isnil(_L, -1)) {
            _prefix = "";
        } else if (lua_isstring(_L, -1)) {
            _prefix = lua_tostring(_L, -1);
        } else {
            throw runtime_error("translation.prefix should be a string");
        }
        lua_pop(_L, 1);

        // get send_raw
        lua_getfield(_L, -1, "send_raw");
        _send_raw = lua_toboolean(_L, -1);
        lua_pop(_L, 1);

        // get address
        lua_getfield(_L, -1, "address");
        if (!lua_isstring(_L, -1)) {
            throw runtime_error("translation.address should be a string");
        }
        _address = lo_address_new_from_url(lua_tostring(_L, -1));
        lua_pop(_L, 1);

        // now we're going to configure the stack for the remaining operations
        lua_getfield(_L, -1, "map");
        if (!lua_istable(_L, -1)) {
            throw runtime_error("translation.map should be a table of functions");
        }

        // note that we then leave translation.map on the stack, since process
        // is going to be using it a LOT.
    }

    void Translator::process(const Msg & message) {
        if (_send_raw) {
            send_raw(message);
        }
        send_translated(message);
    }

    void Translator::send_raw(const Msg & message) {
        // prepare the data for the OSC message
        ostringstream path;
        path << _prefix << "/raw/" << hex << message.id();
        lo_message oscmsg = lo_message_new();
        for (uint8_t b : message) {
            lo_message_add_int32(oscmsg, b);
        }

        // send the message
        lo_send_message(_address, path.str().c_str(), oscmsg);

        // free the message
        lo_message_free(oscmsg);
    }

    void Translator::send_translated(const Msg & message) {
        // we know from configure that translation.map is still on the stack.
        // so we can index into it by CAN ID
        int map = lua_absindex(_L, -1);
        lua_rawgeti(_L, -1, message.id());
        if (lua_isnil(_L, -1)) {
            // we don't map that id, so just return now.
            lua_pop(_L, 1);
            return;
        } else if (!lua_isfunction(_L, -1)) {
            throw runtime_error("translation.map should be a table of functions");
        }

        // push the message bytes onto the stack
        for (uint8_t b : message) {
            lua_pushnumber(_L, b);
        }

        // call the function
        lua_call(_L, message.size(), 1);

        // this results in a table of n items, where the keys are OSC paths
        // and the values are the values. Loop over them and dispatch the
        // methods.
        lua_pushnil(_L);
        while (lua_next(_L, -2) != 0) {
            if (!lua_isstring(_L, -2))
                throw runtime_error(
                        "translation.map functions should return string keys");
            if (!lua_istable(_L, -1))
                throw runtime_error(
                        "translation.map functions should return table values");

            string path = _prefix + lua_tostring(_L, -2);

            // create a message object, and add the valeus to it
            lo_message oscmsg = lo_message_new();

            lua_pushnil(_L);
            while (lua_next(_L, -2) != 0) {
                // each argument is also a table
                if (!lua_istable(_L, -1))
                    throw runtime_error("bad map function");

                int arg = lua_absindex(_L, -1);
                lua_rawgeti(_L, arg, 1);
                lua_rawgeti(_L, arg, 2);
                if (!lua_isstring(_L, -2))
                    throw runtime_error("bad map function");

                string type = lua_tostring(_L, -2);
                if (type == "i") {
                    lo_message_add_int32(oscmsg, lua_tointeger(_L, -1));
                } else if (type == "f") {
                    lo_message_add_float(oscmsg, lua_tonumber(_L, -1));
                } else if (type == "s") {
                    lo_message_add_string(oscmsg, lua_tostring(_L, -1));
                } else {
                    throw runtime_error("invalid OSC type tag");
                }

                // pop off the type tag, value, and iteration value
                lua_pop(_L, 3);
            }

            // message created, now send it
            lo_send_message(_address, path.c_str(), oscmsg);
            lo_message_free(oscmsg);

            // discard the message value
            lua_pop(_L, 1);
        }

        // at this point the last thing on the stack is the function result
        lua_pop(_L, 1);

        // we should now be back at the translation.map
        if (lua_absindex(_L, -1) != map) {
            throw runtime_error("bad stack state");
        }
    }
}
