check that runqueue is producing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ valgrind --quiet --leak-check=full test-runqueue
  [1/1] start 'sleep 1' (killer)
  [1/1] killing process (killer)
  [0/1] finish 'sleep 1' (killer) 
  [0/1] finish 'sleep 1' (killer) 
  [0/1] finish 'sleep 1' (killer) 
  [1/1] start 'sleep 1' (sleeper)
  [1/1] cancel 'sleep 1' (sleeper)
  [0/1] finish 'sleep 1' (sleeper) 
  [1/1] start 'sleep 1' (sleeper)
  [1/1] cancel 'sleep 1' (sleeper)
  [0/1] finish 'sleep 1' (sleeper) 
  [1/1] start 'sleep 1' (sleeper)
  [1/1] cancel 'sleep 1' (sleeper)
  [0/1] finish 'sleep 1' (sleeper) 
  All done!

  $ test-runqueue-san
  [1/1] start 'sleep 1' (killer)
  [1/1] killing process (killer)
  [0/1] finish 'sleep 1' (killer) 
  [0/1] finish 'sleep 1' (killer) 
  [0/1] finish 'sleep 1' (killer) 
  [1/1] start 'sleep 1' (sleeper)
  [1/1] cancel 'sleep 1' (sleeper)
  [0/1] finish 'sleep 1' (sleeper) 
  [1/1] start 'sleep 1' (sleeper)
  [1/1] cancel 'sleep 1' (sleeper)
  [0/1] finish 'sleep 1' (sleeper) 
  [1/1] start 'sleep 1' (sleeper)
  [1/1] cancel 'sleep 1' (sleeper)
  [0/1] finish 'sleep 1' (sleeper) 
  All done!
