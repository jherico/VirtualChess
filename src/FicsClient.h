#pragma once

namespace Fics {

  class Game {

  };

  class Client {
  public:
    void connect(const std::string & username, const std::string & passwword);
    std::list<Game> games();
  };

}
