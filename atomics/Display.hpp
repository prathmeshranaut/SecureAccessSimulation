//
// Created by Prathmesh Ranaut on 10/18/19.
//

#ifndef __DISPLAY_HPP__
#define __DISPLAY_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

struct Display_defs {
    struct out : public out_port<Message_t> {
    };
    struct in : public in_port<Message_t> {
    };
};

enum DisplayState {
    DisarmedMsg, ArmedMsg, PINMsg, InvalidAction, InvalidPin, DoorUnlocked
};

template<typename TIME>
class Display {

public:
    TIME preparationTime;

    Display() noexcept {
        preparationTime = TIME("00:00:05");
        state.status = Disarmed;
        state.next_internal = preparationTime;
    }

    struct state_type {
        Status status;
        DisplayState display;
        TIME next_internal;
    };

    state_type state;

    // ports definition
    using input_ports = std::tuple<typename Display_defs::in>;
    using output_ports = std::tuple<typename Display_defs::out>;

    void internal_transition() {
        if (state.display == ArmedMsg || state.display == DisarmedMsg || state.display == PINMsg) {
            // DO nothing
            state.next_internal = std::numeric_limits<TIME>::infinity();
        } else {
            if (state.status == Armed) {
                state.display = ArmedMsg;
                state.next_internal = std::numeric_limits<TIME>::infinity();
            } else if (state.status == Disarmed) {
                state.display = DisarmedMsg;
                state.next_internal = std::numeric_limits<TIME>::infinity();
            }
        }
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        if (get_messages<typename Display_defs::in>(mbs).size() > 1)
            assert(false && "One message per time unit");

        for (const auto &x : get_messages<typename Display_defs::in>(mbs)) {
            if (x.message == 0) {
                //Disarmed
                state.display = DisarmedMsg;
                state.status = Disarmed;
                state.next_internal = preparationTime;
            } else if (x.message == 1) {
                //Armed
                state.display = ArmedMsg;
                state.status = Armed;
                state.next_internal = preparationTime;
            } else if (x.message == 4) {
                //Invalid Pin
                state.display = InvalidPin;
                state.next_internal = preparationTime;
            }
        }
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    //Output function - lambda(s)
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        Message_t out_aux;
        out_aux = Message_t(0, state.display);
        get_messages<typename Display_defs::out>(bags).push_back(out_aux);
        return bags;
    }

    TIME time_advance() const {
        return state.next_internal;
    }

    friend std::ostringstream &operator<<(std::ostringstream &os, const typename Display<TIME>::state_type &i) {
        string display = "Disarmed";
        string status = "Armed";

        switch (i.display) {
            case ArmedMsg:
                display = "Armed";
                break;
            case DisarmedMsg:
                display = "Disarmed";
                break;
            case PINMsg:
                display = "Enter Pin";
                break;
            case InvalidAction:
                display = "Invalid Action";
                break;
            case InvalidPin:
                display = "Invalid Pin";
                break;
            case DoorUnlocked:
                display = "Door unlocked";
                break;
        }

        switch (i.status) {
            case Armed:
                status = "Armed";
                break;
            case Disarmed:
                status = "Disarmed";
                break;
        }

        os << "Display Status:" << status << ";  Display: " << display;
        return os;
    }
};


#endif //__DISPLAY_HPP__
