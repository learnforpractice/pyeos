

#include <boost/python.hpp>

#include <eos/chain/message.hpp>
#include <eos/chain/types.hpp>
#include <eos/types/generated.hpp>
#include <eos/types/native.hpp>
#include <numeric>

using namespace boost::python;
// using namespace eosio::chain;
using namespace eosio::types;

struct World {
   void set(std::string msg) { this->msg = msg; }
   std::string greet() { return msg; }
   std::string msg;
};

BOOST_PYTHON_MODULE(hello) {
   class_<World>("World").def("greet", &World::greet).def("set", &World::set);

   class_<Transaction>(
       "Transaction",
       init<const UInt16&, const UInt32&, const Time&,
            const Vector<AccountName>&, const Vector<AccountName>&,
            const Vector<Message>&>())
       .def_readwrite("refBlockNum", &Transaction::refBlockNum)
       .def_readwrite("refBlockPrefix", &Transaction::refBlockPrefix)
       .def_readwrite("expiration", &Transaction::expiration)
       .def_readwrite("scope", &Transaction::scope)
       .def_readwrite("messages", &Transaction::messages)
       //        .def_readwrite("signatures", &Transaction::signatures)
       ;
   /*
           UInt16                           refBlockNum;
           UInt32                           refBlockPrefix;
           Time                             expiration;
           Vector<AccountName>              scope;
           Vector<AccountName>              readscope;
           Vector<Message>                  messages;
   */
}
