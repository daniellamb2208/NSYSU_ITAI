#!/usr/bin/python3
import cppyy

cppyy.include('../c++/driver/interfaces.hpp')
cppyy.load_library("../c++/cmake/libant.so")


def gen_ant(num, pos):
    cppyy.gbl.ant_game.add_ant(num, pos)


def add_food(x, y):
    cppyy.gbl.ant_game.add_food(x, y)


def init():
    cppyy.gbl.ant_game.init()


def view():
    return cppyy.gbl.ant_game.view()


def birth():
    cppyy.gbl.ant_game.birth()


def go():
    cppyy.gbl.ant_game.go()


if __name__ == '__main__':
    print("hi")
