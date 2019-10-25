//Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

//Time class header
#include <NDTime.hpp>

//Messages structures
#include "../data_structures/message.hpp"

//Atomic model headers
#include <cadmium/basic_model/pdevs/iestream.hpp> //Atomic model for inputs
#include "../atomics/AlarmAdmin.hpp"
#include "../atomics/Authentication.hpp"
#include "../atomics/Display.hpp"

//C++ headers
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>


using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

/***** Define input port for coupled models *****/
struct inp_command : public in_port<Message_t> {
};
struct inp_sysadmin : public in_port<Message_t> {

};
struct inp_1 : public in_port<Message_t> {
};
struct inp_2 : public in_port<Message_t> {
};
/***** Define output ports for coupled model *****/
struct outp_ack : public out_port<Message_t> {
};
struct outp_1 : public out_port<Message_t> {
};
struct outp_2 : public out_port<Message_t> {
};
struct outp_msg : public out_port<Message_t> {
};

/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Message_t : public iestream_input<Message_t,T> {
public:
    InputReader_Message_t () = default;
    InputReader_Message_t (const char* file_path) : iestream_input<Message_t,T>(file_path) {}
};

int main(int argc, char **argv) {
    const char *i_input = "../input_data/saa_input_test.txt";
    shared_ptr <dynamic::modeling::model> input_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_Message_t, TIME, const char *>(
            "input_reader", move(i_input));

    /****** Alarm Admin atomic model instantiation *******************/
    shared_ptr <dynamic::modeling::model> alarmAdmin = dynamic::translate::make_dynamic_atomic_model<AlarmAdmin, TIME>(
            "alarmAdmin");

    /****** Authentication atomic model instantiation *******************/
    shared_ptr <dynamic::modeling::model> authentication = dynamic::translate::make_dynamic_atomic_model<Authentication, TIME>(
            "authentication");

    /****** Display atomic model instantiation *******************/
    shared_ptr <dynamic::modeling::model> display = dynamic::translate::make_dynamic_atomic_model<Display, TIME>(
            "display");

    /******* SystemAdmin COUPLED MODEL********/
    dynamic::modeling::Ports iports_SystemAdmin = {typeid(inp_sysadmin)};
    dynamic::modeling::Ports oports_SystemAdmin = {typeid(outp_1), typeid(outp_2)};
    dynamic::modeling::Models submodels_SystemAdmin = {alarmAdmin, authentication};
    dynamic::modeling::EICs eics_SystemAdmin = {
            dynamic::translate::make_EIC<inp_sysadmin, AlarmAdmin_defs::in>("alarmAdmin"),
    };
    dynamic::modeling::EOCs eocs_SystemAdmin = {
            dynamic::translate::make_EOC<AlarmAdmin_defs::out, outp_1>("alarmAdmin"),
            dynamic::translate::make_EOC<Authentication_defs::displayOut, outp_2>("authentication")
    };
    dynamic::modeling::ICs ics_SystemAdmin = {
            dynamic::translate::make_IC<AlarmAdmin_defs::out, Authentication_defs::in>("alarmAdmin", "authentication"),
            dynamic::translate::make_IC<Authentication_defs::out, AlarmAdmin_defs::authIn>("authentication",
                                                                                           "alarmAdmin")
    };
    shared_ptr <dynamic::modeling::coupled<TIME>> SYSTEMADMIN;
    SYSTEMADMIN = make_shared < dynamic::modeling::coupled < TIME >> (
            "SystemAdmin", submodels_SystemAdmin, iports_SystemAdmin,
                    oports_SystemAdmin, eics_SystemAdmin, eocs_SystemAdmin, ics_SystemAdmin
    );


    /******* SAA SIMULATOR COUPLED MODEL********/
    dynamic::modeling::Ports iports_SAA = {typeid(inp_command)};
    dynamic::modeling::Ports oports_SAA = {typeid(outp_msg)};
    dynamic::modeling::Models submodels_SAA = {SYSTEMADMIN, display};
    dynamic::modeling::EICs eics_SAA = {
            cadmium::dynamic::translate::make_EIC<inp_command, inp_sysadmin>("SystemAdmin")
    };
    dynamic::modeling::EOCs eocs_SAA = {
            dynamic::translate::make_EOC<Display_defs::out, outp_msg>("display"),
    };
    dynamic::modeling::ICs ics_SAA = {
            dynamic::translate::make_IC<outp_1, Display_defs::in>("SystemAdmin", "display")
    };
    shared_ptr <dynamic::modeling::coupled<TIME>> SAA;
    SAA = make_shared < dynamic::modeling::coupled < TIME >> (
            "SAA", submodels_SAA, iports_SAA, oports_SAA, eics_SAA, eocs_SAA, ics_SAA
    );


    /*******TOP COUPLED MODEL********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(outp_msg)};
    dynamic::modeling::Models submodels_TOP = {input_reader, SAA};
    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP = {
            dynamic::translate::make_EOC<outp_msg, outp_msg>("SAA")
    };
    dynamic::modeling::ICs ics_TOP = {
            dynamic::translate::make_IC<iestream_input_defs<Message_t>::out, inp_command>("input_reader", "SAA")
    };
    shared_ptr <cadmium::dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared < dynamic::modeling::coupled < TIME >> (
            "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/SAA_output_messages.txt");
    struct oss_sink_messages {
        static ostream &sink() {
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/SAA_output_state.txt");
    struct oss_sink_state {
        static ostream &sink() {
            return out_state;
        }
    };

    using state=logger::logger <logger::logger_state, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using log_messages=logger::logger <logger::logger_messages, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_mes=logger::logger <logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_sta=logger::logger <logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_state>;

    using logger_top=logger::multilogger<state, log_messages, global_time_mes, global_time_sta>;

    /************** Runner call ************************/
    dynamic::engine::runner <NDTime, logger_top> r(TOP, {0});
    r.run_until_passivate();
    return 0;
}