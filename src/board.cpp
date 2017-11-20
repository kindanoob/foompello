#include "board.h"


const u64 kInitialWhiteBitboard = 0b0000000000000000000000000001000000001000000000000000000000000000ull;
const u64 kInitialBlackBitboard = 0b0000000000000000000000000000100000010000000000000000000000000000ull;
const u64 kFullBitboard = 0b1111111111111111111111111111111111111111111111111111111111111111ull;


Board::Board() {
    white_bitboard_ = kInitialWhiteBitboard;
    black_bitboard_ = kInitialBlackBitboard;
}

Board::Board(const Board& board) {
    white_bitboard_ = board.white_bitboard_;
    black_bitboard_ = board.black_bitboard_;

}

u64 Board::Occupied() {
  return (white_bitboard_ | black_bitboard_);
}

u64 Board::Empty() {
  return ~(white_bitboard_ | black_bitboard_);
}

int Board::BlackScore() {
    return BitboardPopcount(black_bitboard_);
}

int Board::WhiteScore() {
    return BitboardPopcount(white_bitboard_);
}

u64 Board::BitboardPopcount(u64 bitboard) {
    int cnt = 0;
    while(bitboard) {
        bitboard &= (bitboard - 1);
        ++cnt;
    }
    return cnt;
}

u64 Board::BitboardPopcount2(u64 b) {
     b = (b & 0x5555555555555555ull) + ((b >> 1) & 0x5555555555555555ull);
     b = (b & 0x3333333333333333ull) + ((b >> 2) & 0x3333333333333333ull);
     b += (b >> 4) & 0x0F0F0F0F0F0F0F0Full;
     b += (b >> 8);
     b += (b >> 16);
     b += ((b >> 32) & 0x0000007Full);
     return static_cast<int>(b);
}

bool CompareByEvalAscending(const ScoreMove& score_move_1, const ScoreMove& score_move_2) {
    return score_move_1.score < score_move_2.score;
}


bool CompareByEvalDescending(const ScoreMove& score_move_1, const ScoreMove& score_move_2) {
    return score_move_1.score > score_move_2.score;
}


double Board::EvalBoard(int move_number, int color) {
    if (move_number < 59) {        
        u64 empty = ~(black_bitboard_ | white_bitboard_);

        int corners_black = BitboardPopcount(black_bitboard_ & 0b1000000100000000000000000000000000000000000000000000000010000001ull);
        int corners_white = BitboardPopcount(white_bitboard_ & 0b1000000100000000000000000000000000000000000000000000000010000001ull);


        int x_square_black = BitboardPopcount(black_bitboard_ & 0b0000000001000010000000000000000000000000000000000100001000000000ull);
        int x_square_white = BitboardPopcount(white_bitboard_ & 0b0000000001000010000000000000000000000000000000000100001000000000ull);

        u64 adjacent_black = (West(black_bitboard_) | East(black_bitboard_) | North(black_bitboard_) | South(black_bitboard_) | NorthEast(black_bitboard_) | NorthWest(black_bitboard_) | SouthEast(black_bitboard_) | SouthWest(black_bitboard_)) & empty & 0b1111111110111101111111111111111111111111111111111011110111111111ull;
        u64 adjacent_white = (West(white_bitboard_) | East(white_bitboard_) | North(white_bitboard_) | South(white_bitboard_) | NorthEast(white_bitboard_) | NorthWest(white_bitboard_) | SouthEast(white_bitboard_) | SouthWest(white_bitboard_)) & empty & 0b1111111110111101111111111111111111111111111111111011110111111111ull;
        int num_adjacent_black = BitboardPopcount(adjacent_black);//number of discs adjacent to black discs in all directions
        int num_adjacent_white = BitboardPopcount(adjacent_white);

        
        return color * (((num_adjacent_white - num_adjacent_black) << 1) + ((corners_black - corners_white) << 6) + ((x_square_black - x_square_white) << 2));
        
    } else {
        return color * (BitboardPopcount(black_bitboard_) - BitboardPopcount(white_bitboard_));
    }
}

