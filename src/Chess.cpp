#include "Common.h"

namespace Chess {
  Piece Board::STANDARD_BOARD[64] = {
    WR, WN, WB, WQ, WK, WB, WN, WR,
    WP, WP, WP, WP, WP, WP, WP, WP,
    NP, NP, NP, NP, NP, NP, NP, NP,
    NP, NP, NP, NP, NP, NP, NP, NP,
    NP, NP, NP, NP, NP, NP, NP, NP,
    NP, NP, NP, NP, NP, NP, NP, NP,
    BP, BP, BP, BP, BP, BP, BP, BP,
    BR, BN, BB, BQ, BK, BB, BN, BR,
  };

  static std::string TYPE_NAMES[] = {
      "Unknown",
      "Standard",
      "Nonstandard",
      "Untimed",
      "Examined",
      "Lightning",
      "Blitz",
      "Suicide",
      "Wild",
      "Crazyhouse",
      "Bughouse",
      "Losers",
      "Atomic",
  };

  const std::string & getTypeName(int gameType) {
    return TYPE_NAMES[gameType + 1];
  }


}
