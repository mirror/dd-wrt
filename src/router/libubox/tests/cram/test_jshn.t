set jshn for convenience:

  $ [ -n "$JSHN" ] && export PATH="$(dirname "$JSHN"):$PATH"
  $ alias jshn="valgrind --quiet --leak-check=full jshn"

check usage:

  $ jshn
  Usage: jshn [-n] [-i] -r <message>|-R <file>|-o <file>|-p <prefix>|-w
  [2]

  $ jshn-san
  Usage: jshn-san [-n] [-i] -r <message>|-R <file>|-o <file>|-p <prefix>|-w
  [2]

test bad json:

  $ jshn -r '[]'
  Failed to parse message data
  [1]

  $ jshn-san -r '[]'
  Failed to parse message data
  [1]

test good json:

  $ jshn -r '{"foo": "bar", "baz": {"next": "meep"}}'
  json_init;
  json_add_string 'foo' 'bar';
  json_add_object 'baz';
  json_add_string 'next' 'meep';
  json_close_object;

  $ jshn-san -r '{"foo": "bar", "baz": {"next": "meep"}}'
  json_init;
  json_add_string 'foo' 'bar';
  json_add_object 'baz';
  json_add_string 'next' 'meep';
  json_close_object;

test json from file:

  $ echo '[]' > test.json; jshn -R test.json
  Failed to parse message data
  [1]

  $ echo '[]' > test.json; jshn-san -R test.json
  Failed to parse message data
  [1]

  $ jshn -R nada.json
  Error opening nada.json
  [3]

  $ jshn-san -R nada.json
  Error opening nada.json
  [3]

  $ echo '{"foo": "bar", "baz": {"next": "meep"}}' > test.json; jshn -R test.json
  json_init;
  json_add_string 'foo' 'bar';
  json_add_object 'baz';
  json_add_string 'next' 'meep';
  json_close_object;

  $ echo '{"foo": "bar", "baz": {"next": "meep"}}' > test.json; jshn-san -R test.json
  json_init;
  json_add_string 'foo' 'bar';
  json_add_object 'baz';
  json_add_string 'next' 'meep';
  json_close_object;

test json formatting without prepared environment:

  $ jshn -p procd -w
  { }

  $ jshn-san -p procd -w
  { }

  $ jshn -i -p procd -w
  {
  \t (esc)
  }

  $ jshn-san -i -p procd -w
  {
  \t (esc)
  }

  $ jshn -i -n -p procd -w
  {
  \t (esc)
  } (no-eol)

  $ jshn-san -i -n -p procd -w
  {
  \t (esc)
  } (no-eol)

  $ jshn -p procd -o test.json; cat test.json
  { }

  $ jshn-san -p procd -o test.json; cat test.json
  { }

  $ jshn -i -p procd -o test.json; cat test.json
  {
  \t (esc)
  }

  $ jshn-san -i -p procd -o test.json; cat test.json
  {
  \t (esc)
  }

  $ jshn -i -n -p procd -o test.json; cat test.json
  {
  \t (esc)
  } (no-eol)

  $ jshn-san -i -n -p procd -o test.json; cat test.json
  {
  \t (esc)
  } (no-eol)

  $ chmod oug= test.json
  $ jshn -i -n -p procd -o test.json
  Error opening test.json
  [3]
  $ jshn-san -i -n -p procd -o test.json
  Error opening test.json
  [3]
  $ rm -f test.json

test json formatting with prepared environment:

  $ export procdJSON_CUR=J_V
  $ export procdJ_A3_1=/sbin/urngd
  $ export procdJ_T1_instance1=J_T2
  $ export procdJ_T2_command=J_A3
  $ export procdJ_V_data=J_T5
  $ export procdJ_V_instances=J_T1
  $ export procdJ_V_name=urngd
  $ export procdJ_V_script=/etc/init.d/urngd
  $ export procdJ_V_triggers=J_A4
  $ export procdK_J_A3=1
  $ export procdK_J_A4=
  $ export procdK_J_T1=instance1
  $ export procdK_J_T2=command
  $ export procdK_J_T5=
  $ export procdK_J_V="name script instances triggers data"
  $ export procdT_J_A3_1=string
  $ export procdT_J_T1_instance1=object
  $ export procdT_J_T2_command=array
  $ export procdT_J_V_data=object
  $ export procdT_J_V_instances=object
  $ export procdT_J_V_name=string
  $ export procdT_J_V_script=string
  $ export procdT_J_V_triggers=array

  $ jshn -p procd -w
  { "name": "urngd", "script": "\/etc\/init.d\/urngd", "instances": { "instance1": { "command": [ "\/sbin\/urngd" ] } }, "triggers": [ ], "data": { } }

  $ jshn-san -p procd -w
  { "name": "urngd", "script": "\/etc\/init.d\/urngd", "instances": { "instance1": { "command": [ "\/sbin\/urngd" ] } }, "triggers": [ ], "data": { } }

  $ jshn -i -p procd -w
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  }

  $ jshn-san -i -p procd -w
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  }

  $ jshn -n -i -p procd -w
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  } (no-eol)

  $ jshn-san -n -i -p procd -w
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  } (no-eol)

  $ jshn -p procd -o test.json; cat test.json
  { "name": "urngd", "script": "\/etc\/init.d\/urngd", "instances": { "instance1": { "command": [ "\/sbin\/urngd" ] } }, "triggers": [ ], "data": { } }

  $ jshn-san -p procd -o test.json; cat test.json
  { "name": "urngd", "script": "\/etc\/init.d\/urngd", "instances": { "instance1": { "command": [ "\/sbin\/urngd" ] } }, "triggers": [ ], "data": { } }

  $ jshn -i -p procd -o test.json; cat test.json
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  }

  $ jshn-san -i -p procd -o test.json; cat test.json
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  }

  $ jshn -n -i -p procd -o test.json; cat test.json
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  } (no-eol)

  $ jshn-san -n -i -p procd -o test.json; cat test.json
  {
  \t"name": "urngd", (esc)
  \t"script": "/etc/init.d/urngd", (esc)
  \t"instances": { (esc)
  \t\t"instance1": { (esc)
  \t\t\t"command": [ (esc)
  \t\t\t\t"/sbin/urngd" (esc)
  \t\t\t] (esc)
  \t\t} (esc)
  \t}, (esc)
  \t"triggers": [ (esc)
  \t\t (esc)
  \t], (esc)
  \t"data": { (esc)
  \t\t (esc)
  \t} (esc)
  } (no-eol)

  $ chmod oug= test.json
  $ jshn -n -i -p procd -o test.json
  Error opening test.json
  [3]
  $ jshn-san -n -i -p procd -o test.json
  Error opening test.json
  [3]
