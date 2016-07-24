class recfilter(object):
    lastval = 0
    a = None
    b = None

    def __init__(self, fcoef, initval):
        self.lastval = float(initval)
        self.a = 1.0 - float(fcoef)
        self.b = float(fcoef)

    def apply(self, x):
        self.lastval = self.a * float(x) + self.b * self.lastval
        return self.lastval

class recfilter_int(object):
    lastval = None
    a = None
    b = None
    scale = None
    scale_in = None

    def __init__(self, fcoef, scale, scale_out, initval):
        self.lastval = initval
        self.a = scale - fcoef
        self.b = fcoef
        self.scale = scale
        self.scale_in = scale * scale_out
        print 'self.a, self.b, self.scale, self.scale_in =', self.a, self.b, self.scale, self.scale_in

    def apply(self, x):
        lastval = self.lastval
        print 'apply', (self.a * x * self.scale_in) / self.scale, (self.b * lastval) / self.scale
        nextval = (self.a * x * self.scale_in + self.b * lastval) / self.scale
        print 'nextval =', nextval, 'nextval - lastval = ', nextval - lastval
        self.lastval += nextval - lastval
        
        return nextval / self.scale

class ClassIDetector(object):
    ref_clk = 0
    tp_clk = 0
    fl = None
    fl_f = None

    def clk_tick(self):
        self.ref_clk += 1
        if self.fl == None:
            self.tp_clk += 1
            self.fl = recfilter_int(9991, 10000, 1000, 0)
            self.fl_f = recfilter(0.9991, 0)

    def get_tp_error_raw(self, tp_clk):
        err0r = (self.ref_clk & 0x1) ^ (tp_clk & 0x1)
        return err0r

    def tp_tick(self):
        tp_clk = self.tp_clk
        self.tp_clk += 1

        raw_error = self.get_tp_error_raw(tp_clk)
        print raw_error

        rval = self.fl.apply(raw_error)
        print 'CSV3: %f,%d' % (float(self.ref_clk), rval)
        print 'CSV4: %f,%f' % (float(self.ref_clk), self.fl_f.apply(raw_error))
        print 'err0r filtered', rval
        return rval

class PFDi(object):
    ref_clk = 0
    last_ref_clk = 0
    tp_clk = 0
    fl = None
    fl_f = None

    def clk_tick(self):
        print 'CSV: %f,%d' % (float(self.ref_clk), 0)
        print 'CSV: %f,%d' % (float(self.ref_clk) + 0.001, 1)
        print 'CSV: %f,%d' % (float(self.ref_clk) + 0.5 - 0.001, 1)
        print 'CSV: %f,%d' % (float(self.ref_clk) + 0.5, 0)
        if self.fl == None:
            self.tp_clk += 1
            self.fl = recfilter_int(99, 100, 1000000, 0)
            self.fl_f = recfilter(0.99, 0)
        self.ref_clk += 1

    def get_tp_error_raw(self):
        t_since_last = float(self.ref_clk - self.last_ref_clk) / 2.0
        print 'CSV1: %f,%d' % (float(self.last_ref_clk), 0)
        print 'CSV1: %f,%d' % (float(self.last_ref_clk) + 0.001, 1)
        print 'CSV1: %f,%d' % (float(self.ref_clk) - t_since_last - 0.001, 1)
        print 'CSV1: %f,%d' % (float(self.ref_clk) - t_since_last, 0)
        self.last_ref_clk = self.ref_clk

        #print 'in ref', self.ref_clk, 'tp', self.tp_clk
        tp_clk = self.tp_clk
        self.tp_clk += 1
        ref_clk = self.ref_clk

        err0r = ref_clk - tp_clk
        print 'err0r', -err0r
        self.tp_clk += err0r
        #if err0r > 0:
        #    self.tp_clk += 1
        #print 'out ref', self.ref_clk, 'tp', self.tp_clk
        print 'CSV2: %f,%d' % (float(self.ref_clk), -err0r)
        return -err0r

    def tp_tick(self):
        raw_error = self.get_tp_error_raw()
        if raw_error > 0:
            raw_error = 1
        elif raw_error < 0:
            raw_error = -1
        rval = self.fl.apply(raw_error)
        print 'CSV3: %f,%d' % (float(self.ref_clk), rval)
        print 'CSV4: %f,%f' % (float(self.ref_clk), self.fl_f.apply(raw_error))
        print 'err0r filtered', rval
        return rval

if __name__ == '__main__':
    pf = PFDi()
    pf = ClassIDetector()

    for i in range(0, 1000):
        print '>> cycle'
        pf.clk_tick()
        pf.clk_tick()
        e = pf.tp_tick()
        pf.clk_tick()
        pf.clk_tick()
        e = pf.tp_tick()
        pf.clk_tick()
        e = pf.tp_tick()
        e = pf.tp_tick()
        e = pf.tp_tick()

    print 'cycle---'
    pf.clk_tick()
    #pf.clk_tick()
    for i in range(0, 200):
        e = pf.tp_tick()
        pf.clk_tick()

#from PFDi import recfilter_int, recfilter
#a=recfilter_int(999, 1000, 1000000, 0)
#b=recfilter(0.999, 0)
#for i in range(1,30):
#    print a.apply(1)
#    print b.apply(1)
#print 'switch'
#for i in range(1,20):
#    print a.apply(0)
#    print b.apply(0)

