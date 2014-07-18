#include "Common.h"
#include "FicsClient.h"

#include <exception>

using namespace boost;
using namespace std;
using boost::asio::ip::tcp;

namespace Fics {

  namespace BlockDelimiter {
    enum {
      BLOCK_START = 21, /* '\U' */
      BLOCK_SEPARATOR = 22, /* '\V' */
      BLOCK_END = 23, /* '\W' */
      BLOCK_POSE_START = 24, /* \X */
      BLOCK_POSE_END = 25, /* \Y */
    };
  }

  namespace BlockCode {
    enum {
      BLK_NULL = 0,
      BLK_GAME_MOVE = 1,
      BLK_ABORT = 10,
      BLK_ACCEPT = 11,
      BLK_ADDLIST = 12,
      BLK_ADJOURN = 13,
      BLK_ALLOBSERVERS = 14,
      BLK_ASSESS = 15,
      BLK_BACKWARD = 16,
      BLK_BELL = 17,
      BLK_BEST = 18,
      BLK_BNAME = 19,
      BLK_BOARDS = 20,
      BLK_BSETUP = 21,
      BLK_BUGWHO = 22,
      BLK_CBEST = 23,
      BLK_CLEARMESSAGES = 24,
      BLK_CLRSQUARE = 25,
      BLK_CONVERT_BCF = 26,
      BLK_CONVERT_ELO = 27,
      BLK_CONVERT_USCF = 28,
      BLK_COPYGAME = 29,
      BLK_CRANK = 30,
      BLK_CSHOUT = 31,
      BLK_DATE = 32,
      BLK_DECLINE = 33,
      BLK_DRAW = 34,
      BLK_ECO = 35,
      BLK_EXAMINE = 36,
      BLK_FINGER = 37,
      BLK_FLAG = 38,
      BLK_FLIP = 39,
      BLK_FMESSAGE = 40,
      BLK_FOLLOW = 41,
      BLK_FORWARD = 42,
      BLK_GAMES = 43,
      BLK_GETGI = 44,
      BLK_GETPI = 45,
      BLK_GINFO = 46,
      BLK_GOBOARD = 47,
      BLK_HANDLES = 48,
      BLK_HBEST = 49,
      BLK_HELP = 50,
      BLK_HISTORY = 51,
      BLK_HRANK = 52,
      BLK_INCHANNEL = 53,
      BLK_INDEX = 54,
      BLK_INFO = 55,
      BLK_ISET = 56,
      BLK_IT = 57,
      BLK_IVARIABLES = 58,
      BLK_JKILL = 59,
      BLK_JOURNAL = 60,
      BLK_JSAVE = 61,
      BLK_KIBITZ = 62,
      BLK_LIMITS = 63,
      BLK_LINE = 64, /* Not on FICS */
      BLK_LLOGONS = 65,
      BLK_LOGONS = 66,
      BLK_MAILHELP = 67,
      BLK_MAILMESS = 68,
      BLK_MAILMOVES = 69,
      BLK_MAILOLDMOVES = 70,
      BLK_MAILSOURCE = 71,
      BLK_MAILSTORED = 72,
      BLK_MATCH = 73,
      BLK_MESSAGES = 74,
      BLK_MEXAMINE = 75,
      BLK_MORETIME = 76,
      BLK_MOVES = 77,
      BLK_NEWS = 78,
      BLK_NEXT = 79,
      BLK_OBSERVE = 80,
      BLK_OLDMOVES = 81,
      BLK_OLDSTORED = 82,
      BLK_OPEN = 83,
      BLK_PARTNER = 84,
      BLK_PASSWORD = 85,
      BLK_PAUSE = 86,
      BLK_PENDING = 87,
      BLK_PFOLLOW = 88,
      BLK_POBSERVE = 89,
      BLK_PREFRESH = 90,
      BLK_PRIMARY = 91,
      BLK_PROMOTE = 92,
      BLK_PSTAT = 93,
      BLK_PTELL = 94,
      BLK_PTIME = 95,
      BLK_QTELL = 96,
      BLK_QUIT = 97,
      BLK_RANK = 98,
      BLK_RCOPYGAME = 99,
      BLK_RFOLLOW = 100,
      BLK_REFRESH = 101,
      BLK_REMATCH = 102,
      BLK_RESIGN = 103,
      BLK_RESUME = 104,
      BLK_REVERT = 105,
      BLK_ROBSERVE = 106,
      BLK_SAY = 107,
      BLK_SERVERS = 108,
      BLK_SET = 109,
      BLK_SHOUT = 110,
      BLK_SHOWLIST = 111,
      BLK_SIMABORT = 112,
      BLK_SIMALLABORT = 113,
      BLK_SIMADJOURN = 114,
      BLK_SIMALLADJOURN = 115,
      BLK_SIMGAMES = 116,
      BLK_SIMMATCH = 117,
      BLK_SIMNEXT = 118,
      BLK_SIMOBSERVE = 119,
      BLK_SIMOPEN = 120,
      BLK_SIMPASS = 121,
      BLK_SIMPREV = 122,
      BLK_SMOVES = 123,
      BLK_SMPOSITION = 124,
      BLK_SPOSITION = 125,
      BLK_STATISTICS = 126,
      BLK_STORED = 127,
      BLK_STYLE = 128,
      BLK_SUBLIST = 129,
      BLK_SWITCH = 130,
      BLK_TAKEBACK = 131,
      BLK_TELL = 132,
      BLK_TIME = 133,
      BLK_TOMOVE = 134,
      BLK_TOURNSET = 135,
      BLK_UNALIAS = 136,
      BLK_UNEXAMINE = 137,
      BLK_UNOBSERVE = 138,
      BLK_UNPAUSE = 139,
      BLK_UPTIME = 140,
      BLK_USCF = 141,
      BLK_USTAT = 142,
      BLK_VARIABLES = 143,
      BLK_WHENSHUT = 144,
      BLK_WHISPER = 145,
      BLK_WHO = 146,
      BLK_WITHDRAW = 147,
      BLK_WNAME = 148,
      BLK_XKIBITZ = 149,
      BLK_XTELL = 150,
      BLK_XWHISPER = 151,
      BLK_ZNOTIFY = 152,
      BLK_REPLY = 153, /* Not on FICS */
      BLK_SUMMON = 154,
      BLK_SEEK = 155,
      BLK_UNSEEK = 156,
      BLK_SOUGHT = 157,
      BLK_PLAY = 158,
      BLK_ALIAS = 159,
      BLK_NEWBIES = 160,
      BLK_SR = 161,
      BLK_CA = 162,
      BLK_TM = 163,
      BLK_GETGAME = 164,
      BLK_CCNEWSE = 165,
      BLK_CCNEWSF = 166,
      BLK_CCNEWSI = 167,
      BLK_CCNEWSP = 168,
      BLK_CCNEWST = 169,
      BLK_CSNEWSE = 170,
      BLK_CSNEWSF = 171,
      BLK_CSNEWSI = 172,
      BLK_CSNEWSP = 173,
      BLK_CSNEWST = 174,
      BLK_CTNEWSE = 175,
      BLK_CTNEWSF = 176,
      BLK_CTNEWSI = 177,
      BLK_CTNEWSP = 178,
      BLK_CTNEWST = 179,
      BLK_CNEWS = 180,
      BLK_SNEWS = 181,
      BLK_TNEWS = 182,
      BLK_RMATCH = 183,
      BLK_RSTAT = 184,
      BLK_CRSTAT = 185,
      BLK_HRSTAT = 186,
      BLK_GSTAT = 187,
    };
  }

