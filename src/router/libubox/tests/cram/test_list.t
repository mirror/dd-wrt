check that list is producing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ valgrind --quiet --leak-check=full test-list
  init_list: list_empty: yes
  init_list: list_add_tail: zero one two three four five six seven eight nine ten eleven twelve 
  init_list: list_empty: no
  test_basics: first=zero last=twelve
  test_basics: 'zero' is first, yes
  test_basics: 'twelve' is last, yes
  test_basics: removing 'twelve' and 'zero'
  test_basics: first=one last=eleven
  test_basics: 'one' is first, yes
  test_basics: 'eleven' is last, yes
  test_basics: moving 'one' to the tail
  test_basics: first=two last=one
  test_basics: 'two' is first, yes
  test_basics: 'one' is last, yes
  test_basics: list_for_each_entry: two three four five six seven eight nine ten eleven one 
  test_basics: list_for_each_entry_reverse: one eleven ten nine eight seven six five four three two 
  test_basics: delete all entries
  test_basics: list_empty: yes
  init_list: list_empty: yes
  init_list: list_add_tail: zero one two three four five six seven eight nine ten eleven twelve 
  init_list: list_empty: no
  test_while_list_empty: delete all entries
  test_while_list_empty: list_empty: yes

  $ test-list-san
  init_list: list_empty: yes
  init_list: list_add_tail: zero one two three four five six seven eight nine ten eleven twelve 
  init_list: list_empty: no
  test_basics: first=zero last=twelve
  test_basics: 'zero' is first, yes
  test_basics: 'twelve' is last, yes
  test_basics: removing 'twelve' and 'zero'
  test_basics: first=one last=eleven
  test_basics: 'one' is first, yes
  test_basics: 'eleven' is last, yes
  test_basics: moving 'one' to the tail
  test_basics: first=two last=one
  test_basics: 'two' is first, yes
  test_basics: 'one' is last, yes
  test_basics: list_for_each_entry: two three four five six seven eight nine ten eleven one 
  test_basics: list_for_each_entry_reverse: one eleven ten nine eight seven six five four three two 
  test_basics: delete all entries
  test_basics: list_empty: yes
  init_list: list_empty: yes
  init_list: list_add_tail: zero one two three four five six seven eight nine ten eleven twelve 
  init_list: list_empty: no
  test_while_list_empty: delete all entries
  test_while_list_empty: list_empty: yes
