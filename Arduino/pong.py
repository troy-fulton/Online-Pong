import pygame
import random
import sys
import requests
import re
import time

# Set the IP based on the Arduino IP Address for the server
ip_addr = "192.168.137.221"

class Paddle(pygame.Rect):
    def __init__(self, velocity, up_key, down_key, *args, **kwargs):
        self.velocity = velocity
        self.up_key = up_key
        self.down_key = down_key
        super().__init__(*args, **kwargs)

    def move_paddle(self, board_height, board_percentage):

        self.y = board_percentage * (board_height - self.height)
##        
##        keys_pressed = pygame.key.get_pressed()        
##
##        if keys_pressed[self.up_key]:
##            if self.y - self.velocity > 0:
##                self.y -= self.velocity
##
##        if keys_pressed[self.down_key]:
##            if self.y + self.velocity < board_height - self.height:
##                self.y = self.y + self.velocity


class Ball(pygame.Rect):
    def __init__(self, velocity, *args, **kwargs):
        self.velocity = velocity
        self.angle = 0
        super().__init__(*args, **kwargs)

    def move_ball(self):
        self.x += self.velocity
        self.y += self.angle


class Pong():
    height = 500
    width = 1100

    paddle_width = 10
    paddle_height = 100

    ball_width = 10
    ball_velocity = 10
    ball_angle = 0

    color = (255, 255, 255)  

    def __init__(self):
        pygame.init()  # Start the pygame instance.

        self.game_over_pattern = re.compile("game_over ([0,1])")

        self.game_over = 0 
        
        self.msp432_paddle_pattern = re.compile("msp432_paddle ([0-9]*\.[0-9]*)")
                
        self.other_paddle_pattern = re.compile("other_paddle ([0-9]*\.[0-9]*)")

        

        # Setup the screen
        self.screen = pygame.display.set_mode((self.width, self.height))
        self.clock = pygame.time.Clock()

        # Create the player objects.

        self.paddles = []
        self.balls = []
        self.paddles.append(Paddle(  # The left paddle
            self.ball_velocity,
            pygame.K_w, #w key 
            pygame.K_s, #s key 
            0, #x position 
            self.height / 2 - self.paddle_height / 2, #y position 
            self.paddle_width, #width 
            self.paddle_height #height 
        ))

        self.paddles.append(Paddle(  # The right paddle
            self.ball_velocity,
            pygame.K_UP,
            pygame.K_DOWN,
            self.width - self.paddle_width,
            self.height / 2 - self.paddle_height / 2,
            self.paddle_width,
            self.paddle_height
        ))

        self.balls.append(Ball(
            self.ball_velocity,
            self.width / 2 - self.ball_width / 2,
            self.height / 2 - self.ball_width / 2,
            self.ball_width,
            self.ball_width
        ))

        self.central_line = pygame.Rect(self.width/2, 0, 1, self.height)

    def check_ball_hits_wall(self):
        for ball in self.balls:
            if ball.x > self.width or ball.x < 0:
                main()

            if ball.y > self.height - self.ball_width or ball.y < 0:
                ball.angle = -ball.angle

    def check_ball_hits_paddle(self):
        for ball in self.balls:
            for paddle in self.paddles:
                if ball.colliderect(paddle):
                    ball.velocity = -ball.velocity
                    ball.angle = random.randint(-10, 10)
                    break

    def game_loop(self):

        counter = 0

        msp432_paddle_percentage = 0

        other_paddle_percentage = 0 
        
        while True:
            counter += 1
            if(counter == 200):
                counter = 0
                msp432_paddle_percentage,other_paddle_percentage = request_state()
                
                
            for event in pygame.event.get():
                # Add some extra ways to exit the game.
                if event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
                    return

            self.check_ball_hits_paddle()
            self.check_ball_hits_wall()

            # Redraw the screen.
            self.screen.fill((0, 0, 0))

            msp_paddle = self.paddles[0]
            other_paddle = self.paddles[1]

            msp_paddle.move_paddle(self.height, msp432_paddle_percentage)
            other_paddle.move_paddle(self.height, other_paddle_percentage)
            pygame.draw.rect(self.screen, self.color, msp_paddle)
            pygame.draw.rect(self.screen, self.color, other_paddle)
                

            # loop that moves the ball .
            for ball in self.balls:
                ball.move_ball()
                pygame.draw.rect(self.screen, self.color, ball)

            pygame.draw.rect(self.screen, self.color, self.central_line)

            pygame.display.flip()
            self.clock.tick(60) #60 fps

    def request_state():
             # Request the page:
        response = requests.get("http://"+ip_addr)
        #print(response.text)
        
        # Parse the data we need out of the response with regex (don't need to worry about this stuff)
        # Eventually, we will need to parse out which player we are when we get to two-player
   
        game_over_match = int(game_over_pattern.findall(response.text)[0])
        
        self.game_over = game_over_match 
        
        msp432_paddle_match = float(msp432_paddle_pattern.findall(response.text)[0])        
       
        other_paddle_match = float(other_paddle_pattern.findall(response.text)[0])

        return msp432_paddle_match, other_paddle_match

        

def main():
    pong = Pong()
    pong.game_loop()


if __name__ == '__main__':
    main()
