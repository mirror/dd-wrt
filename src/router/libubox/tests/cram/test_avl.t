check that avl is producing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ valgrind --quiet --leak-check=full test-avl
  test_basics: insert: 0=zero 0=one 0=two 0=three 0=four 0=five 0=six 0=seven 0=eight 0=nine 0=ten 0=eleven 0=twelve 
  test_basics: insert duplicate: -1=zero -1=one -1=two -1=three -1=four -1=five -1=six -1=seven -1=eight -1=nine -1=ten -1=eleven -1=twelve 
  test_basics: first=eight last=zero
  test_basics: for each element: eight eleven five four nine one seven six ten three twelve two zero 
  test_basics: delete 'one' element
  test_basics: for each element reverse: zero two twelve three ten six seven nine four five eleven eight 
  test_basics: delete all elements

  $ test-avl-san
  test_basics: insert: 0=zero 0=one 0=two 0=three 0=four 0=five 0=six 0=seven 0=eight 0=nine 0=ten 0=eleven 0=twelve 
  test_basics: insert duplicate: -1=zero -1=one -1=two -1=three -1=four -1=five -1=six -1=seven -1=eight -1=nine -1=ten -1=eleven -1=twelve 
  test_basics: first=eight last=zero
  test_basics: for each element: eight eleven five four nine one seven six ten three twelve two zero 
  test_basics: delete 'one' element
  test_basics: for each element reverse: zero two twelve three ten six seven nine four five eleven eight 
  test_basics: delete all elements
