#ifndef DUALVENTER_SNOWFLAKE_HPP
#define DUALVENTER_SNOWFLAKE_HPP

#include <cstdint>
#include <string>

namespace dualventer {

struct Snowflake {
  Snowflake() = delete;
  Snowflake(std::uint_fast64_t _id) : id(_id) {}

  std::string to_string() {
    return std::to_string(id);
  };

  std::uint_fast64_t id;
};

}

#endif // DUALVENTER_SNOWFLAKE_HPP
