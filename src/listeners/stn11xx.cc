#include "stn11xx.hh"

#include <stdexcept>
#include <cstdio>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

namespace CANxOSC {
    namespace Listeners {

        STN11xx::STN11xx(const string & tty, const string & transcript) {
            // open the TTY device
            _fd_tty = open(tty.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
            if (_fd_tty == -1)
                throw std::runtime_error("Could not open tty");

            _fd_transcript = open(transcript.c_str(),
                    O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);
            if (_fd_transcript == -1)
                throw std::runtime_error("Could not open transcript");

            configure_tty();
            configure_modem();
        }

        STN11xx::~STN11xx() {
            close(_fd_tty);
            close(_fd_transcript);
        }

        void STN11xx::configure_tty() {
            // set blocking mode
            fcntl(_fd_tty, F_SETFL, 0);

            // get the current TTY settings
            struct termios options;
            tcgetattr(_fd_tty, &options);

            // set the baud rate as high as we can
            cfsetispeed(&options, B115200);
            cfsetospeed(&options, B115200);

            // enable receiver and mark as "local" (non-owned) line
            options.c_cflag |= (CLOCAL | CREAD);

            // STN11xx uses 8 bits with no parity
            options.c_cflag &= ~CRTSCTS; // no hardware flow control
            options.c_cflag &= ~PARENB; // disable parity bit
            options.c_cflag &= ~CSTOPB; // 1 stop bit
            options.c_cflag &= ~CSIZE; // clear data bits
            options.c_cflag |= CS8; // set to 8 bits

            // local settings
            options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG); // raw input

            // input settings
            options.c_iflag &= ~INPCK; // no parity

            // output settings
            options.c_oflag &= ~OPOST; // raw output


            // write out settings
            if (tcsetattr(_fd_tty, TCSANOW, &options) != 0)
                throw std::runtime_error("Could not configure TTY");
        }

        void STN11xx::configure_modem() {
            // send ATZ twice to reset the modem. If there's unprocessed input,
            // the first one will cause it to become an invalid command which
            // will be discarded.
            send_line("ATZ");
            wait_for_prompt();
            send_line("ATZ");
            wait_for_prompt();

            // disable input echoing
            send_line("ATE0");
            wait_for_prompt();

            // crank the baud rate
            send_line("STBR 2000000");
            wait_for_prompt();

            // show headers when monitoring
            send_line("ATH1");
            wait_for_prompt();

            // set the protocol to HS-CAN, 29-bit
            send_line("STP32");
            wait_for_prompt();

            // clear filters
            send_line("STFCP"); // pass filters
            wait_for_prompt();
            send_line("STFCB"); // block filters
            wait_for_prompt();
            send_line("STFCFC"); // flow control filters
            wait_for_prompt();

            // start monitoring
            send_line("STMA");
        }

        bool STN11xx::has_next() {
            // for now
            return true;
        }

        Msg STN11xx::next() {
            // read a line in
            std::string line = read_line();

            if (line == "BUFFER FULL") {
                wait_for_prompt();
                send_line("STMA");
                line = read_line();
            }

            // the line will be in the format ID BYTE BYTE BYTE BYTE, where in
            // hexadecimal. ID is 29 bits long (a little under 8 nybbles)
            Id id;
            std::array<uint8_t, 8> b;
            int matched = std::sscanf(line.c_str(),
                    "%8x %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx",
                    &id,
                    &b[0], &b[1], &b[2], &b[3],
                    &b[4], &b[5], &b[6], &b[7]);

            // return the message
            return Msg(id, matched - 1, b.cbegin());
        }

        void STN11xx::wait_for_prompt() {
            char c;
            while (read(_fd_tty, &c, 1) > 0) {
                // write received character to transcript
                write(_fd_transcript, &c, 1);

                // break on prompt
                if (c == '>')
                    break;
            }

            fsync(_fd_transcript);
        }

        void STN11xx::send_line(const std::string & cmd) {
            // write to real device
            write(_fd_tty, cmd.c_str(), cmd.length());
            write(_fd_tty, "\r", 1);

            // also to transcript
            write(_fd_transcript, cmd.c_str(), cmd.length());
            write(_fd_transcript, "\r", 1);
            fsync(_fd_transcript);
        }

        std::string STN11xx::read_line() {
            std::string line;
            char c;
            while (read(_fd_tty, &c, 1) > 0) {
                // write received character to transcript
                write(_fd_transcript, &c, 1);

                // break on CR
                if (c == '\r')
                    break;

                // append to string
                line.push_back(c);
            }
            fsync(_fd_transcript);

            return line;
        }
    }
}

