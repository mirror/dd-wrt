from math import trunc

def sigmoid(x):
    return (x / (1 + abs(x)))

class PFD(object):
    target_clk = None

    def __init__(self, ctime):
        self.target_clk = trunc(ctime) + 1.0

    def get_error(self, ctime, raw_error = False):
        err0r = self.target_clk - ctime
        next_clk = trunc(ctime) + 1.0
        if err0r > 0:
            self.target_clk = next_clk + 1
        else:
            self.target_clk = next_clk
        if not raw_error:
            return sigmoid(err0r)
        return err0r

if __name__ == '__main__':
    p=PFD(0.0)
    print p.get_error(0.75)
    print p.get_error(1.75)
    print p.get_error(3.5)
    print p.get_error(4.5)
    print p.get_error(5.75)
    print p.get_error(6.50)
    print p.get_error(6.90)
    print p.get_error(7.75)
    print p.get_error(10.25)
    p=PFD(0.0)
    print p.get_error(1.01)
    print p.get_error(2.02)
    print p.get_error(3.03)
    print p.get_error(4.04)
    print p.get_error(5.05)
    print p.get_error(16.06)
    print p.get_error(17.07)
    print p.get_error(18.08)
    print p.get_error(19.09)
    print p.get_error(0.75)
    print p.get_error(1.75)
    print p.get_error(3.5)
    print p.get_error(4.5)
    print p.get_error(5.75)
    print p.get_error(6.50)
    print p.get_error(6.90)
    print p.get_error(7.75)
    print p.get_error(10.25)

