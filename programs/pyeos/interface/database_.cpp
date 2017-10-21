#include "database_.hpp"
using chainbase::database;
using namespace std;
// using namespace eos;

int database_create(string& shared_memory_dir, int read_only,
                    int shared_memory_size, database** db) {
   try {
      *db = new chainbase::database(
          shared_memory_dir,
          read_only ? database::read_only : database::read_write,
          shared_memory_size);
      return 0;
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   }
   return -1;
}
