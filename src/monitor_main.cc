#include "config.hh"

#include <stdexcept>
#include <iostream>
#include <fstream>

using namespace std;
using namespace CANxOSC;

int main(int argc, char * argv[]) {
    // decide where the config file is
    string config_file;
    if (argc >= 2) {
        config_file = argv[1];
    } else {
        config_file = "/etc/canxoscd.conf";
    }

    // create a config reader
    Config config(config_file);

    unique_ptr<IListener> listener = config.create_listener();

    // blithely output results to stdout
    while (listener->has_next()) {
        cout << listener->next().to_string() << endl;
    }

    return 0;
}
