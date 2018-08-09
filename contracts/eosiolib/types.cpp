#include "types.hpp"
#include "privileged.hpp"
#include "action.hpp"

bool operator==(const checksum256& lhs, const checksum256& rhs) {
   return memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}

bool operator==(const checksum160& lhs, const checksum160& rhs) {
   return memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}


bool operator!=(const checksum160& lhs, const checksum160& rhs) {
   return memcmp(&lhs, &rhs, sizeof(lhs)) != 0;
}

namespace eosio {
   void require_auth(const permission_level& level) {
      require_auth2( level.actor, level.permission );
   }
}
