import pygame
import random
import sys
import requests
import re
import time

# Set the IP based on the Arduino IP Address for the server
ip_addr = "192.168.137.84"
headers = {
        'player': '1',
        'game_over': '0',
        'x': '0',
        'y': '0',
        'angle': '0',
        'other_paddle': '0'
        }

class Paddle(pygame.Rect):
    def __init__(self, velocity, up_key, down_key, *args, **kwargs):
        self.velocity = velocity
        self.up_key = up_key
        self.down_key = down_key
        super().__init__(*args, **kwargs)

    def move_paddle(self, board_height, board_percentage, player):
        if player == 1 or (player == 2 and headers['player'] != '2'):
            self.y = int(board_percentage * (board_height - self.height))
            
        if player == 2 and (player == 2 and headers['player'] == '2'):
            keys_pressed = pygame.key.get_pressed()
            
            if keys_pressed[self.up_key]:
                if self.y - self.velocity > 0:
                    self.y -= self.velocity
                    
            if keys_pressed[self.down_key]:
                if self.y + self.velocity < board_height - self.height:
                    self.y += self.velocity
            
            headers['other_paddle'] = str(100* (float(self.y) / float(board_height - self.height)))


class Ball(pygame.Rect):
    def __init__(self, velocity, *args, **kwargs):
        self.velocity = velocity
        self.angle = 0
        super().__init__(*args, **kwargs)

    def move_ball(self):
        self.x += self.velocity
        self.y += self.angle


class Pong:
    height = 500
    width = 1100

    paddle_width = 10
    paddle_height = 100

    ball_width = 10
    ball_velocity = 15
    ball_angle = 0

    color = (255, 0, 0)  
    
    game_over = False

    def __init__(self):
        pygame.init()  # Start the pygame instance.
        
        self.msp432_paddle_pattern = re.compile("msp432_paddle -?([0-9]*\.[0-9]*)")
                
        self.other_paddle_pattern = re.compile("other_paddle -?([0-9]*\.[0-9]*)")
        
        self.game_over_pattern = re.compile("game_over ([0-9]*)")
        self.x_pattern = re.compile("x -?([0-9]*)")
        self.y_pattern = re.compile("y -?([0-9]*)")
        self.angle_pattern = re.compile("angle -?([0-9]*)")

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
                i = 0
                # Notify the arduino that the game is over
                headers['game_over'] = '1'
                requests.get("http://"+ip_addr, headers=headers)
                while True:
                    i += 1
                    for event in pygame.event.get(): #press 1 to restart or 2 to quit 
                        if event.type == pygame.KEYDOWN and event.key == pygame.K_1:
                            main()
                        if event.type == pygame.KEYDOWN and event.key == pygame.K_2:
                            pygame.quit()

            if ball.y > self.height - self.ball_width or ball.y < 0:
                ball.angle = -ball.angle

    def check_ball_hits_paddle(self):
        for ball in self.balls:
            for paddle in self.paddles:
                if ball.colliderect(paddle):
                    ball.velocity = -ball.velocity
                    if headers['player'] == '1':
                        ball.angle = random.randint(-10, 10)
                    break

    def game_loop(self):
        
        my_counter = 0

        msp432_paddle_percentage = 0

        other_paddle_percentage = 0
        
        while True:
            my_counter += 1
            msp432_paddle_percentage,other_paddle_percentage = self.request_state()

            for event in pygame.event.get():
                # Add some extra ways to exit the game.
                if event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
                    return

            if headers['player'] == '1':
                self.check_ball_hits_paddle()
                self.check_ball_hits_wall()

            # Redraw the screen.
            self.screen.fill((0, 0, 0))

            msp_paddle = self.paddles[0]
            other_paddle = self.paddles[1]

            msp_paddle.move_paddle(self.height, msp432_paddle_percentage, 1)
            other_paddle.move_paddle(self.height, other_paddle_percentage, 2)
            pygame.draw.rect(self.screen, self.color, msp_paddle)
            pygame.draw.rect(self.screen, self.color, other_paddle)
                

            # loop that moves the ball .
            for ball in self.balls:
                ball.move_ball()
                pygame.draw.rect(self.screen, self.color, ball)

            pygame.draw.rect(self.screen, self.color, self.central_line)

            pygame.display.flip()
            self.clock.tick(60)

    def request_state(self):
        # Request the page:
        if headers['player'] == '1':
            headers['x'] = str(self.balls[0].x)
            headers['y'] = str(self.balls[0].y)
            headers['angle'] = str(self.balls[0].angle)
            
        print(headers)
            
        response = requests.get("http://"+ip_addr, headers=headers)
        #print(response.text)
        
        # Parse the data we need out of the response with regex (don't need to worry about this stuff)
        # Eventually, we will need to parse out which player we are when we get to two-player
        
        other_paddle_match = 0
        if headers['player'] == '1':
            other_paddle_match = float(self.other_paddle_pattern.findall(response.text)[0])
        
        msp432_paddle_match = float(self.msp432_paddle_pattern.findall(response.text)[0])
       
        if headers['player'] == '2':
            x_match = int(self.x_pattern.findall(response.text)[0])
            y_match = int(self.y_pattern.findall(response.text)[0])
            angle_match = int(self.angle_pattern.findall(response.text)[0])
            game_over_match = self.game_over_pattern.findall(response.text)[0]
            headers['game_over'] = game_over_match
            print(headers)
            
            headers['x'] = str(x_match)
            headers['y'] = str(y_match)
            headers['angle'] = str(angle_match)
            
            self.balls[0].x = x_match
            self.balls[0].y = y_match
            self.balls[0].angle = angle_match

        return msp432_paddle_match/100, other_paddle_match/100
           

def main():
    headers['game_over'] = '0'
    pong = Pong()
    pong.game_loop()


if __name__ == '__main__':
    main()
