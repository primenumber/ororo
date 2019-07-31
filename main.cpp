#include <iostream>
#include <optional>
#include <tuple>

constexpr std::size_t size = 5;

enum class State {
  Empty,
  Black,
  White,
};

struct Board {
 public:
  Board() {
    for (std::size_t i = 0; i < size; ++i)
      for (std::size_t j = 0; j < size; ++j)
        table[i][j] = State::Empty;
    std::size_t h = (size + 1) / 2;
    table[h-1][h-2] = State::White;
    table[h-1][h-1] = State::White;
    table[h][h-2] = State::White;
    table[h][h-1] = State::White;
    table[h-1][h] = State::Black;
    table[h-1][h+1] = State::Black;
    table[h][h] = State::Black;
    table[h][h+1] = State::Black;
  }
  Board(const Board& that) {
    for (std::size_t i = 0; i < size; ++i)
      for (std::size_t j = 0; j < size; ++j)
        table[i][j] = that.table[i][j];
  }
  Board& operator=(const Board& that) {
    for (std::size_t i = 0; i < size; ++i)
      for (std::size_t j = 0; j < size; ++j)
        table[i][j] = that.table[i][j];
  }
  State table[size][size];
};

bool is_movable(const Board& bd, const int i, const int j, const State s) {
  if (i < 0 || i >= size || j < 0 || j >= size) throw std::logic_error("Out of board");
  if (s != State::Black && s != State::White) throw std::logic_error("Unknown state");
  if (bd.table[i][j] != State::Empty) return false;
  const int di[] = {1, 1, 1, 0, -1, -1, -1, 0};
  const int dj[] = {1, 0, -1, -1, -1, 0, 1, 1};
  for (int d = 0; d < 8; ++d) {
    bool ok = true;
    for (int k = 1; k <= 2; ++k) {
      const int ni = i + di[d] * k;
      const int nj = j + dj[d] * k;
      if (ni < 0 || ni >= size || nj < 0 || nj >= size) {
        ok = false;
        break;
      }
      if (bd.table[ni][nj] != s) {
        ok = false;
        break;
      }
    }
    if (ok) return true;
  }
  return false;
}

bool is_movable(const Board& bd, const State s) {
  for (std::size_t i = 0; i < size; ++i)
    for (std::size_t j = 0; j < size; ++j)
      if (is_movable(bd, i, j, s)) return true;
  return false;
}

std::optional<Board> move(const Board& bd, const int i, const int j, const State s) {
  if (!is_movable(bd, i, j, s)) return std::nullopt;
  Board res = bd;
  res.table[i][j] = s;
  return res;
}

int score(const Board& bd) {
  int b = 0;
  int w = 0;
  for (std::size_t i = 0; i < size; ++i)
    for (std::size_t j = 0; j < size; ++j) {
      switch (bd.table[i][j]) {
        case State::Black:
          ++b;
          break;
        case State::White:
          ++w;
          break;
      }
    }
  if (b == w) {
    return 0;
  } else if (b > w) {
    return size * size - w;
  } else { // b < w
    return b - size * size;
  }
}

int score(const Board& bd, const State s) {
  int raw = score(bd);
  if (s == State::Black) return raw;
  else return -raw;
}

State flip(const State s) {
  return s == State::Black ? State::White : State::Black;
}

struct Hand {
  int i;
  int j;
};

std::tuple<int, std::optional<Hand>> alpha_beta(const Board& bd, int alpha, int beta, const State s, const bool passed) {
  if (!is_movable(bd, s)) {
    if (passed) {
      return std::make_tuple(score(bd, s), std::nullopt);
    } else {
      const auto [score, hand] = alpha_beta(bd, -beta, -alpha, flip(s), true);
      return std::make_tuple(-score, std::nullopt);
    }
  }
  int res = -static_cast<int>(size * size);
  Hand h = {0, 0};
  for (std::size_t i = 0; i < size; ++i) {
    for (std::size_t j = 0; j < size; ++j) {
      if (!is_movable(bd, i, j, s)) continue;
      const auto next = *move(bd, i, j, s);
      const auto [score, hand] = alpha_beta(next, -beta, -alpha, flip(s), false);
      if (-score > res) {
        res = -score;
        h = (Hand) { static_cast<int>(i), static_cast<int>(j) };
      }
      if (res >= beta) return std::make_tuple(res, h);
      alpha = std::max(alpha, res);
    }
  }
  return std::make_tuple(res, h);
}

std::string to_s(const State s) {
  switch (s) {
    case State::Empty: return ".";
    case State::Black: return "X";
    case State::White: return "O";
  }
  return "?";
}

void output(const Board& bd) {
  for (std::size_t i = 0; i < size; ++i) {
    for (std::size_t j = 0; j < size; ++j) {
      std::cout << to_s(bd.table[i][j]);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

bool is_gameover(const Board& bd, const State s) {
  if (is_movable(bd, s)) return false;
  return !is_movable(bd, flip(s));
}

int main() {
  Board bd;
  State s = State::Black;
  while (!is_gameover(bd, s)) {
    output(bd);
    const auto [score, hand] = alpha_beta(bd, -static_cast<int>(size*size), size*size, s, false);
    if (hand) {
      bd = *move(bd, hand->i, hand->j, s);
    }
    s = flip(s);
  }
  output(bd);
  return 0;
}
