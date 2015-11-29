#include "config.hh"

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

    unique_ptr<IListener> listener;
    unique_ptr<Translator> translator;
    {
        // get data from config
        Config config(config_file);
        listener = config.create_listener();
        translator = config.create_translator();
    }

    while (listener->has_next()) {
        translator->process(listener->next());
    }

    return 0;
}
