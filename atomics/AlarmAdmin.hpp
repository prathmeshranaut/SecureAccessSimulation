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
        state.nextInternal = std::numeric_limits<TIME>::infinity();
    }

    struct state_type {
        Status status;
        Request request;
        bool working;
        TIME nextInternal;
    };

    state_type state;

    // ports definition
    using input_ports = std::tuple<typename AlarmAdmin_defs::in, typename AlarmAdmin_defs::authIn>;
    using output_ports = std::tuple<typename AlarmAdmin_defs::out>;

    void internal_transition() {
        if (!state.working) {
            state.request = None;
            state.working = false;
            state.nextInternal = std::numeric_limits<TIME>::infinity();
        }
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        if (get_messages<typename AlarmAdmin_defs::in>(mbs).size() +
            get_messages<typename AlarmAdmin_defs::authIn>(mbs).size() > 1)
            assert(false && "One message per time unit");

        for (const auto &x: get_messages<typename AlarmAdmin_defs::in>(mbs)) {
            if (!state.working) {
                // Retrieve the alarmadmin
                switch (x.message) {
                    case 1:
                        //Arm request
                        state.request = Arm;
                        state.working = true;
                        state.nextInternal = preparationTime;
                        break;
                    case 0:
                        //Disarm request
                        state.request = Disarm;
                        state.working = true;
                        state.nextInternal = preparationTime;
                        break;
                    default:
                        assert(false && "Please enter valid request for alarm");
                }
            } else {
                //Ignore the command as we are already working.
                state.nextInternal -= e;
            }
        }

        for (const auto &x: get_messages<typename AlarmAdmin_defs::authIn>(mbs)) {
            //This is for processing response from PIN
            //cout<<"State: "<<state.working<<" - "<<x.message<<endl;
            if (state.working) {
                if (state.request == Arm && x.message == 1) {
                    //Valid pin, now arm
                    state.status = Armed;
                    state.request = None;
                    state.working = false;
                    state.nextInternal = std::numeric_limits<TIME>::infinity();
                } else if (state.request == Disarm && x.message == 0) {
                    //Valid pin, now disarm
                    state.status = Disarmed;
                    state.request = None;
                    state.working = false;
                    state.nextInternal = std::numeric_limits<TIME>::infinity();
                } else if (x.message == 2) {
                    // Invalid pin
                    state.working = false;
                    state.nextInternal -= e;
                }
            } else {
                //Ignore the PIN message
                state.nextInternal -= e;
            }
        }
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    //Lambda function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        Message_t out_aux;
        if (state.request == Arm) {
            out_aux = Message_t(0, 1);
        } else if (state.request == Disarm) {
            out_aux = Message_t(0, 0);
        }
//        else if (state.request == None) {
//            out_aux = Message_t(0, 2);
//        }
        get_messages<typename AlarmAdmin_defs::out>(bags).push_back(out_aux);
        return bags;
    }

    TIME time_advance() const {
        return state.nextInternal;
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