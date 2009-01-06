struct test_redirections {
  ssize_t (*write)(int fd, const void *buf, size_t count);
};

struct test_redirections *test_redirections();