  namespace BlockError {
    enum {
      BLK_ERROR_BADCOMMAND = 512,
      BLK_ERROR_BADPARAMS = 513,
      BLK_ERROR_AMBIGUOUS = 514,
      BLK_ERROR_RIGHTS = 515,
      BLK_ERROR_OBSOLETE = 516,
      BLK_ERROR_REMOVED = 517,
      BLK_ERROR_NOTPLAYING = 518,
      BLK_ERROR_NOSEQUENCE = 519,
      BLK_ERROR_LENGTH = 520,
    };
  }

  static const string PROMPT = "fics% ";


  const char * const INTERFACE_MACRO = "iset defprompt 1\n"
      "iset gameinfo 1\n"
      "iset ms 1\n"
      "iset allresults 1\n"
      "iset startpos 1\n"
      "iset pendinfo 1\n"
      "iset nowrap 1\n"
      "set interface VirtualChess\n"
      "set style 12\n"
      "set bell 0\n"
      "set ptime 0\n"
      "iset block 1\n"
      "1 iset lock 1\n";

  struct Command {
    int code;
    string result;
    boost::condition_variable condition;
    boost::mutex mutex;
  };



  //------------------------------------------------------------------------------
  // 25 (Exam.    0 Friar          0 Friar     ) [ uu  0   0] W:  1
  // 28 ++++ TryMe       1737 Jack       [ su 30  20]  22:27 - 23:17 (29-30) W: 16
  //  2 2274 OldManII    ++++ Peshkin    [ bu  2  12]   2:34 -  1:47 (39-39) B:  3
  // 29 1622 Vman        1609 PopKid     [ sr 10  10]   1:14 -  5:10 (21-22) B: 18
  // 32 1880 Raskapov    1859 RoboDweeb  [ br  2  12]   1:04 -  1:26 ( 9-10) B: 34
  //  1 1878 Roberto     1881 baraka     [psr 45  30]  30:35 - 34:24 (22-22) W: 21
  //
  //  6 games displayed (of 23 in progress)
  //------------------------------------------------------------------------------
  // http://www.freechess.org/Help/HelpFiles/games.html
  void GameSummary::parse(const string & summary) {
    istringstream buf(summary);
    buf >> id;
    string str;
    forEachSide([&](int side){
      buf >> str;
      if (string("++++") == str) {
        ratings[side] = -1;
      } else {
        ratings[side] = atoi(str.c_str());
      }
      buf >> str;
      players[side] = str;
    });
    private_ = 'p' == summary.at(38);
    rated = 'r' == summary.at(40);
    char c = summary.at(39);
    switch (c) {
    case 'b': type = GameType::BLITZ; break;
    case 'l': type = GameType::LIGHTNING; break;
    case 'u': type = GameType::UNTIMED; break;
    case 'e': type = GameType::EXAMINED; break;
    case 's': type = GameType::STANDARD; break;
    case 'w': type = GameType::WILD; break;
    case 'x': type = GameType::ATOMIC; break;
    case 'z': type = GameType::CRAZYHOUSE; break;
    case 'B': type = GameType::BUGHOUSE; break;
    case 'L': type = GameType::LOSERS; break;
    case 'S': type = GameType::SUICIDE; break;
    case 'n': type = GameType::NON_STANDARD; break;
    }
  }

