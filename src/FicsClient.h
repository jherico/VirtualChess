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
  typedef std::vector<GameSummary> GameList;

  struct GameSummary : public GameBase {
    int     ratings[2];
    int     type{ -1 };
    bool    private_{ false };
    bool    rated{ false };

    void parse(const std::string & summary);
    static GameList parseList(const std::string & listString);
  };

  typedef std::vector<GameSummary> GameList;
  typedef std::shared_ptr<GameList> GameListPtr;

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

  typedef std::shared_ptr<GameState> GameStatePtr;
  class Client;
  typedef std::shared_ptr<Client> ClientPtr;

  namespace EventType {
    enum {
      NETWORK,
      GAME_STATE,
      GAME_LIST,
      CHAT,
      FICS_ERROR,
    };
  }
  
  struct EventGameState {
    uint32_t        type;
    GameState*      state;
  };

  struct EventChat {
    uint32_t        type;
    std::string     message;
  };

  struct EventPlayerList {
    uint32_t        type;
  };

  struct EventGameList {
    uint32_t        type;
    GameList*       list;
  };

  struct EventNetwork {
    uint32_t        type;
    bool            connected;
  };

  typedef union Event
  {
    uint32_t        type;
    EventGameState  gameState;
    EventPlayerList playerList;
    EventGameList   gameList;
    EventNetwork    network;
  } Event;

  class Client {
  protected:
    Client() { }
    virtual ~Client() { }
    boost::function<void(const Event&)> callback;

  public:
    static ClientPtr create();
    virtual void connect(const std::string & username, const std::string & passwword) = 0;
    virtual void listGames() = 0;
    virtual void observeGame(int id) = 0;
    virtual void unobserveGame(int id) = 0;
    virtual void setEventHandler(boost::function<void(const Event&)> callback) {
      this->callback = callback;
    }
  };


}
