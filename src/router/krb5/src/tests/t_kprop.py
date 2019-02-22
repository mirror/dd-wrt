from k5test import *

conf_replica = {'dbmodules': {'db': {'database_name': '$testdir/db.replica'}}}

def setup_acl(realm):
    acl_file = os.path.join(realm.testdir, 'kpropd-acl')
    acl = open(acl_file, 'w')
    acl.write(realm.host_princ + '\n')
    acl.close()

def check_output(kpropd):
    output('*** kpropd output follows\n')
    while True:
        line = kpropd.stdout.readline()
        if 'Database load process for full propagation completed' in line:
            break
        output('kpropd: ' + line)
        if 'Rejected connection' in line:
            fail('kpropd rejected connection from kprop')

# kprop/kpropd are the only users of krb5_auth_con_initivector, so run
# this test over all enctypes to exercise mkpriv cipher state.
for realm in multipass_realms(create_user=False):
    replica = realm.special_env('replica', True, kdc_conf=conf_replica)

    # Set up the kpropd acl file.
    setup_acl(realm)

    # Create the replica db.
    dumpfile = os.path.join(realm.testdir, 'dump')
    realm.run([kdb5_util, 'dump', dumpfile])
    realm.run([kdb5_util, 'load', dumpfile], replica)
    realm.run([kdb5_util, 'stash', '-P', 'master'], replica)

    # Make some changes to the master db.
    realm.addprinc('wakawaka')

    # Start kpropd.
    kpropd = realm.start_kpropd(replica, ['-d'])

    realm.run([kdb5_util, 'dump', dumpfile])
    realm.run([kprop, '-f', dumpfile, '-P', str(realm.kprop_port()), hostname])
    check_output(kpropd)

    realm.run([kadminl, 'listprincs'], replica, expected_msg='wakawaka')

# default_realm tests follow.
# default_realm and domain_realm different than realm.realm (test -r argument).
conf_rep2 = {'dbmodules': {'db': {'database_name': '$testdir/db.replica2'}}}
krb5_conf_rep2 = {'libdefaults': {'default_realm': 'FOO'},
                  'domain_realm': {hostname: 'FOO'}}
# default_realm and domain_realm map differ.
conf_rep3 = {'dbmodules': {'db': {'database_name': '$testdir/db.replica3'}}}
krb5_conf_rep3 = {'domain_realm':  {hostname: 'BAR'}}

realm = K5Realm(create_user=False)
replica2 = realm.special_env('replica2', True, kdc_conf=conf_rep2,
                             krb5_conf=krb5_conf_rep2)
replica3 = realm.special_env('replica3', True, kdc_conf=conf_rep3,
                             krb5_conf=krb5_conf_rep3)

setup_acl(realm)

# Create the replica db.
dumpfile = os.path.join(realm.testdir, 'dump')
realm.run([kdb5_util, 'dump', dumpfile])
realm.run([kdb5_util, '-r', realm.realm, 'load', dumpfile], replica2)
realm.run([kdb5_util, 'load', dumpfile], replica3)

# Make some changes to the master db.
realm.addprinc('wakawaka')

# Test override of default_realm with -r realm argument.
kpropd = realm.start_kpropd(replica2, ['-r', realm.realm, '-d'])
realm.run([kdb5_util, 'dump', dumpfile])
realm.run([kprop, '-r', realm.realm, '-f', dumpfile, '-P',
           str(realm.kprop_port()), hostname])
check_output(kpropd)
realm.run([kadminl, '-r', realm.realm, 'listprincs'], replica2,
          expected_msg='wakawaka')

stop_daemon(kpropd)

# Test default_realm and domain_realm mismatch.
kpropd = realm.start_kpropd(replica3, ['-d'])
realm.run([kdb5_util, 'dump', dumpfile])
realm.run([kprop, '-f', dumpfile, '-P', str(realm.kprop_port()), hostname])
check_output(kpropd)
realm.run([kadminl, 'listprincs'], replica3, expected_msg='wakawaka')

success('kprop tests')
