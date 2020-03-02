#ifndef DUALVENTER_SNOWFLAKE_HPP
#define DUALVENTER_SNOWFLAKE_HPP

#include <cstdint>

namespace dualventer {

struct Snowflake {
  Snowflake() = delete;
  Snowflake(std::uint_fast64_t _id) : id(_id) {}

  const std::uint_fast64_t id;
};

}

#endif // DUALVENTER_SNOWFLAKE_HPP
