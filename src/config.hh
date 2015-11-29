#ifndef CANXOSC_CONFIG_H_
#define CANXOSC_CONFIG_H_

#include "ilistener.hh"
#include "translator.hh"

struct lua_State;

namespace CANxOSC {

    class Config {
        public:
            Config(std::string & config_file);
            ~Config();

            std::unique_ptr<IListener> create_listener();
            std::unique_ptr<Translator> create_translator();

        private:
            lua_State * _L;
            std::string _config_file;
    };

}


#endif /* CANXOSC_CONFIG_H_ */
