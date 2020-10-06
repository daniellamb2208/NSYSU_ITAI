# HW 1 of Introduction to Artificial Intelligence in NSYSU

We implemented Conway's game of life by Python

## In Linux environment quick start

Quick Installation:
```
./setup.sh
```

Quick execution:
```
python3 Conway.py
```

## Usage

Dependent package: `pygame`

[Installation tutorial](https://stackoverflow.com/questions/51793198/pip3-install-pygame-not-working)

Firstly, please make sure you already have installed the pip tool
> Try 
```
python3 -m pip install -U pygame --user
python3 -m pygame.examples.aliens
```

If you can successfully run the command, finished.

Or, Try 
```pip3 install pygame==2.0.0.dev10```

If you still failed please use the `./setup.sh` to set up the environment or google it by yourself.

## About homework UI

The program simply random the cells then follow the rules to show the game.

And there are some keys could control the game:

Press the key `r` to re-random the cells

Press the key`q` to quit the game

(Before create Pattern of customization, the better way to see is press `d` first)

Press the key `j` to create the jet patteren from upper_left to right_bottom moving

Press the key `s` to create the two shadows on the middle of screen

Press the key `d` to kill all the cells

(there some time delay problems in our game, so if press one time doesn't work correctly, you may press the key again)