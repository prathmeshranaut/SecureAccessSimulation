//
// Created by Prathmesh Ranaut on 10/18/19.
//

#ifndef SECUREAREAACCESS_AUTHENTICATION_HPP
#define SECUREAREAACCESS_AUTHENTICATION_HPP

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <stdlib.h>
#include <limits>
#include <assert.h>
#include <string>

#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

struct Authentication_defs {
    struct out : public out_port<Message_t> {
    };
    struct in : public in_port<Message_t> {
    };
    struct alarmStatus2 : public in_port<Message_t> {
    };
};

enum PinCheck {
    DisarmValid, ArmValid, Invalid, DoorValid, DoorInvalid, PNone
};

template<typename TIME>
class Authentication {

public:
    TIME preparationTime;

    Authentication() noexcept {
        preparationTime = TIME("00:00:02");
        state.nextInternal = std::numeric_limits<TIME>::infinity();
        state.pinCheck = PNone;
    }

    struct state_type {
        Status status;
        PinCheck pinCheck;
        TIME nextInternal;
    };

    state_type state;

    // ports definition
    using input_ports = std::tuple<typename Authentication_defs::in>;
    using output_ports = std::tuple<typename Authentication_defs::out>;

    void internal_transition() {
        state.pinCheck = PNone;
        state.nextInternal = std::numeric_limits<TIME>::infinity();
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        if (get_messages<typename Authentication_defs::in>(mbs).size() > 1)
            assert(false && "One message per time unit");

        for (const auto &x : get_messages<typename Authentication_defs::in>(mbs)) {
            double randNumber = (double) rand() / (double) RAND_MAX;

            if (x.message == 0) {
                // Disarm Request
                if (randNumber <= 0.5) {
                    state.pinCheck = DisarmValid;
                } else {
                    state.pinCheck = Invalid;
                }
                state.nextInternal = preparationTime;
            } else if (x.message == 1) {
                // Arm Request
                if (randNumber <= 0.5) {
                    state.pinCheck = ArmValid;
                } else {
                    state.pinCheck = Invalid;
                }
                state.nextInternal = preparationTime;
            }
        }
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }


    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        Message_t out_aux;

        switch (state.pinCheck) {
            case DisarmValid:
                out_aux = Message_t(0, 0);
                get_messages<typename Authentication_defs::out>(bags).push_back(out_aux);
                break;
            case ArmValid:
                out_aux = Message_t(0, 1);
                get_messages<typename Authentication_defs::out>(bags).push_back(out_aux);
                break;
            case Invalid:
                out_aux = Message_t(0, 2);
                get_messages<typename Authentication_defs::out>(bags).push_back(out_aux);
                break;
            case DoorInvalid:
            case DoorValid:
            case PNone:
            default:
                break;
        }

        return bags;
    }

    TIME time_advance() const {
        return state.nextInternal;
    }

    friend std::ostringstream &operator<<(std::ostringstream &os, const typename Authentication<TIME>::state_type &i) {
        string request = "None";
        string status = "Armed";
        string pinCheck = "";

        switch (i.pinCheck) {
            case DisarmValid:
                pinCheck = "DisarmValid";
                break;
            case ArmValid:
                pinCheck = "ArmValid";
                break;
            case Invalid:
                pinCheck = "Invalid";
                break;
            case DoorValid:
                pinCheck = "DoorValid";
                break;
            case DoorInvalid:
                pinCheck = "DoorInvalid";
                break;
            case PNone:
                pinCheck = "PNone";
                break;
        }

        os << "Authentication PinCheck: " << pinCheck;
        return os;
    }
};


#endif //SECUREAREAACCESS_AUTHENTICATION_HPP
