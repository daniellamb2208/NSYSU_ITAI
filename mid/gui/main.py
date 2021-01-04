#!/usr/bin/python3
import sys
import time
import pygame
import pygame.locals as game
import wrapper

width, height = 800, 600

size = (20, 20)  # ants' size
home = (128, 42, 42)  # dark brown
ground = (244, 164, 96)  # orange
hormones = (255, 255, 0)  # yellow
food = (255, 0, 0)  # red
life_bar = (0, 128, 0)  # green
full_life = 100  # upper limit of life
# upper limit of food : 3(decide on yourself)


class ant(pygame.sprite.Sprite):
    def __init__(self, size: tuple, x, y, life):
        super().__init__()
        self.ant_image = pygame.image.load('./ant.png').convert_alpha()
        self.image = pygame.transform.scale(self.ant_image, size)
        self.rect = self.image.get_rect()
        self.rect.topleft = (x, y)
        self.width = size[0]
        self.height = size[0]
        self.life = life


def main():
    pygame.init()
    window = pygame.display.set_mode((width, height))
    pygame.display.set_caption("Ants' life")
    clk = pygame.time.Clock()

    # init C++ ant
    wrapper.init()

    while True:
        for event in pygame.event.get():
            if event.type == game.QUIT:
                pygame.quit()
                sys.exit()
        # file = open("data.txt")
        # obj = []
        # arr = []
        # while True:
        #     line = file.readline()
        #     if line == '':
        #         break
        #     for i in line.split():
        #         obj.append(float(i))
        #     arr.append(obj)
        #     obj = []
        # file.close()
        wrapper.go()
        arr = wrapper.view()
        window.fill(ground)
        pygame.draw.rect(window, home, (0, 0, size[0], size[0]))
        for i in range(len(arr)):
            if arr[i][0] == 0:  # ant
                Ant = ant(size, arr[i][1], arr[i][2], arr[i][3])
                window.blit(Ant.image, Ant.rect.topleft)
                pygame.draw.rect(
                    window, life_bar, (Ant.rect[0], Ant.rect[1] + size[0], size[0]*Ant.life/full_life, 4))
            elif arr[i][0] == 1:  # hormones
                pygame.draw.circle(
                    window, hormones, (arr[i][1], arr[i][2]), size[0]//8)
            elif arr[i][0] == 2:  # food
                pygame.draw.circle(
                    window, food, (arr[i][1], arr[i][2]), size[0]//8)
        pygame.display.flip()  # update to screen
        clk.tick(60)  # fps
        # time.sleep(1)  # synchronization
    pygame.quit()


if __name__ == '__main__':
    main()
