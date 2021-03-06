#pragma once

namespace Chess {
  namespace GameType{
    enum {
      STANDARD,
      NON_STANDARD,
      UNTIMED,
      EXAMINED,
      LIGHTNING,
      BLITZ,
      SUICIDE,
      WILD,
      CRAZYHOUSE,
      BUGHOUSE,
      LOSERS,
      ATOMIC,
    };
  }

  namespace Side {
    enum {
      WHITE, BLACK,
    };
  }

  namespace CastlingDistance {
    enum {
      SHORT, LONG,
    };
  }

  enum Piece {
      NP = 0x00,
      WP = 0x01,
      WR,
      WN,
      WB,
      WQ,
      WK,

      BP = 0x11,
      BR,
      BN,
      BB,
      BQ,
      BK,
  };

  struct Board {
    Piece position[8][8];
    static Piece STANDARD_BOARD[64];

    Board() {
      memcpy(position, STANDARD_BOARD, 64 * sizeof(Piece));
    }
  };

  template <typename Function>
  void forEachSquare(Function f) {
    for (int row = 0; row < 8; ++row) {
      for (int col = 0; col < 8; ++col) {
        f(row, col);
      }
    }
  }

  template <typename Function>
  void forEachSide(Function f) {
    f(Chess::Side::WHITE);
    f(Chess::Side::BLACK);
  }

  const std::string & getTypeName(int gameType);
}
