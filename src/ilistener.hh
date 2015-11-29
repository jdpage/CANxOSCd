#ifndef CANXOSC_ILISTENER_H_
#define CANXOSC_ILISTENER_H_

#include "msg.hh"

namespace CANxOSC {

    class IListener {
        public:
            virtual bool has_next() = 0;
            virtual Msg next() = 0;
    };

}

#endif /* CANXOSC_ILISTENER_H_ */
