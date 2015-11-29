#ifndef CANXOSC_MSG_H_
#define CANXOSC_MSG_H_

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <sstream>

namespace CANxOSC {
        
    typedef int32_t Id;

    class Msg {
        public:

            template<int N>
            Msg(Id id, std::array<uint8_t, N> data)
            : Msg(id, N, data.begin()) {
                static_assert(0 <= N && N <= 8,
                        "message data is from 0 to 8 bytes");
            }

            template<typename It>
            Msg(Id id, size_t length, It beginData)
            : _id(id), _data(length)
            {
                if (length > 8)
                    throw std::length_error(
                            "message data is limited to 0 to 8 bytes");
                std::copy_n(beginData, length, _data.begin());
            }

            Msg(Id id) : _id(id), _data(0) { }

            Id id() const { return _id; }

            size_t size() const { return _data.size(); }

            auto begin() const { return _data.cbegin(); }

            auto end() const { return _data.cend(); }

            auto cbegin() const { return _data.cbegin(); }

            auto cend() const { return _data.cend(); }

            const uint8_t * data() const { return _data.data(); }

            std::string to_string() const {
                std::stringstream info;
                info << std::hex << _id << " ";

                if (size() == 0) {
                    info << "()";
                } else {
                    auto it = _data.cbegin();
                    info << "(" << ((int)*it);
                    it++;
                    for (; it != _data.cend(); it++) {
                        info << ", " << ((int)*it);
                    }
                    info << ")";
                }

                return info.str();
            }

        private:
            Id _id;
            std::vector<uint8_t> _data;
    };

}

#endif /* CANXOSC_MSG_H_ */
