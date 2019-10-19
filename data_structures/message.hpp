#ifndef BOOST_SIMULATION_MESSAGE_HPP
#define BOOST_SIMULATION_MESSAGE_HPP

#include <assert.h>
#include <iostream>
#include <string>

using namespace std;

/*******************************************/
/**************** Message_t ****************/
/*******************************************/
struct Message_t{
  Message_t(){}
  Message_t(int i_port, int i_message)
   :port(i_port), message(i_message){}

  	int   port;
  	int   message;
};

istream& operator>> (istream& is, Message_t& msg);

ostream& operator<<(ostream& os, const Message_t& msg);

enum Status {
    Disarmed, Armed
};
enum Request {
    Arm, Disarm, Pin, None
};

#endif // BOOST_SIMULATION_MESSAGE_HPP