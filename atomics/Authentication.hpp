//
// Created by Prathmesh Ranaut on 10/18/19.
//

#ifndef SECUREAREAACCESS_AUTHENTICATION_HPP
#define SECUREAREAACCESS_AUTHENTICATION_HPP

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

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
};


enum Status {
    Disarmed, Armed
};

enum Request {
    Arm, Disarm, None
};

enum PinCheck {
    DisarmValid, ArmValid, Invalid, DoorValid, DoorInvalid, None
};

template<typename TIME>
class Authentication {

public:
    TIME preparationTime;

    Authentication() noexcept {
        preparationTime = TIME("00:00:10");
        state.request = None;
        state.status = Disarmed;
        state.working = false;
    }

    struct state_type {
        Status status;
        Request request;
        PinCheck pinCheck;
        TIME nextInternal;
        bool working;
    };

    state_type state;

    // ports definition
    using input_ports = std::tuple<typename Authentication_defs::in>;
    using output_ports = std::tuple<typename Authentication_defs::out>;

    void internal_transition() {
        if (state.request == Arm || state.request == Disarm) {
            state.working = true;
        } else {
            state.working = false;
            state.request = None;
        }
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
                if (state.request == None) {
                    if (message == 0) {
                        state.status = Disarmed;
                    } else if (message == 1) {
                        state.status = Disarmed;
                    } else {
                        assert(false && "Invalid message passed to Pin Model");
                    }
                    state.request = None;
                    state.pinCheck = None;
                    state.nextInternal = std::numeric_limits<TIME>::infinity();
                }
                break;
            case 1:
                if (message = 1) {

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
        out_aux = Message_t(0, 1);
        get_messages<typename AlarmAdmin_defs::out>(bags).push_back(out_aux);
        return bags;
    }

    TIME time_advance() const {
        TIME next_interval;
        if (state.working) {
            next_interval = preparationTime;
        } else {
            next_interval = std::numeric_limits<TIME>::infinity();
        }

        return next_interval;
    }

    friend std::ostringstream &operator<<(std::ostringstream &os, const typename AlarmAdmin<TIME>::state_type &i) {
        string request = "None";
        string status = "Armed";

        switch (i.request) {
            case Arm:
                request = "Arm";
                break;
            case Disarm:
                request = "Disarm";
                break;
            case Pin:
                request = "Pin";
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

        os << "AlarmAdmin Status:" << status << ";   Request: " << request;
        return os;
    }
};


#endif //SECUREAREAACCESS_AUTHENTICATION_HPP
