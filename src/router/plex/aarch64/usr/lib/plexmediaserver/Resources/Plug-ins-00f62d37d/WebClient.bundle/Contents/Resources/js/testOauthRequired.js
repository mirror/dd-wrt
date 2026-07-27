window.fetch('/oauth2/auth').then((response) => {
  // If the response is 401 that means the oauth2 exists and we should try to
  // authorize the user
  // See: https://oauth2-proxy.github.io/oauth2-proxy/endpoints
  if (response.status === 401) {
    window.location =
      '/oauth2/sign_in?rd=' + encodeURIComponent(window.location.pathname);
  }
});