double Board::EvalBoardSort(int color) {
    u64 empty = ~(black_bitboard_ | white_bitboard_);
    u64 adjacent_black = (West(black_bitboard_) | East(black_bitboard_) | North(black_bitboard_) | South(black_bitboard_) | NorthEast(black_bitboard_) | NorthWest(black_bitboard_) | SouthEast(black_bitboard_) | SouthWest(black_bitboard_)) & empty & 0b1111111110111101111111111111111111111111111111111011110111111111ull;
    u64 adjacent_white = (West(white_bitboard_) | East(white_bitboard_) | North(white_bitboard_) | South(white_bitboard_) | NorthEast(white_bitboard_) | NorthWest(white_bitboard_) | SouthEast(white_bitboard_) | SouthWest(white_bitboard_)) & empty & 0b1111111110111101111111111111111111111111111111111011110111111111ull;
    int num_adjacent_black = BitboardPopcount(adjacent_black);//number of discs adjacent to black discs in all directions
    int num_adjacent_white = BitboardPopcount(adjacent_white);    
    return color * (num_adjacent_white - num_adjacent_black);
}


void Board::OrderMovesByEvalDescending(std::vector<ScoreMove>& possible_move_score_pairs) {
    std::sort(possible_move_score_pairs.begin(), 
              possible_move_score_pairs.end(), 
              [](const ScoreMove& a, const ScoreMove& b) {
                  return a.score > b.score;
              });
}


std::vector<ScoreMove> Board::GenPossibleMoveScorePairsOne(const std::vector<u64> &possible_moves_indicators,
                                                           int color,
                                                           int move_number) {
    std::vector<ScoreMove> possible_move_score_pairs;
    possible_move_score_pairs.reserve(64);
    ScoreMove temp_score_move;
    int m_size = possible_moves_indicators.size();
    u64 current_move;
    u64 tiles_to_flip;
    for (int i = 0; i < m_size; i++) {
        current_move = possible_moves_indicators[i];
        tiles_to_flip = MakeMoveImproved(color, current_move);
        temp_score_move.move = current_move;
        temp_score_move.score = EvalBoardSort(color);
        UnmakeMove(color, current_move, tiles_to_flip);
        possible_move_score_pairs.push_back(temp_score_move);
    }
    return possible_move_score_pairs;
}



int Board::BitScanForward(u64 bb) {
   return kIndex64[((bb ^ (bb - 1)) * kDebruijn64) >> 58];
}


std::vector<u64> Board::GenPossibleMovesIndicators(u64 data) {
    std::vector<u64> res;
    res.reserve(BitboardPopcount(data));
    while (data) {
        res.emplace_back(kIndicatorBitboards[BitScanForward(data)]);
        u64 m = data & (0 - data);
        data ^= m;
    }
    return res;
}


//ANOTHER VERSION: 
/*
std::vector<u64> Board::GenPossibleMovesIndicators(u64 possible_moves) {
    //u64 temp = possible_moves;
    std::vector<u64> possible_moves_indicators;
    ScoreMove temp_score_move;
    for (int i = 0; i < 64; i++) {
        if ((possible_moves & kIndicatorBitboards[i]) != 0ull) {
            possible_moves_indicators.push_back(kIndicatorBitboards[i]);
        }
    }
    return possible_moves_indicators;
}
*/





std::vector<u64> Board::GetScore() {
    return {BitboardPopcount(white_bitboard_), BitboardPopcount(white_bitboard_)};
}


void Board::Reset() {
    white_bitboard_ = kInitialWhiteBitboard;
    black_bitboard_ = kInitialBlackBitboard;
}

bool Board::IsOnBoard(u64 b) {    
    return (b & kFullBitboard);
}

bool Board::IsOnCorner(u64 my_move) {
    u64 corner_board = 0b1000000100000000000000000000000000000000000000000000000010000001ull;
    return (my_move & corner_board);
}

