#include "ipc_.hpp"
#include <boost/interprocess/ipc/message_queue.hpp>

using namespace boost::interprocess;

void* ipc_create(std::string& name, int max_num_msg, int max_msg_size) {
   message_queue* mq;
   try {
      message_queue::remove(name.c_str());
      mq = new message_queue(create_only, name.c_str(), max_num_msg, max_msg_size);
      return mq;
   } catch (interprocess_exception &ex) {
//      message_queue::remove();
      std::cout << ex.what() << std::endl;
      return 0;
  }

}

void* ipc_open(std::string& name) {
   message_queue* mq;
   try {
      mq = new message_queue(open_only, name.c_str());
      return mq;
   } catch (interprocess_exception &ex) {
//      message_queue::remove();
      std::cout << ex.what() << std::endl;
      return 0;
  }

}

int ipc_receive(void* _mq, vector<char>& buffer) {
   message_queue* mq = (message_queue*)_mq;
   try {
      unsigned int priority;
      message_queue::size_type receive_size;
      mq->receive((char*)buffer.data(), buffer.size(), receive_size, priority);
      return receive_size;
   } catch (interprocess_exception &ex) {
//      message_queue::remove();
      std::cout << ex.what() << std::endl;
      return 0;
  }
}

int ipc_send(void* _mq, string& buffer) {
   message_queue* mq = (message_queue*)_mq;
   try{
      mq->send(buffer.c_str(), buffer.length(), 0);
      return 1;
   } catch(interprocess_exception &ex) {
       std::cout << ex.what() << std::endl;
       return 0;
   }
}

void ipc_remove(string& name) {
   message_queue::remove(name.c_str());
}