  Piece pieceFromChar(char c) {
    switch (c) {
    case 'p': return WP; 
    case 'r': return WR;
    case 'n': return WN;
    case 'b': return WB;
    case 'q': return WQ;
    case 'k': return WK;
    case 'P': return BP;
    case 'R': return BR;
    case 'N': return BN;
    case 'B': return BB;
    case 'Q': return BQ;
    case 'K': return BK;
    }
    return NP;
  }

  void GameState::parseStyle12(const string & gameState) {
    enum Offsets {
      BOARD = 5,
      RANK_SIZE = 9,
      NEXT_MOVE = BOARD + 8 * RANK_SIZE,
      PAWN_PUSH = NEXT_MOVE + 2,
    };
    assert(0 == gameState.find("<12> "));
    for (int rank = 0; rank < 8; ++rank) {
      const char * rankData = gameState.data() + rank * RANK_SIZE + BOARD;
      for (int col = 0; col < 8; ++col) {
        board.position[rank][col] = pieceFromChar(rankData[col]);
      }
    }
    char c = gameState.at(NEXT_MOVE);
    nextMove = c == 'W' ? Side::WHITE : Side::BLACK;
    istringstream buf(gameState.substr(PAWN_PUSH));
    // -1 if the previous move was NOT a double pawn push, otherwise the chess
    //  board file(numbered 0--7 for a--h) in which the double push was made
    buf >> pawnPush;
    int castle;
    buf >> castle;
    //can White still castle short ? (0 = no, 1 = yes)
    castling[Side::WHITE][CastlingDistance::SHORT] = castle ? true : false;
    buf >> castle;
    //can White still castle long ?
    castling[Side::WHITE][CastlingDistance::LONG] = castle ? true : false;
    buf >> castle;
    //can Black still castle short ?
    castling[Side::BLACK][CastlingDistance::SHORT] = castle ? true : false;
    buf >> castle;
    //can Black still castle long ?
    castling[Side::BLACK][CastlingDistance::LONG] = castle ? true : false;
      
    //*the number of moves made since the last irreversible move.  (0 if last move
    //  was irreversible.If the value is >= 100, the game can be declared a draw
    //  due to the 50 move rule.)
    buf >> reversibleMoves;
    //* The game number
    buf >> id;
    //* White's name
    buf >> players[Side::WHITE];
    //* Black's name
    buf >> players[Side::BLACK];
    //* my relation to this game:
    buf >> relation;

    //* initial time(in seconds) of the match
    buf >> initialTime;
    //* increment In seconds) of the match
    buf >> incrementTime;
    //* White material strength
    buf >> material[Side::WHITE];
    //* Black material strength
    buf >> material[Side::BLACK];
    //* White's remaining time
    buf >> remainingTime[Side::WHITE];
    //* Black's remaining time
    buf >> remainingTime[Side::BLACK];
    //* the number of the move about to be made(standard chess numbering -- White's and Black's first moves are both 1, etc.)
    buf >> moveNumber;
    //* verbose coordinate notation for the previous move("none" if there were none)[note this used to be broken for examined games]
    buf >> lastMoveVerbose;
    //* time taken to make previous move "(min:sec)".
    buf >> lastMoveTime;
    //* pretty notation for the previous move("none" if there is none)
    buf >> lastMovePretty;
    //* flip field for board orientation : 1 = Black at bottom, 0
    buf >> c;
  }

