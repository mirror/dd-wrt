#ifndef TRAYICON_H_171149531
#define TRAYICON_H_171149531

class CFrontendDlg;

class TrayIcon {
public:
  TrayIcon(CFrontendDlg &, HINSTANCE);
  ~TrayIcon();

  void displayPopup();

  static TrayIcon *getInstance() {
    return instance;
  }
  enum status { CONNECTED, ON, OFF };

  void setStatus(status con_status, const char *message);

private:
  void setTrayAppearance(bool, unsigned int, const char *message);

  HINSTANCE hInst;

  CFrontendDlg & main_dlg;

  friend LRESULT CALLBACK TrayIconProc(HWND, UINT, WPARAM, LPARAM);
  static TrayIcon *instance;
};

#endif // TRAYICON_H_171149531

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
