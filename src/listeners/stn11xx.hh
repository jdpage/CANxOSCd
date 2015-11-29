#ifndef CANXOSC_LISTENERS_STN11XX_H_
#define CANXOSC_LISTENERS_STN11XX_H_

#include "../ilistener.hh"

namespace CANxOSC {
    namespace Listeners {

        class STN11xx : public IListener {
            public:
                STN11xx(const std::string & tty, const std::string & transcript);
                ~STN11xx();
                bool has_next() override;
                Msg next() override;

            private:
                void configure_tty();
                void configure_modem();
                void wait_for_prompt();
                void send_line(const std::string & cmd);
                std::string read_line();

                int _fd_tty;
                int _fd_transcript;
        };

    }
}

#endif /* CANXOSC_LISTENERS_STN11XX_H_ */