u64 Board::RandomComputerMove(int color) {
    u64 possible_computer_moves = this->ValidMoves(color);
    u64 temp = possible_computer_moves;
    std::vector<int> index_vector;
    for (int i = 0; i < 64; i++)
    {
        if ((temp % 2) == 1) {
            index_vector.push_back(i);
        }
        temp /= 2;
    }
    int rand_index = rand() % index_vector.size();
    int rand_bit = index_vector[rand_index];
    u64 b = possible_computer_moves;
    std::bitset<64> a1 (b >> 32);
    std::bitset<64> a2 (b & 0b0000000000000000000000000000000011111111111111111111111111111111ull);
    std::bitset<64> a ((a1 << 32) | a2);
    return 0b0000000000000000000000000000000000000000000000000000000000000001ull << rand_bit;

}


void Board::ConvertToBinary(u64 n) /*Alex says: Good to use unsigned int if u can*/
{
    if (n / 2 != 0) {
        ConvertToBinary(n / 2);
    }
    std::cout << n % 2;
}

u64 Board::WhiteBitboard() {
    return white_bitboard_;
}

u64 Board::BlackBitboard() {
    return black_bitboard_;
}

u64 Board::North(u64 x) {
  return x << 8;
}

u64 Board::North2(u64 x) {
  return x << 16;
}

u64 Board::North4(u64 x) {
  return x << 32;
}

u64 Board::South(u64 x) {
  return x >> 8;
}

u64 Board::South2(u64 x) {
  return x >> 16;
}

u64 Board::South4(u64 x) {
  return x >> 32;
}

u64 Board::East(u64 x) {
  return (x & 0xfefefefefefefefeull) >> 1;
}

u64 Board::East2(u64 x) {
  return (x & 0xfcfcfcfcfcfcfcfcull) >> 2;
}

u64 Board::East4(u64 x) {
  return (x & 0xf0f0f0f0f0f0f0f0ull) >> 4;
}

u64 Board::West(u64 x) {
  return (x & 0x7f7f7f7f7f7f7f7full) << 1;
}

u64 Board::West2(u64 x) {
  return (x & 0x3f3f3f3f3f3f3f3full) << 2;
}

u64 Board::West4(u64 x) {
  return (x & 0x0f0f0f0f0f0f0f0full) << 4;
}

u64 Board::NorthWest(u64 x) {
  return North(West(x));
}

u64 Board::NorthWest2(u64 x) {
  return North2(West2(x));
}

u64 Board::NorthWest4(u64 x) {
  return North4(West4(x));
}

u64 Board::NorthEast(u64 x) {
  return North(East(x));
}

u64 Board::NorthEast2(u64 x) {
  return North2(East2(x));
}

u64 Board::NorthEast4(u64 x) {
  return North4(East4(x));
}

u64 Board::SouthWest(u64 x) {
  return South(West(x));
}

u64 Board::SouthWest2(u64 x) {
  return South2(West2(x));
}

u64 Board::SouthWest4(u64 x) {
  return South4(West4(x));
}

u64 Board::SouthEast(u64 x) {
  return South(East(x));
}

u64 Board::SouthEast2(u64 x) {
  return South2(East2(x));
}

u64 Board::SouthEast4(u64 x) {
  return South4(East4(x));
}


u64 Board::SaturateEast(u64 x, u64 obstacles) {
  x |= East(x) & ~obstacles;
  obstacles |= East(obstacles);
  x |= East2(x) & ~obstacles;
  obstacles |= East2(obstacles);
  x |= East4(x) & ~obstacles;
  return x;
}

u64 Board::SaturateWest(u64 x, u64 obstacles) {
  x |= West(x) & ~obstacles;
  obstacles |= West(obstacles);
  x |= West2(x) & ~obstacles;
  obstacles |= West2(obstacles);
  x |= West4(x) & ~obstacles;
  return x;
}

u64 Board::SaturateNorth(u64 x, u64 obstacles) {
  x |= North(x) & ~obstacles;
  obstacles |= North(obstacles);
  x |= North2(x) & ~obstacles;
  obstacles |= North2(obstacles);
  x |= North4(x) & ~obstacles;
  return x;
}

u64 Board::SaturateSouth(u64 x, u64 obstacles) {
  x |= South(x) & ~obstacles;
  obstacles |= South(obstacles);
  x |= South2(x) & ~obstacles;
  obstacles |= South2(obstacles);
  x |= South4(x) & ~obstacles;
  return x;
}

