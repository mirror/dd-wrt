const SESSION_STORAGE_KEY = 'splashScreenViewed';

try {
  const container = document.querySelector('.plex-preloader-container');
  const splashScreenViewed =
    window.sessionStorage.getItem(SESSION_STORAGE_KEY) === 'true';

  if (splashScreenViewed && container) {
    container.classList.remove('plex-logo');
  } else if (!splashScreenViewed) {
    window.sessionStorage.setItem(SESSION_STORAGE_KEY, 'true');
  }
} catch {
  // Silently fail
}
