#pragma once

namespace Fics {
  using namespace Chess;

  enum GameRelation {
    ISOLATED = -3,
    EXAMINATION_OBSERVER = -2,
    EXAMINER = 2,
    PLAYING_OPPONENT_MOVE = -1,
    PLAYING_MY_MOVE = 1,
    OBSERVING = 0
  };


  struct GameBase {
    int          id;
    std::string  players[2];
  };

  // http://www.freechess.org/Help/HelpFiles/games.html
  struct GameSummary;
  typedef std::list<GameSummary> GameList;

  struct GameSummary : public GameBase {
    int     ratings[2];
    int     type{ -1 };
    bool    private_{ false };
    bool    rated{ false };

    void parse(const std::string & summary);
    static GameList parseList(const std::string & listString);
  };

  typedef std::list<GameSummary> GameList;

  // http://www.freechess.org/Help/HelpFiles/style12.html
  struct GameState : public GameBase {
    Board   board;
    int     nextMove;
    int     pawnPush;
    bool    castling[2][2];
    int     reversibleMoves;
    int     relation;
    int     initialTime;
    int     incrementTime;
    int     material[2];
    int     remainingTime[2];
    int     moveNumber;
    std::string  lastMoveVerbose;
    std::string  lastMoveTime;
    std::string  lastMovePretty;

    GameState() {

    }

    GameState(const std::string & gameState) {
      parseStyle12(gameState);
    }

    void parseStyle12(const std::string & gameState);
  };

  class Client;
  typedef std::shared_ptr<Client> ClientPtr;

  class Client {
  protected:
    Client() { }
    virtual ~Client() { }

  public:
    static ClientPtr create();
    virtual void connect(const std::string & username, const std::string & passwword) = 0;
    virtual GameList games() = 0;
    virtual bool observe(int id) = 0;
    virtual void setGameCallback(boost::function<void(const GameState&)> callback) = 0;
  };


}
