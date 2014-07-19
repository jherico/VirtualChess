#pragma once

class Strings {
  Strings() {}
public:
  static std::string replaceAll(const std::string & in, const std::string & from, const std::string & to);
  static std::vector<std::string> & split(const std::string &s, char delim, std::vector<std::string> &elems);
  static std::vector<std::string> split(const std::string &s, char delim);
};
