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
        preparationTime = TIME("00:00:10");
        state.request = Request::None;
        state.status = Disarmed;
        state.nextInternal = preparationTime;
        state.pinCheck = PNone;
    }

    struct state_type {
        Status status;
        Request request;
        PinCheck pinCheck;
        TIME nextInternal;
    };

    state_type state;

    // ports definition
    using input_ports = std::tuple<typename Authentication_defs::in>;
    using output_ports = std::tuple<typename Authentication_defs::out>;

    void internal_transition() {
        state.pinCheck = PNone;
        state.request = Request::None;
        state.nextInternal = std::numeric_limits<TIME>::infinity();
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        if (get_messages<typename Authentication_defs::in>(mbs).size() > 1)
            assert(false && "One message per time unit");
        vector <Message_t> message_port_in;
        message_port_in = get_messages<typename Authentication_defs::in>(mbs);
        int port = message_port_in[0].port;
        int message = message_port_in[0].message;

        switch (port) {
            case 0:
                if (state.request == Request::None) {
                    if (message == 0) {
                        state.status = Disarmed;
                    } else if (message == 1) {
                        state.status = Armed;
                    } else {
                        assert(false && "Invalid message passed to Pin Model");
                    }
                    state.request = Request::None;
                    state.pinCheck = PNone;
                    state.nextInternal = std::numeric_limits<TIME>::infinity();
                } else {
                    assert(false && "Invalid message/port reset with request None");
                    state.nextInternal -= e;
                }
                break;
            case 1:
                if (message == 1) {
                    double randNumber = (double) rand() / (double) RAND_MAX;
                    if (state.request == Arm) {
                        if (randNumber <= 0.9) {
                            state.pinCheck = ArmValid;
                        } else {
                            state.pinCheck = Invalid;
                        }
                    } else if (state.request == Request::Disarm) {
                        if (randNumber <= 0.9) {
                            state.pinCheck = DisarmValid;
                        } else {
                            state.pinCheck = Invalid;
                        }
                    } else {
                        if (randNumber <= 0.9) {
                            state.pinCheck = DoorValid;
                        } else {
                            state.pinCheck = DoorInvalid;
                        }
                    }
                    state.nextInternal = preparationTime;
                }
                break;
            case 2:
                if (message == 0) {
                    state.request = Arm;
                    state.nextInternal = std::numeric_limits<TIME>::infinity();
                }
                if (message == 1) {
                    state.request = Disarm;
                    state.nextInternal = std::numeric_limits<TIME>::infinity();
                }
                state.nextInternal -= e;
        }
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }


    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        Message_t out_aux;
        switch (i.pinCheck) {
            case DisarmValid:
                out_aux = Message_t(0, 1);
                break;
            case ArmValid:
                out_aux = Message_t(0, 2);
                break;
            case Invalid:
                out_aux = Message_t(0, 3);
                break;
            case DoorValid:
                out_aux = Message_t(0, 4);
                break;
            case DoorInvalid:
                out_aux = Message_t(0, 5);
                break;
            case PNone:
                out_aux = Message_t(0, 6);
                break;
        }


        get_messages<typename Authentication_defs::out>(bags).push_back(out_aux);
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

        switch (i.request) {
            case Arm:
                request = "Arm";
                break;
            case Disarm:
                request = "Disarm";
                break;
            case Pin:
                request = "Pin"; //Never be here
                break;
            case None:
                request = "None";
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

        os << "Authentication PinCheck: " << pinCheck << "; Status:" << status << ";   Request: " << request;
        return os;
    }
};


#endif //SECUREAREAACCESS_AUTHENTICATION_HPP
