
provider dnscrypt_proxy {
  probe certs__update__start();
  probe certs__update__received();
  probe certs__update__error__communication();
  probe certs__update__error__security();
  probe certs__update__error__nocerts();
  probe certs__update__retry();
  probe certs__update__done(unsigned char[32]);

  probe request__udp__start(void *);
  probe request__udp__replied(void *);
  probe request__udp__truncated(void *);
  probe request__udp__overloaded();
  probe request__udp__network_error(void *);
  probe request__udp__done(void *);

  probe request__udp__proxy_resolver__start(void *);
  probe request__udp__proxy_resolver__replied(void *);
  probe request__udp__proxy_resolver__got_invalid_reply(void *);
  probe request__udp__proxy_resolver__done(void *);

  probe request__udp__retry_scheduled(void *, unsigned char);
  probe request__udp__retry(void *, unsigned char);
  probe request__udp__timeout(void *);

  probe request__tcp__start(void *);
  probe request__tcp__replied(void *);
  probe request__tcp__overloaded();
  probe request__tcp__network_error(void *);
  probe request__tcp__done(void *);

  probe request__tcp__proxy_resolver__start(void *);
  probe request__tcp__proxy_resolver__connected(void *);
  probe request__tcp__proxy_resolver__replied(void *);
  probe request__tcp__proxy_resolver__got_invalid_reply(void *);
  probe request__tcp__proxy_resolver__network_error(void *);
  probe request__tcp__proxy_resolver__done(void *);

  probe request__tcp__timeout(void *);

  probe request__curve_start(void *, size_t);
  probe request__curve_error(void *);
  probe request__curve_done(void *, size_t);

  probe request__uncurve_start(void *, size_t);
  probe request__uncurve_error(void *);
  probe request__uncurve_done(void *, size_t);

  probe request__plugins__pre__start(void *, size_t, size_t);
  probe request__plugins__pre__error(void *, int);
  probe request__plugins__pre__done(void *, size_t, size_t);

  probe request__plugins__post__start(void *, size_t, size_t);
  probe request__plugins__post__error(void *, int);
  probe request__plugins__post__done(void *, size_t, size_t);

  probe status__requests__active(unsigned int, unsigned int);
};
