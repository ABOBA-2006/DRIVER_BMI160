class Kalman:
    def __init__(self):
        # how we trust the gyro
        self.Q_angle = 0.01
        
        # how fast the gyro drift can change
        self.Q_bias = 0.8
    
        # how we trust the accel: less value -> more trust
        self.R_measure = 0.5
        
        #filtered angle
        self.angle = 0.0

        # gyro drift
        self.bias = 0.05

        # cov matrix 2x2 of errors
        self.P = [[0.0, 0.0], [0.0, 0.0]]


    def compute(self, new_accel_angle, new_gyro_rate, dt):
        # making a prediction for new angle based on gyro rate
        rate = new_gyro_rate - self.bias
        self.angle += rate * dt

        # updating the matrix of errors. Just using the Kalman Math
        self.P[0][0] += dt * (dt * self.P[1][1] - self.P[0][1] - self.P[1][0] + self.Q_angle)
        self.P[0][1] -= dt * self.P[1][1]
        self.P[1][0] -= dt * self.P[1][1]
        self.P[1][1] += self.Q_bias * dt

        # delta of predicted angle and real angle from accel
        S = self.P[0][0] + self.R_measure

        # calculating the Kalman Gain value
        K = [0.0, 0.0]
        K[0] = self.P[0][0] / S
        K[1] = self.P[1][0] / S

        # the delta between predicted value and real value
        y = new_accel_angle - self.angle
        
        # correct the angle and drift
        self.angle += K[0] * y
        self.bias += K[1] * y
        
        P00_temp = self.P[0][0]
        P01_temp = self.P[0][1]
        # update the cov matrix of errors after correction
        self.P[0][0] -= K[0] * P00_temp
        self.P[0][1] -= K[0] * P01_temp
        self.P[1][0] -= K[1] * P00_temp
        self.P[1][1] -= K[1] * P01_temp

        return self.angle



