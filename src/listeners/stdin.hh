#ifndef CANXOSC_LISTENERS_STDIN_H_
#define CANXOSC_LISTENERS_STDIN_H_

#include "../ilistener.hh"

namespace CANxOSC {
    namespace Listeners {

        class StdIn : public IListener {
            public:
                bool has_next() override;
                Msg next() override;

            private:
                Msg _last_msg = Msg(0);
                bool _has_last_msg;
        };

    }
}

#endif /* CANXOSC_LISTENERS_STDIN_H_ */
