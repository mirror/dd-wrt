check that blob_parse is producing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ export TEST_INPUTS="$TESTDIR/inputs"
  $ export FUZZ_CORPUS="$TESTDIR/../fuzz/corpus"

  $ valgrind --quiet --leak-check=full test-blob-parse $TEST_INPUTS/key-build.ucert
  === CHAIN ELEMENT 01 ===
  signature:
  ---
  untrusted comment: signed by key 84bfc88a17166577
  RWSEv8iKFxZld+bQ+NTqCdDlHOuVYNw5Qw7Q8shjfMgFJcTqrzaqO0bysjIQhTadmcwvWiWvHlyMcwAXSix2BYdfghz/zhDjvgU=
  ---
  payload:
  ---
  "ucert": {
  \t"certtype": 1, (esc)
  \t"validfrom": 1546188410, (esc)
  \t"expiresat": 1577724410, (esc)
  \t"pubkey": "untrusted comment: Local build key\\nRWSEv8iKFxZld6vicE1icWhYNfEV9PM7C9MKUKl+YNEKB+PdAWGDF5Z9\\n" (esc)
  }
  ---

  $ valgrind --quiet --leak-check=full test-blob-parse $TEST_INPUTS/signature.ucert
  === CHAIN ELEMENT 01 ===
  signature:
  ---
  untrusted comment: signed by key ca85add129e64bab
  RWTKha3RKeZLq0Sb8kCH9p/3BcFFud8rJnZiRICyRNhjbbpeZSwO2VhkwGaMd7ujW2/YSvT3O67pB0QguV6czgpP5kLX4AKBaQ4=
  ---
  payload:
  ---
  "ucert": {
  \t"certtype": 1, (esc)
  \t"validfrom": 1588532405, (esc)
  \t"expiresat": 1620068405, (esc)
  \t"pubkey": "untrusted comment: Local build key\\nRWTKha3RKeZLq1EaPsqvnXu+FqLMHZIS7nvDgwjpRo69j+th6eihGvQo\\n" (esc)
  }
  ---
  === CHAIN ELEMENT 02 ===
  signature:
  ---
  untrusted comment: signed by key ca85add129e64bab
  RWTKha3RKeZLq9VW9CIMyumCQ4J0iFPLQYXr/YvUhw0OTrwpSh2XpKaRZQNZCXfO8ooMOCvG2TPor2veDjskHP1R2RGPIHp57wA=
  ---

  $ valgrind --quiet --leak-check=full test-blob-parse $TEST_INPUTS/invalid.ucert
  cannot parse cert invalid.ucert

  $ test-blob-parse-san $TEST_INPUTS/key-build.ucert
  === CHAIN ELEMENT 01 ===
  signature:
  ---
  untrusted comment: signed by key 84bfc88a17166577
  RWSEv8iKFxZld+bQ+NTqCdDlHOuVYNw5Qw7Q8shjfMgFJcTqrzaqO0bysjIQhTadmcwvWiWvHlyMcwAXSix2BYdfghz/zhDjvgU=
  ---
  payload:
  ---
  "ucert": {
  \t"certtype": 1, (esc)
  \t"validfrom": 1546188410, (esc)
  \t"expiresat": 1577724410, (esc)
  \t"pubkey": "untrusted comment: Local build key\\nRWSEv8iKFxZld6vicE1icWhYNfEV9PM7C9MKUKl+YNEKB+PdAWGDF5Z9\\n" (esc)
  }
  ---

  $ test-blob-parse-san $TEST_INPUTS/signature.ucert
  === CHAIN ELEMENT 01 ===
  signature:
  ---
  untrusted comment: signed by key ca85add129e64bab
  RWTKha3RKeZLq0Sb8kCH9p/3BcFFud8rJnZiRICyRNhjbbpeZSwO2VhkwGaMd7ujW2/YSvT3O67pB0QguV6czgpP5kLX4AKBaQ4=
  ---
  payload:
  ---
  "ucert": {
  \t"certtype": 1, (esc)
  \t"validfrom": 1588532405, (esc)
  \t"expiresat": 1620068405, (esc)
  \t"pubkey": "untrusted comment: Local build key\\nRWTKha3RKeZLq1EaPsqvnXu+FqLMHZIS7nvDgwjpRo69j+th6eihGvQo\\n" (esc)
  }
  ---
  === CHAIN ELEMENT 02 ===
  signature:
  ---
  untrusted comment: signed by key ca85add129e64bab
  RWTKha3RKeZLq9VW9CIMyumCQ4J0iFPLQYXr/YvUhw0OTrwpSh2XpKaRZQNZCXfO8ooMOCvG2TPor2veDjskHP1R2RGPIHp57wA=
  ---

  $ test-blob-parse-san $TEST_INPUTS/invalid.ucert
  cannot parse cert invalid.ucert

  $ for blob in $(LC_ALL=C find $FUZZ_CORPUS -type f | sort ); do
  >   valgrind --quiet --leak-check=full test-blob-parse $blob; \
  >   test-blob-parse-san $blob; \
  > done
  cannot parse cert 71520a5c4b5ca73903216857abbad54a8002d44a
  cannot parse cert 71520a5c4b5ca73903216857abbad54a8002d44a
  cannot parse cert c1dfd96eea8cc2b62785275bca38ac261256e278
  cannot parse cert c1dfd96eea8cc2b62785275bca38ac261256e278
  cannot parse cert c42ac1c46f1d4e211c735cc7dfad4ff8391110e9
  cannot parse cert c42ac1c46f1d4e211c735cc7dfad4ff8391110e9
  cannot parse cert crash-1b8fb1be45db3aff7699100f497fb74138f3df4f
  cannot parse cert crash-1b8fb1be45db3aff7699100f497fb74138f3df4f
  cannot parse cert crash-333757b203a44751d3535f24b05f467183a96d09
  cannot parse cert crash-333757b203a44751d3535f24b05f467183a96d09
  cannot parse cert crash-4c4d2c3c9ade5da9347534e290305c3b9760f627
  cannot parse cert crash-4c4d2c3c9ade5da9347534e290305c3b9760f627
  cannot parse cert crash-5e9937b197c88bf4e7b7ee2612456cad4cb83f5b
  cannot parse cert crash-5e9937b197c88bf4e7b7ee2612456cad4cb83f5b
  cannot parse cert crash-75b146c4e6fac64d3e62236b27c64b50657bab2a
  cannot parse cert crash-75b146c4e6fac64d3e62236b27c64b50657bab2a
  cannot parse cert crash-813f3e68661da09c26d4a87dbb9d5099e92be50f
  cannot parse cert crash-813f3e68661da09c26d4a87dbb9d5099e92be50f
  cannot parse cert crash-98595faa58ba01d85ba4fd0b109cd3d490b45795
  cannot parse cert crash-98595faa58ba01d85ba4fd0b109cd3d490b45795
  cannot parse cert crash-a3585b70f1c7ffbdec10f6dadc964336118485c4
  cannot parse cert crash-a3585b70f1c7ffbdec10f6dadc964336118485c4
  cannot parse cert crash-b3585b70f1c7ffbdec10f6dadc964336118485c4
  cannot parse cert crash-b3585b70f1c7ffbdec10f6dadc964336118485c4
  cannot parse cert crash-d0f3aa7d60a094b021f635d4edb7807c055a4ea1
  cannot parse cert crash-d0f3aa7d60a094b021f635d4edb7807c055a4ea1
  cannot parse cert crash-df9d1243057b27bbad6211e5a23d1cb699028aa2
  cannot parse cert crash-df9d1243057b27bbad6211e5a23d1cb699028aa2
  cannot parse cert crash-e0f8ecc694d96a09a1fced27b2a0838b670d34a0
  cannot parse cert crash-e0f8ecc694d96a09a1fced27b2a0838b670d34a0
  cannot parse cert crash-e2fd5ecb3b37926743256f1083f47a07c39e10c2
  cannot parse cert crash-e2fd5ecb3b37926743256f1083f47a07c39e10c2
  cannot parse cert valid-blobmsg.bin
  cannot parse cert valid-blobmsg.bin
