#include "stdin.hh"

#include <iostream>

using namespace std;

namespace CANxOSC {
    namespace Listeners {

        bool StdIn::has_next() {
            // loop until we match or run out of stream
            while (!_has_last_msg) {
                if (cin.eof())
                    return false;

                string line;
                getline(cin, line);

                // the line will be in the format ID BYTE BYTE BYTE BYTE, where in
                // hexadecimal. ID is 29 bits long (a little under 8 nybbles)
                Id id;
                std::array<uint8_t, 8> b;
                int matched = std::sscanf(line.c_str(),
                        "%8x %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx",
                        &id,
                        &b[0], &b[1], &b[2], &b[3],
                        &b[4], &b[5], &b[6], &b[7]);

                if (matched >= 1) {
                    _last_msg = Msg(id, matched - 1, b.cbegin());
                    _has_last_msg = true;
                }
            }

            return true;
        }

        Msg StdIn::next() {
            if (!has_next())
                throw runtime_error("no next message!");

            _has_last_msg = false;
            return _last_msg;
        }
    }
}
