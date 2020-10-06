#include <functional>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <algorithm>
#include <unistd.h>

using namespace std;

void initMap(vector<vector<char>> &conwayMap, size_t width, size_t height,
             char element = '.') {
  vector<char> row;
  row.resize(width, element);
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
  uniform_int_distribution<> distribution(0, range.first * range.second - 1);
  auto rand = bind(distribution, gen);

  vector<size_t> result;
  result.resize(num);
  for (auto iter = result.begin(); iter != result.end(); iter++)
    *iter = rand();
  return result;
}

void setCell(vector<vector<char>> &conayMap, vector<size_t> cells) {
  for (auto i : cells) {
    auto column = i % conayMap.size();
    auto row = i / conayMap.size();
    conayMap.at(column).at(row) = 'O';
  }
}

bool checkLoop(vector<vector<char>> nextState, vector<string> &history) {
  // Snapshot the vector to prevent the looping state
  bool opSuccess;
  string snapshot;
  for (auto i : nextState)
    snapshot += string(i.begin(), i.end());
  // Find is it appeared?
  auto isAppeared = find(history.begin(), history.end(), snapshot);
  if (opSuccess = isAppeared == history.end()) // Not found
    history.push_back(snapshot);
  return opSuccess;
}

bool updateState(vector<vector<char>> &currentState, vector<string> &history) {
  vector<vector<char>> nextState = currentState;

  // span the map
  vector<vector<char>> span;
  initMap(span, currentState.at(0).size() + 2, currentState.size() + 2, 0);
  for (size_t i = 0; i < currentState.size(); i++)
    for (size_t j = 0; j < currentState.at(0).size(); j++)
      span.at(i + 1).at(j + 1) = char(currentState.at(i).at(j) == 'O');

  // The four rules
  for (size_t i = 0; i < nextState.size(); i++)
    for (size_t j = 0; j < nextState.at(0).size(); j++) {
      int aliveNum = span[i][j] + span[i][j + 1] + span[i][j + 2] +
                     span[i + 1][j] + span[i + 1][j + 2] + span[i + 2][j] +
                     span[i + 2][j + 1] + span[i + 2][j + 2];
      if (currentState[i][j] == 'O') // origin is alive.
        nextState[i][j] = ((aliveNum < 2 || aliveNum > 3)) ? '.' : 'O';
      else
        nextState[i][j] = (aliveNum == 3) ? 'O' : '.';
    }

  // Update current state from the buffer state
  currentState = nextState;
  return checkLoop(nextState, history);
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
	cout << "\033[2J\033[H";
    printMap(conwayMap);
    usleep(100000);
  } while (updateState(conwayMap, history));

  return 0;
}
