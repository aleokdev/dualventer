#ifndef DUALVENTER_SNOWFLAKE_HPP
#define DUALVENTER_SNOWFLAKE_HPP

#include <cstdint>

namespace dualventer {

struct Snowflake {
  Snowflake();

  const std::uint_fast64_t id;
};

}

#endif // DUALVENTER_SNOWFLAKE_HPP