  /**

  */
  class SocketClient : public ::SocketClient {
    typedef std::shared_ptr<Command> CommandPtr;
    typedef map<int, CommandPtr> CommandMap;
    friend class ClientImpl;

    string readBuffer;
    string username, password;
    CommandMap commandMap;
    boost::mutex commandMapLock;
    int commandId{10};
    boost::function<void(const string &)> lineCallback;
  public:
    enum State {
      CONNECT, WAIT_LOGIN, WAIT_PASSWORD, LOGGED_IN, INTERFACE_SETUP, IDLE, FAIL
    };

    State state { WAIT_LOGIN };

    SocketClient(boost::asio::io_service& io_service) : ::SocketClient(io_service) {
    }

    void waitForIdle() {
      while (IDLE != state && ERROR != state) {
        Platform::sleepMillis(500);
      }
    }

    void connect(tcp::resolver::iterator & endpoint_iterator, const std::string & username, const std::string & password) {
      this->username = username;
      this->password = password;
      ::SocketClient::connect(endpoint_iterator);
    }

    string command(const string & commandString) {
      int id = commandId++;

      commandMap[id].reset(new Command());
      Command & c = *(commandMap[id]);

      // Write the command
      write(Platform::format("%d %s\n", id, commandString.c_str()));

      // Wait for the response
      withScopedLock(c.mutex, [&](boost::mutex::scoped_lock & lock){
        c.condition.wait(lock);
      });

      // Extract the result
      string result = c.result;
      commandMap.erase(id);
      if (commandMap.empty()) {
        commandId = 10;
      }
      return result;
    }

  protected:
    void onRead(ReadBuffer & buffer) {
      readBuffer.append(buffer.begin(), buffer.end());
      buffer.clear();
      switch (state) {
      case WAIT_LOGIN:
        if (string::npos != readBuffer.find("login:")) {
          write(username);
          write("\n");
          readBuffer.clear();
          state = WAIT_PASSWORD;
        }
        break;

      case WAIT_PASSWORD:
        if (string::npos != readBuffer.find("password:")) {
          write(password);
          write("\n");
          readBuffer.clear();
          state = LOGGED_IN;
        }
        break;

      case LOGGED_IN:
        write(INTERFACE_MACRO);
        state = IDLE;
        break;

      case IDLE:
        processReadBuffer();
        break;
      }
    }

    void processReadBuffer() {
      parseCommands();
      parseLines();
    }

    void parseLines() {
      int commandStart = readBuffer.find(BlockDelimiter::BLOCK_START);
      int lineEnd;
      while (string::npos != (lineEnd = readBuffer.find("\n\r"))) {
        if (string::npos != commandStart && lineEnd >= commandStart) {
          break;
        }
        parseLine(readBuffer.substr(0, lineEnd));
        readBuffer.erase(0, lineEnd + 2);
        if (string::npos != commandStart ) {
          commandStart -= lineEnd;
        }
      }
    }

    void parseCommands() {
      int startCommand, endCommand;
      while (string::npos != (startCommand = readBuffer.find(BlockDelimiter::BLOCK_START))) {
        endCommand = readBuffer.find(BlockDelimiter::BLOCK_END, startCommand);
        // Incomplete command in buffer
        if (string::npos == endCommand) {
          break;
        }
        string commandBuffer = readBuffer.substr(startCommand + 1, endCommand - (startCommand + 1));
        readBuffer.erase(startCommand, endCommand + 1);
        parseCommand(commandBuffer);
      }
    }


    void parseCommand(const string & command) {
      int idSep = command.find(BlockDelimiter::BLOCK_SEPARATOR);
      int codeSep = command.find(BlockDelimiter::BLOCK_SEPARATOR, idSep + 1);

      commandId = atoi(command.substr(0, idSep++).c_str());
      if (commandMap.count(commandId)) {
        Command & c = *(commandMap[commandId]);
        c.code = atoi(command.substr(idSep, codeSep++ - idSep).c_str());
        c.result = command.substr(codeSep);
        withScopedLock(c.mutex, [&](boost::mutex::scoped_lock & lock){
          c.condition.notify_one();
        });
      } else {
        SAY("Unrequested command %d result: ", commandId);
        SAY(command.substr(codeSep).c_str());
        SAY("=================");
      }
    }

