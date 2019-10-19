//
// Created by Prathmesh Ranaut on 10/17/19.
//

#ifndef __ALARMADMIN_HPP__
#define __ALARMADMIN_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

struct AlarmAdmin_defs {
    struct out : public out_port<Message_t> {
    };
    struct in : public in_port<Message_t> {
    };
    struct authIn : public in_port<Message_t> {
    };
};

template<typename TIME>
class AlarmAdmin {

public:
    TIME preparationTime;

    AlarmAdmin() noexcept {
        preparationTime = TIME("00:00:10");
        state.request = None;
        state.status = Status::Disarmed;
        state.working = false;
    }

    struct state_type {
        Status status;
        Request request;
        bool working;
    };

    state_type state;

    // ports definition
    using input_ports = std::tuple<typename AlarmAdmin_defs::in, typename AlarmAdmin_defs::authIn>;
    using output_ports = std::tuple<typename AlarmAdmin_defs::out>;

    void internal_transition() {
        if (state.request == Arm || state.request == Disarm) {
            state.working = true;
        } else {
            state.working = false;
            state.request = None;
        }
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        if (get_messages<typename AlarmAdmin_defs::in>(mbs).size() > 1)
            assert(false && "One message per time unit");
        vector <Message_t> message_port_in;
        message_port_in = get_messages<typename AlarmAdmin_defs::in>(mbs);

        assert(false && message_port_in.size());

        int port = message_port_in[0].port;
        int message = message_port_in[0].message;

        switch (port) {
            case Arm:
                if (message == 1 && state.status == Status::Disarmed) {
                    state.request = Arm;
                    state.working = true;
                } else {
                    assert("Invalid State sent to Arm - AlarmAdmin");
                }
                break;
            case Disarm:
                if (message == 1 && state.status == Status::Armed) {
                    state.request = Disarm;
                    state.working = true;
                } else {
                    assert("Invalid State sent to Disarm - AlarmAdmin");
                }
                break;
            case Pin:
                if (message == 1 && state.status == Status::Disarmed) {
                    state.request = None;
                    state.status = Status::Armed;
                    state.working = true;
                }
                if (message == 0 && state.status == Status::Armed) {
                    state.request = None;
                    state.status = Status::Disarmed;
                    state.working = true;
                }
                if (message == 2) {
                    //Invalid pin
                    state.request = None;
                    state.working = true;
                }
                break;
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
        string status = "Status::Armed";

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
            case Status::Armed:
                status = "Status::Armed";
                break;
            case Status::Disarmed:
                status = "Disarmed";
                break;
        }

        os << "AlarmAdmin Status:" << status << ";   Request: " << request;
        return os;
    }
};

#endif