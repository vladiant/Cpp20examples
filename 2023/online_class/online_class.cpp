// onlineClass.cpp

#include <algorithm>
#include <chrono>
#include <format>
#include <iomanip>
#include <iostream>

using namespace std::chrono_literals;

template <typename ZonedTime>
auto getMinutes(const ZonedTime& zonedTime) {
  return std::chrono::floor<std::chrono::minutes>(zonedTime.get_local_time());
}

void printStartEndTimes(const std::chrono::local_days& localDay,
                        const std::chrono::hours& h,
                        const std::chrono::hours& durationClass,
                        const std::initializer_list<std::string>& timeZones) {
  std::chrono::zoned_time startDate{std::chrono::current_zone(), localDay + h};
  std::chrono::zoned_time endDate{std::chrono::current_zone(),
                                  localDay + h + durationClass};
  std::cout << "Local time: [" << getMinutes(startDate) << ", "
            << getMinutes(endDate) << "]" << '\n';

  auto longestStringSize =
      std::max(timeZones, [](const std::string& a, const std::string& b) {
        return a.size() < b.size();
      }).size();
  for (auto timeZone : timeZones) {
    std::cout << "  " << std::setw(longestStringSize + 1) << std::left
              << timeZone << "["
              << getMinutes(std::chrono::zoned_time(timeZone, startDate))
              << ", " << getMinutes(std::chrono::zoned_time(timeZone, endDate))
              << "]" << '\n';

    /*
    std::cout << std::vformat("  {:} [{:}, {:}]\n",
                                    std::make_format_args(timeZone,
                                    getMinutes(std::chrono::zoned_time(timeZone,
    startDate)), getMinutes(std::chrono::zoned_time(timeZone, endDate)) ) );
    */
  }
}

int main() {
  using namespace std::string_literals;

  std::cout << '\n';

  constexpr auto classDay{std::chrono::year(2021) / 2 / 1};
  constexpr auto durationClass = 4h;
  auto timeZones = {
      "America/Los_Angeles"s, "America/Denver"s,   "America/New_York"s,
      "Europe/London"s,       "Europe/Minsk"s,     "Europe/Moscow"s,
      "Asia/Kolkata"s,        "Asia/Novosibirsk"s, "Asia/Singapore"s,
      "Australia/Perth"s,     "Australia/Sydney"s};

  for (auto startTime : {7h, 13h, 17h}) {
    printStartEndTimes(std::chrono::local_days{classDay}, startTime,
                       durationClass, timeZones);
    std::cout << '\n';
  }

  std::cout << '\n';
}
