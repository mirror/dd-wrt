
;; Help from http://two-wugs.net/emacs/wpdl-mode.el

(defvar conf-mode-hook nil)
(defvar conf-mode-map
  (let ((conf-mode-map (make-keymap)))
;    (define-key conf-mode-map "\C-j" 'newline-and-indent)
    conf-mode-map)
  "Keymap for Conf major mode")

;; Set us up to load by default for .conf files
(add-to-list 'auto-mode-alist '("\\.conf$" . conf-mode))




; Note comments are done via. syntax table...
(defconst conf-font-lock-keywords-1
  (list
   '("\\<\\(AND\\|and\\|OR\\|or\\|||\\|&&\\|NOT\\|not\\|!\\)\\>" . 
     font-lock-function-name-face)
;   '("\\<\\(\\w\\w*\\)\\>" . 
;     font-lock-variable-name-face)
   '("^#!\\( \\|/\\).*" . 
     font-lock-warning-face))
  "Minimal highlighting expressions for Conf mode")

(defconst conf-font-lock-keywords-2
  (append conf-font-lock-keywords-1
	  (list
	   '("[][()]" . font-lock-keyword-face)
	   '("\\<\\(TRUE\\|true\\|FALSE\\|false\\|ELSE\\|else\\)\\>" . 
	     font-lock-constant-face)))
  "Level 2 Keywords to highlight in Conf mode")

; httpd specific keywords...
(defconst conf-font-lock-keywords-3
  (append conf-font-lock-keywords-2
	  (list
	   '("\\<\\(org.and.daemon-conf-1.0\\|org.and.httpd-conf-main-1.0\\|org.and.httpd-conf-req-1.0\\)\\>" . font-lock-function-name-face)))
  "Level 3 Keywords to highlight in Conf mode")

(defconst conf-font-lock-keywords-4
  (append conf-font-lock-keywords-3
	  (list
	   '("\\<return\\>" . font-lock-keyword-face)
	   '("\\<policy\\|connection-policy\\>" . font-lock-builtin-face)))
  "Level 4 Keywords to highlight in Conf mode")


; By default highlight for httpd...
(defvar conf-font-lock-keywords conf-font-lock-keywords-4
  "Default highlighting expressions for Conf mode")


(defun conf-indent-line ()
  "Indent current line as Conf code."
  (interactive)
  (beginning-of-line)
  (indent-line-to 0)		   ; First line is always non-indented
)

(defvar conf-mode-syntax-table
  (let ((conf-mode-syntax-table (make-syntax-table)))
	
    ; This is added so symbols can be more easily parsed
	(modify-syntax-entry ?- "w" conf-mode-syntax-table)
	(modify-syntax-entry ?_ "w" conf-mode-syntax-table)
	(modify-syntax-entry ?. "w" conf-mode-syntax-table)
	(modify-syntax-entry ?, "w" conf-mode-syntax-table)
	(modify-syntax-entry ?` "w" conf-mode-syntax-table)
	(modify-syntax-entry ?{ "w" conf-mode-syntax-table)
	(modify-syntax-entry ?} "w" conf-mode-syntax-table)
	(modify-syntax-entry ?& "w" conf-mode-syntax-table)
	(modify-syntax-entry ?| "w" conf-mode-syntax-table)
	(modify-syntax-entry ?! "w" conf-mode-syntax-table)
	(modify-syntax-entry ?: "w" conf-mode-syntax-table)
	
;;	(modify-syntax-entry ?\( "$" conf-mode-syntax-table)
;;	(modify-syntax-entry ?\) "$" conf-mode-syntax-table)
;;	(modify-syntax-entry ?\[ "$" conf-mode-syntax-table)
;;	(modify-syntax-entry ?\] "$" conf-mode-syntax-table)
	(modify-syntax-entry ?\' "\"" conf-mode-syntax-table)
	(modify-syntax-entry ?\; "<" conf-mode-syntax-table)
	(modify-syntax-entry ?\# "<" conf-mode-syntax-table)
	(modify-syntax-entry ?\n ">" conf-mode-syntax-table)
	conf-mode-syntax-table)
  "Syntax table for conf-mode")
 
(defun conf-mode ()
  (interactive)
  (kill-all-local-variables)
  (use-local-map conf-mode-map)
  (set-syntax-table conf-mode-syntax-table)
  ;; Set up font-lock
  (set (make-local-variable 'font-lock-defaults) '(conf-font-lock-keywords))
  ;; Register our indentation function
  (set (make-local-variable 'indent-line-function) 'conf-indent-line)  
  (setq major-mode 'conf-mode)
  (setq mode-name "CONF")
  (run-hooks 'conf-mode-hook))

(provide 'conf-mode)

;;; conf-mode.el ends here
