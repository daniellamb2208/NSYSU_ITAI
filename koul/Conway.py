import pygame
from random import randint
from sys import exit


class Conway:
    alive=1
    dead=0
    def __init__(self,wid=800,hei=600,sz=10,a_c=(255,255,255),d_c=(0,0,0)):
              
        self.window_wid = int(wid)
        self.window_hei = int(hei)
        self.cell_size = int(sz)
        self.alive_color = a_c
        self.dead_color = d_c
        self.using_lattice = 0
        self.columns = self.window_wid // self.cell_size
        self.rows = self.window_hei // self.cell_size
        self.lattice = []
        self.initialize_lattice()
        self.generate_random_cell()
        
        pygame.init()
        self.window = pygame.display.set_mode((self.window_wid,self.window_hei))
        self.fill_window()
        pygame.display.flip()
        
    def initialize_lattice(self):
        for i in range(2):
            tmp = []
            for r in range(self.rows):
                col = [0] * self.columns
                tmp.append(col)
            self.lattice.append(tmp)
            
    def generate_random_cell(self,latt=0):
        for r in range(self.rows):
            for c in range(self.columns):
                self.lattice[latt][r][c] = randint(0,1)
                
    def set_to_dead(self,latt=0):
        for r in range(self.rows):
            for c in range(self.columns):
                self.lattice[latt][r][c] = Conway.dead
    
    def clean_up(self):
        self.window.fill(self.dead_color)
        
    def idle_lattice(self):
        return (self.using_lattice+1)%2
    
    def cell_value(self,r,c):
        try:
            value = self.lattice[self.using_lattice][r][c]
        except:
            value = Conway.dead
        return value
    
    def how_many_nearby(self,r,c):
        nei = 0
        for i in range(r-1,r+2):
            for j in range(c-1,c+2):
                if i != r or j != c:
                    nei += self.cell_value(i,j)
        return nei
        
    def dead_or_alive(self,r,c):
        living_nei = self.how_many_nearby(r,c)
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
                status = self.dead_or_alive(r,c)
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
                pygame.draw.circle(self.window,color,(positionX,positionY),self.cell_size//2,0)
        pygame.display.flip()
    
    def control(self):
        for e in pygame.event.get():
            if e.type == pygame.KEYDOWN:
                if e.unicode == 'r':
                    self.generate_random_cell()
                if e.unicode == 'q':
                    exit()
    
    def launch(self):
        clk = pygame.time.Clock()
        while True:
            self.control()
            self.update()
            self.fill_window()
            clk.tick(5)

Conway().launch()