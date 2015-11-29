#ifndef CANXOSC_TRANSLATOR_H_
#define CANXOSC_TRANSLATOR_H_

#include "msg.hh"

struct lua_State;

namespace CANxOSC {

    class Translator {
        public:
            Translator(const std::string & config_file);
            ~Translator();

            void process(const Msg & message);

        private:
            void configure();
            void send_raw(const Msg & message);
            void send_translated(const Msg & message);

            lua_State * _L;
            void * _address;
            std::string _prefix;
            bool _send_raw;
    };

}

#endif /* CANXOSC_TRANSLATOR_H_ */
