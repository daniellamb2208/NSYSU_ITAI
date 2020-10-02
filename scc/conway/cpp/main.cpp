#include <functional>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

using namespace std;

void initMap(vector<vector<char>> &conwayMap, size_t width, size_t height) {
  vector<char> row;
  row.resize(width, '.');
  conwayMap.resize(height, row);
}

void printMap(vector<vector<char>> conwayMap) {
  for (auto i : conwayMap) {
    for (auto j : i)
      cout << j << " ";
    cout << endl;
  }
}

vector<size_t> get_rand(pair<size_t, size_t> range, int num = 1) {
  random_device rd;
  mt19937_64 gen = mt19937_64(rd());
  uniform_int_distribution<> distribution(0, range.first * range.second);
  auto rand = bind(distribution, gen);

  vector<size_t> result;
  result.resize(num);
  for (auto iter = result.begin(); iter != result.end(); iter++)
    *iter = rand();
  return result;
}

void setCell(vector<vector<char>> &conayMap, vector<size_t> cells) {
  for (auto i : cells) {
    auto column = i / conayMap.size();
    auto row = i - column * conayMap.size();
    conayMap.at(column).at(row) = 'O';
  }
}

bool updateState(vector<vector<char>> &currentState, vector<string> &history) {
  bool opSuccess = false;
  vector<vector<char>> nextState = currentState;

  // TODO: Add the rules of life, the rules will update to the nextState for
  // TODO: buffering

  // SOME RULES

  // update current state from the buffer state
  currentState = nextState;
  // After generated next state, snapshot the vector
  // To prevent the looping state
  string snapshot;
  for (auto i : nextState)
    snapshot += string(i.begin(), i.end());
  // find is it appeared?
  auto isAppeared = find(history.begin(), history.end(), snapshot);
  if (opSuccess = isAppeared == history.end()) // not found
    history.push_back(snapshot);
  return opSuccess;
}

int main() {
  size_t width, height;
  cout << "Please input the window size(Width, Height): ";
  cin >> width >> height;

  vector<vector<char>> conwayMap;
  vector<string> history;
  initMap(conwayMap, width, height);

  auto cells = get_rand(make_pair(width, height), width * height / 3);

  setCell(conwayMap, cells);
  do {
    printMap(conwayMap);
  } while (updateState(conwayMap, history));

  return 0;
}