u64 Board::SaturateNorthEast(u64 x, u64 obstacles) {
  x |= NorthEast(x) & ~obstacles;
  obstacles |= NorthEast(obstacles);
  x |= NorthEast2(x) & ~obstacles;
  obstacles |= NorthEast2(obstacles);
  x |= NorthEast4(x) & ~obstacles;
  return x;
}

u64 Board::SaturateNorthWest(u64 x, u64 obstacles) {
  x |= NorthWest(x) & ~obstacles;
  obstacles |= NorthWest(obstacles);
  x |= NorthWest2(x) & ~obstacles;
  obstacles |= NorthWest2(obstacles);
  x |= NorthWest4(x) & ~obstacles;
  return x;
}

u64 Board::SaturateSouthEast(u64 x, u64 obstacles) {
  x |= SouthEast(x) & ~obstacles;
  obstacles |= SouthEast(obstacles);
  x |= SouthEast2(x) & ~obstacles;
  obstacles |= SouthEast2(obstacles);
  x |= SouthEast4(x) & ~obstacles;
  return x;
}

u64 Board::SaturateSouthWest(u64 x, u64 obstacles) {
  x |= SouthWest(x) & ~obstacles;
  obstacles |= SouthWest(obstacles);
  x |= SouthWest2(x) & ~obstacles;
  obstacles |= SouthWest2(obstacles);
  x |= SouthWest4(x) & ~obstacles;
  return x;
}


u64 Board::ValidMoves(int color) {
    u64 mine = 0ull;
    u64 enemy = 0ull;
    if (color == -1) {
        mine = white_bitboard_;
        enemy = black_bitboard_;
    } else if (color == 1) {
        mine = black_bitboard_;
        enemy = white_bitboard_;
    }
    u64 empty = ~(mine | enemy);
    return (North(SaturateNorth(mine, empty) & enemy) & empty) | 
           (South(SaturateSouth(mine, empty) & enemy) & empty) | 
           (East(SaturateEast(mine, empty) & enemy) & empty) | 
           (West(SaturateWest(mine, empty) & enemy) & empty) | 
           (NorthEast(SaturateNorthEast(mine, empty) & enemy) & empty) | 
           (NorthWest(SaturateNorthWest(mine, empty) & enemy) & empty) | 
           (SouthEast(SaturateSouthEast(mine, empty) & enemy) & empty) | 
           (SouthWest(SaturateSouthWest(mine, empty) & enemy) & empty);
}

