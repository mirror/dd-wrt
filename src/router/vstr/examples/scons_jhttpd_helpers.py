
import sys
import os
import glob

def init_builders(bldr):
    global jhighlight_bld
    global jssi_bld
    global make_examples_index_bld
    global make_index_bld
    global ln_bld
    global lns_bld
    
    jhighlight_bld = bldr(action = '\
jhighlight --css-file=http://www.and.org/f_c.css $SOURCE > $TARGET')

    jssi_bld       = bldr(action = 'jssi       $SOURCE > $TARGET')

    make_examples_index_bld = bldr(action = '\
make_index.pl --filter-args="-A README -A Makefile -A SConstruct --acpt-name-end .c --acpt-name-end .h --deny-all" --output $TARGET $SOURCE')

    make_index_bld = bldr(action = '\
make_index.pl $MKINDXARGS --output $TARGET $SOURCE')

    ln_bld  = bldr(action = 'ln $SOURCE $TARGET')
    lns_bld = bldr(action = 'ln -s $SOURCE $TARGET')

conf = {'GENERATE_INDEX_NAME' : "index.html",
	'ROOT_DIR' : "html/",
	'GENERATED_DIR' : "generated_html/",
	'CONF_DIR' : "conf/",
	}

# Generate index.html files automatically in a different dir...
def gen_dif_idx(env, data, x, rec=False):
    subdir = x
    def gen__name(x):
        indx = conf['GENERATE_INDEX_NAME']
        f1 = conf['GENERATED_DIR'] + subdir + '/' + x + '/' + indx
        f2 = conf['ROOT_DIR']      + subdir + '/' + x + '/'
        f3 = conf['CONF_DIR']      + subdir + '/' + x + '/' + indx
        data.append([f1, env.Dir(f2), f3])

        if not rec:
            return
		
        for y in glob.glob(f2 + '*'):
            if os.path.islink(y) or not os.path.isdir(y):
                continue
            gen__name(x + '/' + os.path.basename(y))
    return gen__name

def apply_dif_idx(data, Make_indx, LN, conf_gen_html):
    for i in data:
        Make_indx(i[0], i[1])
        LN(i[2], conf_gen_html)


# Generate index.html files automatically in same dir...
def gen_sam_idx(env, data, x, rec=False):
    subdir = x
    def gen__name(x):
        indx = conf['GENERATE_INDEX_NAME']
        f1 = conf['ROOT_DIR'] + subdir + '/' + x + '/' + indx
        f2 = conf['ROOT_DIR'] + subdir + '/' + x + '/'
        # all this is needed due to bug#1185284
        files = []
        for name in glob.glob(f2 + '*'):
            if os.path.basename(name) == indx:
                continue
            if os.path.basename(name) == indx + ".gz":
                continue
            if os.path.basename(name) == indx + ".bz2":
                continue
			
            if os.path.islink(name) and not os.path.isfile(name):
                continue # recursion bug#1185293
            if os.path.getsize(name) > (1000 * 1000 * 1000):
                continue # string error bug#1185299

            if os.path.isdir(name):
                files.append(env.Dir(name))
            else:
                files.append(name)
        if len(files):
            data.append([f1, env.Dir(f2), files])
        
        if not rec:
            return
		
        for y in glob.glob(f2 + '*'):
            if os.path.islink(y) or not os.path.isdir(y):
                continue
            gen__name(x + '/' + os.path.basename(y))
		
    return gen__name

def apply_sam_idx(data, env, Make_indx):
    for i in data:
        abcd = Make_indx(i[0], i[1])
        for j in i[2]:
            env.Depends(abcd, j)
        env.Ignore(abcd, i[1])


# Special code for the examples dir.
def apply_sam_examples_code(data, Highlight, LN, ex_neg, ex_txt):
    for pat in data:
        for i in glob.glob(conf['ROOT_DIR'] + pat[0] + pat[1]):
            Highlight(i + ".html", i)

            name = os.path.basename(i)
            conf_name = conf['CONF_DIR'] + pat[0] + name
            LN(conf_name, ex_neg)
            LN(conf_name + ".txt", ex_txt)

def apply_sam_examples_idx(data, env, Make_ex_indx):
    for src in data:
        name = conf['ROOT_DIR'] + src[0]
        abcd = Make_ex_indx(name + "/" + conf['GENERATE_INDEX_NAME'],
                            env.Dir(name))
        for j in glob.glob(name + "/" + src[1]):
            env.Depends(abcd, j)
        env.Ignore(abcd, env.Dir(name))

# Apply over directories
def gen_files_sam_ext(env, data, x, rec=False):
    subdir = x
    def gen__name(x, ext_in=".shtml", ext_out=".html"):
        indx = conf['GENERATE_INDEX_NAME']
        f2 = conf['ROOT_DIR'] + subdir + '/' + x + '/'
        # all this is needed due to bug#1185284
        files = []
        for name in glob.glob(f2 + '*' + ext_in):
            if os.path.basename(name) == indx:
                continue
            if os.path.basename(name) == indx + ".gz":
                continue
            if os.path.basename(name) == indx + ".bz2":
                continue
			
            if os.path.islink(name) and not os.path.isfile(name):
                continue # recursion bug#1185293
            if os.path.getsize(name) > (1000 * 1000 * 1000):
                continue # string error bug#1185299

            out_name = name[:-len(ext_in)] + ext_out
            files.append([out_name, name])
        if len(files):
            data.append([ext_in, ext_out, files])
        
        if not rec:
            return
		
        for y in glob.glob(f2 + '*'):
            if os.path.islink(y) or not os.path.isdir(y):
                continue
            gen__name(x + '/' + os.path.basename(y))
		
    return gen__name

def apply_sam_ssi(data, SSI):
    for i in data:
        ext_in  = i[0]
        ext_out = i[1]
        for fname in i[2]:
            SSI(fname[0], fname[1])
