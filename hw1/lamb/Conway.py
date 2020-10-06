import pygame
from random import randint
from sys import exit
import threading


class Conway:
    alive = 1
    dead = 0

    def __init__(self, wid=800, hei=600, sz=10, a_c=(255, 255, 255), d_c=(0, 0, 0)):

        self.window_wid = int(wid)
        self.window_hei = int(hei)
        self.cell_size = int(sz)
        self.alive_color = a_c
        self.dead_color = d_c
        self.using_lattice = 0
        self.columns = self.window_wid // self.cell_size  # how many cell
        self.rows = self.window_hei // self.cell_size
        self.lattice = []
        self.initialize_lattice()
        self.generate_random_cell()
        # self.set_to_alive()
        self.multi_thread = False
        self.running = True

        pygame.init()
        self.window = pygame.display.set_mode(
            (self.window_wid, self.window_hei))
        self.fill_window()
        pygame.display.flip()

    def initialize_lattice(self):
        for i in range(2):
            tmp = []
            for r in range(self.rows):
                col = [0] * self.columns
                tmp.append(col)
            self.lattice.append(tmp)

    def generate_random_cell(self, latt=0):
        for r in range(self.rows):
            for c in range(self.columns):
                self.lattice[latt][r][c] = randint(0, 1)

    def set_to_dead(self, latt=0):
        for r in range(self.rows):
            for c in range(self.columns):
                self.lattice[latt][r][c] = Conway.dead

    def set_to_alive(self, latt=0):
        for r in range(self.rows):
            for c in range(self.columns):
                self.lattice[latt][r][c] = Conway.alive

    def clean_up(self):
        self.window.fill(self.dead_color)

    def idle_lattice(self):
        return (self.using_lattice+1) % 2

    def cell_value(self, r, c):
        if r < 0 or c < 0 or r >= self.rows or c >= self.columns:
            return Conway.dead
        return self.lattice[self.using_lattice][r][c]

    def how_many_nearby(self, r, c):
        nei = 0
        for i in range(r-1, r+2):
            for j in range(c-1, c+2):
                if i != r or j != c:
                    nei += self.cell_value(i, j)
        return nei

    def dead_or_alive(self, r, c):
        living_nei = self.how_many_nearby(r, c)
        if self.lattice[self.using_lattice][r][c] == Conway.alive:
            if living_nei > 3 or living_nei < 2:
                return Conway.dead
            else:
                return Conway.alive
        elif self.lattice[self.using_lattice][r][c] == Conway.dead:
            if living_nei == 3:
                return Conway.alive
            else:
                return Conway.dead

    def update(self):
        idle = self.idle_lattice()
        self.set_to_dead(idle)

        for r in range(self.rows):
            for c in range(self.columns):
                status = self.dead_or_alive(r, c)
                self.lattice[idle][r][c] = status

        self.using_lattice = idle

    def fill_window(self):
        self.clean_up()
        bias = self.cell_size//2
        for r in range(self.rows):
            for c in range(self.columns):
                if self.lattice[self.using_lattice][r][c] == Conway.dead:
                    color = self.dead_color
                else:
                    color = self.alive_color
                positionX = self.cell_size*c + bias
                positionY = self.cell_size*r + bias
                pygame.draw.circle(
                    self.window, color, (positionX, positionY), self.cell_size//2, 0)
        pygame.display.flip()

    def control(self):
        for e in pygame.event.get():
            # print(e)
            if e.type == pygame.KEYDOWN:
                if e.unicode == 'r':
                    self.generate_random_cell()
                if e.unicode == 'q':
                    return False
            if e.type == pygame.QUIT:
                return False
        return True

    def customize(self):
        while self.running:
            for e in pygame.event.get():
                if e.type == pygame.MOUSEBUTTONDOWN:
                    x, y = pygame.mouse.get_pos()[0], pygame.mouse.get_pos()[1]
                    x //= self.cell_size
                    y //= self.cell_size
                    # print(x,y)
                    if self.lattice[self.using_lattice][y][x] == Conway.alive:
                        self.lattice[self.using_lattice][y][x] = Conway.dead
                        # print("dead")
                    else:
                        self.lattice[self.using_lattice][y][x] = Conway.alive
                        # print("alive")
                if e.type == pygame.KEYDOWN:
                    if e.unicode == 'r':
                        self.generate_random_cell()
                    if e.unicode == 'j':
                        self.jet_pattern()
                    if e.unicode == 'd':
                        self.set_to_dead()
                    if e.unicode == 's':
                        self.shadow_pattern()
                    if e.unicode == 'q':
                        self.running = False
                if e.type == pygame.QUIT:
                    self.running = False

    def jet_pattern(self):
        self.lattice[self.using_lattice][2][0] = Conway.alive
        self.lattice[self.using_lattice][2][1] = Conway.alive
        self.lattice[self.using_lattice][2][2] = Conway.alive
        self.lattice[self.using_lattice][0][1] = Conway.alive
        self.lattice[self.using_lattice][1][2] = Conway.alive

    def shadow_pattern(self):
        self.lattice[self.using_lattice][20][50] = Conway.alive
        self.lattice[self.using_lattice][21][50] = Conway.alive
        self.lattice[self.using_lattice][22][50] = Conway.alive
        self.lattice[self.using_lattice][23][50] = Conway.alive
        self.lattice[self.using_lattice][24][50] = Conway.alive
        self.lattice[self.using_lattice][25][50] = Conway.alive
        self.lattice[self.using_lattice][26][50] = Conway.alive
        self.lattice[self.using_lattice][27][50] = Conway.alive
        self.lattice[self.using_lattice][20][51] = Conway.alive
        self.lattice[self.using_lattice][22][51] = Conway.alive
        self.lattice[self.using_lattice][23][51] = Conway.alive
        self.lattice[self.using_lattice][24][51] = Conway.alive
        self.lattice[self.using_lattice][25][51] = Conway.alive
        self.lattice[self.using_lattice][27][51] = Conway.alive
        self.lattice[self.using_lattice][21][52] = Conway.alive
        self.lattice[self.using_lattice][22][52] = Conway.alive
        self.lattice[self.using_lattice][23][52] = Conway.alive
        self.lattice[self.using_lattice][24][52] = Conway.alive
        self.lattice[self.using_lattice][25][52] = Conway.alive
        self.lattice[self.using_lattice][26][52] = Conway.alive
        self.lattice[self.using_lattice][27][52] = Conway.alive
        self.lattice[self.using_lattice][20][52] = Conway.alive

        self.lattice[self.using_lattice][20][20] = Conway.alive
        self.lattice[self.using_lattice][21][20] = Conway.alive
        self.lattice[self.using_lattice][22][20] = Conway.alive
        self.lattice[self.using_lattice][23][20] = Conway.alive
        self.lattice[self.using_lattice][24][20] = Conway.alive
        self.lattice[self.using_lattice][25][20] = Conway.alive
        self.lattice[self.using_lattice][26][20] = Conway.alive
        self.lattice[self.using_lattice][27][20] = Conway.alive
        self.lattice[self.using_lattice][20][21] = Conway.alive
        self.lattice[self.using_lattice][22][21] = Conway.alive
        self.lattice[self.using_lattice][23][21] = Conway.alive
        self.lattice[self.using_lattice][24][21] = Conway.alive
        self.lattice[self.using_lattice][25][21] = Conway.alive
        self.lattice[self.using_lattice][27][21] = Conway.alive
        self.lattice[self.using_lattice][21][22] = Conway.alive
        self.lattice[self.using_lattice][22][22] = Conway.alive
        self.lattice[self.using_lattice][23][22] = Conway.alive
        self.lattice[self.using_lattice][24][22] = Conway.alive
        self.lattice[self.using_lattice][25][22] = Conway.alive
        self.lattice[self.using_lattice][26][22] = Conway.alive
        self.lattice[self.using_lattice][27][22] = Conway.alive
        self.lattice[self.using_lattice][20][22] = Conway.alive

    def launch(self):
        clk = pygame.time.Clock()
        multi_threaded = False
        t = threading.Thread(target=self.customize)
        t.daemon = True
        while self.running:
            if multi_threaded == False:
                t.start()
                multi_threaded = True

            self.running = self.control()
            self.update()
            self.fill_window()
            clk.tick(5)
        t.join()


Conway().launch()