u64 Board::ValidMovesImproved(int color) {
    u64 mine_init = 0ull;
    u64 enemy_init = 0ull;
    if (color == -1) {
        mine_init = white_bitboard_;
        enemy_init = black_bitboard_;
    } else if (color == 1) {
        mine_init = black_bitboard_;
        enemy_init = white_bitboard_;
    }
    u64 empty = ~(mine_init | enemy_init);
    u64 not_mine_init = ~mine_init;
    u64 result = 0ull;


    u64 mine = mine_init;//NORTH
    u64 enemy = enemy_init;
    mine |= enemy & North(mine);
    enemy &= North(enemy);
    mine |= enemy & North2(mine);
    enemy &= North2(enemy);
    mine |= enemy & North4(mine);
    //result |= ((mine & not_mine_init) << 8);
    result |= North(mine & not_mine_init);

    mine = mine_init;//SOUTH
    enemy = enemy_init;
    mine |= enemy & South(mine);
    enemy &= South(enemy);
    mine |= enemy & South2(mine);
    enemy &= South2(enemy);
    mine |= enemy & South4(mine);
    result |= South(mine & not_mine_init);

    mine = mine_init;//WEST
    enemy = enemy_init & 0x7f7f7f7f7f7f7f7full;
    mine |= enemy & West(mine);
    enemy &= (enemy << 1);
    mine |= enemy & West2(mine);
    enemy &= (enemy << 2);
    mine |= enemy & West4(mine);
    result |= West(mine & not_mine_init);

    mine = mine_init;//EAST
    enemy = enemy_init & 0xfefefefefefefefeull;
    mine |= enemy & East(mine);
    enemy &= (enemy >> 1);
    mine |= enemy & East2(mine);
    enemy &= (enemy >> 2);
    mine |= enemy & East4(mine);
    result |= East(mine & not_mine_init);

    mine = mine_init;//NORTH_EAST
    enemy = enemy_init & 0xfefefefefefefefeull;
    mine |= enemy & NorthEast(mine);
    enemy &= (enemy << 7);
    mine |= enemy & NorthEast2(mine);
    enemy &= (enemy << 14);
    mine |= enemy & NorthEast4(mine);
    result |= ((mine & not_mine_init & 0x7f7f7f7f7f7f7f7full) << 7);




    mine = mine_init;//NORTH_WEST
    enemy = enemy_init & 0x7f7f7f7f7f7f7f7full;
    mine |= enemy & NorthWest(mine);
    enemy &= (enemy << 9);
    mine |= enemy & NorthWest2(mine);
    enemy &= (enemy << 18);
    mine |= enemy & NorthWest4(mine);
    result |= ((mine & not_mine_init & 0xfefefefefefefefeull) << 9);





    mine = mine_init;//SOUTH_EAST
    enemy = enemy_init & 0xfefefefefefefefeull;
    mine |= enemy & SouthEast(mine);
    enemy &= (enemy >> 9);
    mine |= enemy & SouthEast2(mine);
    enemy &= (enemy >> 18);
    mine |= enemy & SouthEast4(mine);
    result |= ((mine & not_mine_init & 0x7f7f7f7f7f7f7f7full) >> 9);




    mine = mine_init;//SOUTH_WEST
    enemy = enemy_init & 0x7f7f7f7f7f7f7f7full;
    mine |= enemy & SouthWest(mine);
    enemy &= (enemy >> 7);
    mine |= enemy & SouthWest2(mine);
    enemy &= (enemy >> 14);
    mine |= enemy & SouthWest4(mine);
    result |= ((mine & not_mine_init & 0xfefefefefefefefeull) >> 7);

    return result & empty;
}