    void parseLine(const string & line) {
      if (lineCallback) {
        int start = 0, promptPos = line.find(PROMPT);
        while (string::npos != promptPos) {
          start = promptPos + PROMPT.size();
          promptPos = line.find(PROMPT, start);
        }
        if (start == line.size()) {
          return;
        }
        lineCallback(line.substr(start));
      }
    }
  };

  vector<string> parseFicsLines(const std::string lines) {
    string lineEndings = (0 == lines.find("\r\n") ? "\r\n" : "\n\r");
    return Strings::split(Strings::replaceAll(lines, lineEndings, "\n"), '\n');
  }

  GameList GameSummary::parseList(const std::string & gameListString) {
    GameList result;
    vector<string> lines = parseFicsLines(gameListString);
    for_each(lines.begin(), lines.end(), [&](vector<string>::const_reference s){
      if (s.empty()) {
        return;
      }
      if (string::npos != s.find("(Exam.")) {
        return;
      }
      if (0 == s.find("  ")) {
        return;
      }
      GameSummary summary;
      summary.parse(s);
      result.push_back(summary);
    });
    return result;
  }


  class ClientImpl : public Client {
    boost::asio::io_service io_service;
    boost::asio::io_service::work work;
    boost::thread serviceThread;
    SocketClient client;
    boost::function<void(const GameState&)> gameStateCallback;

    void onLine(const std::string & line) {
      if (gameStateCallback && 0 == line.find("<12> ")) {
          gameStateCallback(GameState(line));
      } else {
        SAY(line.c_str());
      }
    }

  public:
    ClientImpl() : client(io_service), work(io_service),
      serviceThread(boost::bind(&boost::asio::io_service::run, &io_service)) {
      client.lineCallback = boost::bind(&ClientImpl::onLine, this, _1);
    }

    virtual ~ClientImpl() {
      client.close(); // close the telnet client connection
      serviceThread.join(); // wait for the IO service thread to close
    }

    virtual void connect(const std::string & username, const std::string & password) {
      tcp::resolver resolver(io_service);
      tcp::resolver::query query("freechess.org", "5000");
      tcp::resolver::iterator iterator = resolver.resolve(query);
      client.connect(iterator, username, password);
      client.waitForIdle();
    }

    virtual GameList games() {
      string gamesResult = client.command("games");
      return GameSummary::parseList(gamesResult);
    }

    virtual bool observe(int id) {
      string observeResult = client.command(Platform::format("observe %d", id));
      vector<string> lines = parseFicsLines(observeResult);
      for (int i = 0; i < lines.size(); ++i) {
        if (0 == lines[i].find("<12> ")) {
          if (gameStateCallback) {
            gameStateCallback(GameState(lines[i]));
          }
          return true;
        }
      }
      return false;
    }

    virtual void setGameCallback(boost::function<void(const GameState&)> callback) {
      gameStateCallback = callback;
    }

  };

  ClientPtr Client::create() {
    return ClientPtr(new ClientImpl());
  }

  class Test {
  public:
    int run() {
      string gameListString = Platform::getResourceString(Resource::MISC_GAMELIST_TXT);
      GameList gameList = GameSummary::parseList(gameListString);
      /*
      boost::asio::io_service io_service;
      boost::asio::io_service::work work(io_service);
      boost::thread serviceThread(boost::bind(&boost::asio::io_service::run, &io_service));
      tcp::resolver resolver(io_service);
      tcp::resolver::query query("freechess.org", "5000");
      tcp::resolver::iterator iterator = resolver.resolve(query);

      SocketClient client(io_service);
      client.connect(iterator, "ariha", "borklu");
      client.waitForIdle();
      //string testGame("<12> r-bqk--r pppp-ppp -----n-- --b-P--- -------- --N----- PPP-PPPP R-BQKB-R W -1 1 1 1 1 1 8 GuestLYKS GuestGJWZ 0 5 5 36 35 209 289 6 B/f8-c5 (0:13) Bc5 0 1 0");
      //GameState state;
      //state.parseStyle12(testGame);
      string gamesResult = client.command("games");
      SAY(gamesResult.c_str());
      SAY("=================");
      string observeResult = client.command("observe 8");
      SAY(observeResult.c_str());
      SAY("=================");
      while (1) {
        Platform::sleepMillis(100);
      }
      */
      return 0;
    }
  };
}

//RUN_APP(Fics::Test);