void Board::MakeMove(int color, u64 my_move) {
    u64 result = 0ull;
    u64 * my_pieces = 0ull;
    u64 * opp_pieces = 0ull;
    if (color == -1) {
        my_pieces = &white_bitboard_;
        opp_pieces = &black_bitboard_;
    } else if (color == 1) {
        my_pieces = &black_bitboard_;
        opp_pieces = &white_bitboard_;
    }

    u64 tiles_to_flip = North(my_move) & (*opp_pieces);
    for (int i = 0; i < 5; ++i)
        tiles_to_flip |= North(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (N(tiles_to_flip) & (*my_pieces)));
    if ( (North(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    tiles_to_flip = South(my_move) & (*opp_pieces);
    for (int i=0; i<5; ++i)
        tiles_to_flip |= South(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (S(tiles_to_flip) & (*my_pieces)));
    if ( (South(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    tiles_to_flip = East(my_move) & (*opp_pieces);
    for (int i=0; i<5; ++i)
        tiles_to_flip |= East(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (E(tiles_to_flip) & (*my_pieces)));
    if ( (East(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    tiles_to_flip = West(my_move) & (*opp_pieces);
    for (int i=0; i<5; ++i)
        tiles_to_flip |= West(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (W(tiles_to_flip) & (*my_pieces)));
    if ( (West(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    tiles_to_flip = NorthEast(my_move) & (*opp_pieces);
    for (int i=0; i<5; ++i)
        tiles_to_flip |= NorthEast(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (NE(tiles_to_flip) & (*my_pieces)));
    if ( (NorthEast(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    tiles_to_flip = NorthWest(my_move) & (*opp_pieces);
    for (int i=0; i<5; ++i)
        tiles_to_flip |= NorthWest(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (NW(tiles_to_flip) & (*my_pieces)));
    if ( (NorthWest(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    tiles_to_flip = SouthEast(my_move) & (*opp_pieces);
    for (int i=0; i<5; ++i)
        tiles_to_flip |= SouthEast(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (SE(tiles_to_flip) & (*my_pieces)));
    if ( (SouthEast(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    tiles_to_flip = SouthWest(my_move) & (*opp_pieces);
    for (int i=0; i<5; ++i)
        tiles_to_flip |= SouthWest(tiles_to_flip) & (*opp_pieces);
    //result |= (tiles_to_flip & (SW(tiles_to_flip) & (*my_pieces)));
    if ( (SouthWest(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    *my_pieces |= result;
    *my_pieces |= my_move;
    *opp_pieces ^= result;
}

u64 Board::MakeMoveImproved(int color, u64 my_move) {
    u64 result = 0ull;
    u64 * my_pieces = nullptr;
    u64 * opp_pieces = nullptr;
    if (color == -1) {
        my_pieces = &white_bitboard_;
        opp_pieces = &black_bitboard_;
    } else {
        my_pieces = &black_bitboard_;
        opp_pieces = &white_bitboard_;
    }

    u64 frontier = North(my_move) & (*opp_pieces);
    u64 tiles_to_flip = frontier;
    while ((North(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= North(frontier) & (*opp_pieces);
        frontier = North(frontier) & (*opp_pieces);
    }
    if ((North(tiles_to_flip) & (*my_pieces)) != 0ull ) {
        result |= tiles_to_flip;
    }

    frontier = South(my_move) & (*opp_pieces);
    tiles_to_flip = frontier;
    while ((South(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= South(frontier) & (*opp_pieces);
        frontier = South(frontier) & (*opp_pieces);
    }
    if ( (South(tiles_to_flip) & (*my_pieces)) != 0ull ) { 
        result |= tiles_to_flip; 
    }



    frontier = West(my_move) & (*opp_pieces);
    tiles_to_flip = frontier;
    while ((West(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= West(frontier) & (*opp_pieces);
        frontier = West(frontier) & (*opp_pieces);
    }
    if ( (West(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }



    frontier = East(my_move) & (*opp_pieces);
    tiles_to_flip = frontier;
    while ((East(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= East(frontier) & (*opp_pieces);
        frontier = East(frontier) & (*opp_pieces);
    }
    if ( (East(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }




    frontier = SouthEast(my_move) & (*opp_pieces);
    tiles_to_flip = frontier;
    while ((SouthEast(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= SouthEast(frontier) & (*opp_pieces);
        frontier = SouthEast(frontier) & (*opp_pieces);
    }
    if ( (SouthEast(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }



    frontier = SouthWest(my_move) & (*opp_pieces);
    tiles_to_flip = frontier;
    while ((SouthWest(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= SouthWest(frontier) & (*opp_pieces);
        frontier = SouthWest(frontier) & (*opp_pieces);
    }
    if ( (SouthWest(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }



    frontier = NorthEast(my_move) & (*opp_pieces);
    tiles_to_flip = frontier;
    while ((NorthEast(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= NorthEast(frontier) & (*opp_pieces);
        frontier = NorthEast(frontier) & (*opp_pieces);
    }
    if ( (NorthEast(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }


    frontier = NorthWest(my_move) & (*opp_pieces);
    tiles_to_flip = frontier;
    while ((NorthWest(frontier) & (*opp_pieces)) != 0ull) {
        tiles_to_flip |= NorthWest(frontier) & (*opp_pieces);
        frontier = NorthWest(frontier) & (*opp_pieces);
    }
    if ( (NorthWest(tiles_to_flip) & (*my_pieces)) != 0ull ) { result |= tiles_to_flip; }

    *opp_pieces ^= result;
    *my_pieces |= (result | my_move);
    return result;
}

void Board::UnmakeMove(int color, u64 m_move, u64 tiles_to_flip) {
    u64 * my_pieces = 0ull;
    u64 * opp_pieces = 0ull;
    if (color == -1) {
        my_pieces = &white_bitboard_;
        opp_pieces = &black_bitboard_;
    } else {
        my_pieces = &black_bitboard_;
        opp_pieces = &white_bitboard_;
    }
    (*my_pieces) ^= (m_move | tiles_to_flip);
    (*opp_pieces) |= tiles_to_flip;
}

std::vector<u64> CreateIndicatorBitboard() {
    std::vector<u64> indicator_bitboard;
    for (int i = 0; i < 64; i++) {
        indicator_bitboard.emplace_back(0b1ull << i);
    }
    return indicator_bitboard;

}

