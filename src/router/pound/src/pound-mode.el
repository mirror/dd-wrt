;;; mfl-mode.el --- major mode for editing MFL sources

;; Authors: 2025 Sergey Poznyakoff
;; Version: 0.4
;; Keywords: Pound

;; This file is part of Pound
;; Copyright (C) 2025 Sergey Poznyakoff

;; Pound is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3 of the License, or
;; (at your option) any later version.

;; Pound is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with Pound.  If not, see <http://www.gnu.org/licenses/>.

;; Installation:
;; You may wish to use precompiled version of the module. To create it
;; run:
;;    emacs -batch -f batch-byte-compile pound-mode.el
;; Install the file pound-mode.elc (and, optionally, pound-mode.el) to
;; any directory in your Emacs load-path.

;; Customization:
;;  To your .emacs or site-start.el add:
;;  (autoload 'pound-mode "pound-mode")
;;  (add-to-list 'auto-mode-alist
;;               (cons (purecopy "pound\\.cfg\\'")  'pound-mode))

(require 'font-lock)

(defgroup pound nil
  "Variables controlling pound-mode.
"
  :group 'unix)

(defgroup pound nil
  "Pound configuration mode"
  :group 'pound
  :prefix "pound-")

(defgroup pound-indentation nil
  "Variables controlling indentation in pound-mode.
"
  :group 'pound)

(defcustom pound-basic-offset 8
  "*The default indentation increment"
  :type 'integer
  :group 'pound)

(defgroup pound-lint nil
  "Variables controlling pound `lint' mode.
"
  :group 'pound)

(defcustom pound-command "pound"
  "*The default pound command line"
  :type 'string
  :group 'pound-lint)

(defcustom pound-include-dir ""
  "*Include directory to use instead of the default"
  :type 'string
  :group 'pound-lint)

(defvar pound-named-section-keywords
  '("Backend"
    "Condition"
    "Service"
    "ListenHTTP"
    "ListenHTTPS"))

(defvar pound-section-keywords
  '("ACL"
    "ConfigText"
    "Control"
    "CombineHeaders"
    "Else"
    "Lua"
    "Match"
    "Resolver"
    "Rewrite"
    "Session"
    "TrustedIP"
    "End"))

(defvar pound-deprecated-statements
  '("HeadRequire"
    "HeadDeny"
    "IgnoreCase"
    "HeaderAdd"
    "AddHeader"
    "HeaderRemove"
    "HeadRemove"
    "Err400"
    "Err401"
    "Err403"
    "Err404"
    "Err413"
    "Err414"
    "Err500"
    "Err501"
    "Err503"))

(defvar pound-matcher-keywords-1
  '("ACL"
    "BasicAuth"
    "CheckURL"
    "Host"
    "Path"
    "Query"
    "TBF"
    "URL"))

(defvar pound-matcher-keywords-2
  '("Eval"
    "Header"
    "LuaMatch"
    "StringMatch"
    "QueryParam"))

(defvar pound-statement-keywords
  '("ACME"
    "Address"
    "Alive"
    "Anonymise"
    "Anonymize"
    "BackendStats"
    "Balancer"
    "CAlist"
    "CNAMEChain"
    "CPath"
    "CRLlist"
    "Cert"
    "ChangeOwner"
    "Ciphers"
    "Client"
    "ClientCert"
    "ConfigFile"
    "ConfigText"
    "ConnTO"
    "Control"
    "Daemon"
    "Debug"
    "DeleteHeader"
    "DeleteQuery"
    "Disable"
    "Disabled"
    "ECDHCurve"
    "Emergency"
    "Error"
    "ErrorFile"
    "Family"
    "ForwardedHeader"
    "Grace"
    "Group"
    "HTTPS"
    "HeaderOption"
    "ID"
    "IgnoreSRVWeight"
    "Include"
    "IncludeDir"
    "LineBufferSize"
    "Load"
    "LoadGlobal"
    "LogFacility"
    "LogFormat"
    "LogLevel"
    "LogSuppress"
    "LogTag"
    "LuaModify"
    "LuaBackend"
    "MaxRequest"
    "MaxURI"
    "Metrics"
    "Mode"
    "NoHTTPS11"
    "OverrideTTL"
    "Path"
    "PidFile"
    "Port"
    "Priority"
    "Redirect"
    "RegexType"
    "Resolve"
    "RetryInterval"
    "RewriteDestination"
    "RewriteErrors"
    "RewriteLocation"
    "RootJail"
    "SSLAllowClientRenegotiation"
    "SSLEngine"
    "SSLHonorCipherOrder"
    "SendFile"
    "ServerName"
    "SetHeader"
    "SetPath"
    "SetQuery"
    "SetQueryParam"
    "SetURL"
    "Socket"
    "SocketFrom"
    "StringMatch"
    "Supervisor"
    "TTL"
    "Threads"
    "TimeOut"
    "TrustedIP"
    "Type"
    "UseBackend"
    "User"
    "VerifyList"
    "WSTimeOut"
    "WSTimeOut"
    "WatcherTTL"
    "WorkerIdleTimeout"
    "WorkerMaxCount"
    "WorkerMinCount"
    "xHTTP"
))

(defvar pound-match-flags
  '("-file"
    "-filewatch"
    "-forwarded"
    "-re"
    "-exact"
    "-beg"
    "-end"
    "-contain"
    "-icase"
    "-case"
    "-posix"
    "-pcre"
    "-perl"
    "-tag"))

(defconst pound-boolean
  '("yes"
    "true"
    "on"
    "no"
    "false"
    "off"))

(defconst pound-log-facilities
  '("auth"
    "authpriv"
    "cron"
    "daemon"
    "ftp"
    "kern"
    "lpr"
    "mail"
    "news"
    "syslog"
    "user"
    "uucp"
    "local0"
    "local1"
    "local2"
    "local3"
    "local4"
    "local5"
    "local6"
    "local7"))

(defvar pound-statements
  (append
   pound-matcher-keywords-1
   pound-matcher-keywords-2
   pound-statement-keywords
   pound-deprecated-statements))

(defconst pound-font-lock-keywords
  (list
   ;; Boolean values
   (list
    (concat "^[ \t]*" (regexp-opt pound-statements))
    (regexp-opt pound-boolean 'words)
    nil nil '(0 font-lock-constant-face))
   ;; Numeric constants
   (list
    (concat "^[ \t]*" (regexp-opt pound-statements))
    "\\<[0-9]+\\>"
    nil nil '(0 font-lock-constant-face))
   ;; IPv4 addresses
   (list
    (concat "^[ \t]*" (regexp-opt pound-statements))
    "\\<[0-9]\\{1,3\\}\\(?:\\.[0-9]\\{1,3\\}\\)\\{3\\}\\>"
    nil nil '(0 font-lock-constant-face))
   ;; IPv6 addresses
   (list
    (concat "^[ \t]*" (regexp-opt pound-statements))
    "\\<[0-9a-fA-F]\\{1,4\\}\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{7\\}\\>"
    nil nil '(0 font-lock-constant-face))
   (list
    (concat "^[ \t]*" (regexp-opt pound-statements))
    "\\<\\(?:[0-9a-fA-F]\\{1,4\\}\\(?::[0-9a-fA-F]\\{1,4\\}\\)?:\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{5\\}\\)\\|\\(?:[0-9a-fA-F]\\{1,4\\}\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{0,2\\}:\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{4\\}\\)\\|\\(?:[0-9a-fA-F]\\{1,4\\}\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{0,3\\}:\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{3\\}\\)\\|\\(?:[0-9a-fA-F]\\{1,4\\}\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{0,4\\}:\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{2\\}\\)\\|\\(?:[0-9a-fA-F]\\{1,4\\}\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{0,5\\}::[0-9a-fA-F]\\{1,4\\}\\)\\|\\(?::\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{1,7\\}\\)\\|\\(?:[0-9a-fA-F]\\{1,4\\}\\(?::[0-9a-fA-F]\\{1,4\\}\\)\\{7\\}\\)\\>"
    nil nil '(0 font-lock-constant-face))

   ;; Deprecated statements
   (list (concat "^[ \t]*"
		  (regexp-opt pound-deprecated-statements 1)
		  "[ \t]+\\([^\000- ]+\\)")
	  '(1 font-lock-warning-face)
	  '(2 font-lock-variable-name-face))
   ;; Matching conditions
   (list (concat "^[ \t]*"
		 "\\(\\(?:not[ \t]+\\)*\\)"
		 (regexp-opt pound-matcher-keywords-2 1)
		 "[ \t]+"
		 "\\(\"[^\"]*\"\\)"
		 "\\(\\(?:[ \t]+" (regexp-opt pound-match-flags) "\\)*\\)"
		 "[ \t]+"
		 "\\(\"[^\"]*\"\\)")
	 '(1 font-lock-keyword-face)
	 '(2 font-lock-keyword-face)
	 '(3 font-lock-string-face)
	 '(4 font-lock-builtin-face)
	 '(5 font-lock-string-face))
   (list (concat "^[ \t]*"
		 "\\(\\(?:not[ \t]+\\)*\\)"
		 (regexp-opt
		  (append pound-matcher-keywords-1 pound-matcher-keywords-2)
		  1)
		 "\\(\\(?:[ \t]+" (regexp-opt pound-match-flags) "\\)*\\)"
		 "[ \t]+")
	 '(1 font-lock-keyword-face)
	 '(2 font-lock-keyword-face)
	 '(3 font-lock-builtin-face))

   ;; Keywords
   '("\\<Family\\>"
     "\\<any\\|unix\\|inet\\|inet6\\>"
     nil nil (0 font-lock-constant-face))

   '("\\<Disable\\>"
     "\\<SSLv2\\|SSLv3\\|TLSv1_1\\|TLSv1_2\\|TLSv1\\>"
     nil nil (0 font-lock-constant-face))

   '("\\<Resolve\\>"
     "\\<immediate\\|first\\|all\\|srv\\>"
     nil nil (0 font-lock-constant-face))

   '("\\<Type\\>"
     "\\<IP\\|COOKIE\\|URL\\|PARM\\|BASIC\\|HEADER\\>"
     nil nil (0 font-lock-constant-face))

   '("\\<LogSuppress\\>"
     "\\<all\\|info\\|success\\|redirect\\|clterr\\|srverr\\>"
     nil nil (0 font-lock-constant-face))

   '("\\<HeaderOptions\\>"
     "\\<forwarded\\|ssl\\|all\\>"
     nil nil (0 font-lock-constant-face))

   '("\\<RegexType\\>"
     "\\<posix\\|pcre\\|perl\\>"
     nil nil (0 font-lock-constant-face))

   (list "\\<LogFacility\\>"
	 (regexp-opt pound-log-facilities 'words)
	 nil nil '(0 font-lock-constant-face))

   ;; Reqular keywords
   (list (concat "^[ \t]*"
		  (regexp-opt pound-statement-keywords 1)
		  "[ \t]+\\([^\000- ]+\\)")
	  '(1 font-lock-keyword-face)
	  '(2 font-lock-variable-name-face))
   (list (concat "^[ \t]*"
		  (regexp-opt pound-statement-keywords 1)
		  "[ \t]*\\(?:#.*\\)?$")
	  '(1 font-lock-keyword-face))

   ;; Named sections
   (list (concat "^[ \t]*"
		  (regexp-opt pound-named-section-keywords 1)
		  "[ \t]+\\(\".+\"\\)")
	  '(1 font-lock-keyword-face)
	  '(2 font-lock-string-face))
   ;; Special sections
   (list "^[ \t]*\\(Rewrite\\)[ \t]+\\(request\\|response\\)\\>"
	 '(1 font-lock-keyword-face)
	 '(2 font-lock-constant-face))
   (list "^[ \t]*\\(\\(?:not[ \t]+\\)*Match\\)\\([ \t]+\\(and\\|or\\)\\)?\\>"
	 '(1 font-lock-keyword-face)
	 '(2 font-lock-constant-face))

   ;; Sections
   (list (concat "^[ \t]*"
		 (regexp-opt
		  (append
		   pound-section-keywords
		   pound-named-section-keywords)
		  1)
		 "[ \t]*\\(?:#.*\\)?$")
	 '(1 font-lock-keyword-face)) ))

(defvar pound-true-sections
  '("Backend"
    "Service"
    "Condition"
    "CombineHeaders"
    "Resolver"
    "Else"
    "Rewrite"
    "Session"))

(defvar pound-maybe-sections
  '("TrustedIP"
    "ACL"))

(defvar pound-section-delim
  (concat "^[ \t]*"
	  "\\("
	  (regexp-opt (append pound-true-sections (list "End")) 'words)
	  "\\|\\(?:"
	  "\\(?:"
	    (regexp-opt pound-maybe-sections)
	    ;; Technically, the following is not quite correct, as it is
	    ;; only ACL that takes the "forwarded" option, but it will do
	    ;; no harm to assume that for TrustedIP as well.
	    "\\(?:[ \t]*-forwarded\\)?\\)"
	  "\\|"
	  "\\(?:\\(?:not[ \t]+\\)*Match\\(?:[ \t]+\\(?:and\\|or\\)\\)?\\)"
	  "\\)"
	  "[ \t]*\\(?:#.*\\)?$\\)"))

(defun pound-streq (a b)
  "Case-insensitive string equality"
  (eq t (compare-strings a 0 nil b 0 nil t)))

(defun pound-scan (limit stk)
  (catch 'ret
    (forward-line)
    (while (and (< (point) limit)
		(re-search-forward pound-section-delim limit t))
      (cond
       ((pound-streq (match-string 1) "end")
	(setq stk (cdr stk))
	(if (not stk) (throw 'ret stk)))
       ((and (looking-at "^[ \t]*Control[ \t]*\\(?:#.*\\)?$")
	     (pound-at-control-backend)))
       (t
	(setq stk (cons (marker-position (nth 2 (match-data))) stk))))
      (forward-line))
    stk))

(defun pound-at-control-backend ()
  "Return true if the statement at point is a Control backend keyword within
a Service, as opposed to the top-level Control (whether block or directive)"
  (save-excursion
    (catch 'ret
      (beginning-of-line)
      (while (not (bobp))
	(forward-line -1)
	(cond
	 ((looking-at "^[ \t]*Service")
	  (throw 'ret t))
	 ((looking-at "^[ \t]*End\\>")
	  (throw 'ret nil)))))))

(defun pound-top-level-form ()
  "Move point to the beginning of the nearest top-level block statement.
Return t if such was found, nil otherwise."
  (if (re-search-backward "^[ \t]*\\(ListenHTTPS?\\(?:[ \t]\".*\"\\)?\\|\\(Condition[ \t]+\".*\"\\(?:[ \t]+\\(?:and\\|or\\)\\)?\\)\\|Control\\|Resolver\\|CombineHeaders\\|Lua\\)[ \t]*\\(?:#.*\\)?$" nil t)
      (cond
       ((and (pound-streq (match-string 1) "Control")
	     (pound-at-control-backend))
	(pound-top-level-form))
       (t
	t))))

(defun pound-get-context ()
  (save-excursion
    (let* ((start (point))
	   (ctx (if (pound-top-level-form)
		    (pound-scan start
				(list (marker-position
				       (nth 2 (match-data))))))))
      (if ctx
	  ctx
	(goto-char start)
	(if (and
	     (re-search-backward "^[ \t]*\\(\\(?:\\(?:ACL\\|Backend\\)[ \t]+\".*\"\\)\\|\\(?:TrustedIP[ \t]*\\(\\(?:[ \t]\".*\"\\)\\|\\(?:\\(?:#.*\\)?$\\)\\)\\)\\|Service\\>\\)" nil t)
	     (not (looking-at "^[ \t]*\\(?:\\(?:\\(?:ACL[ \t]+\".*\"\\)\\|TrustedIP\\)[ \t]+-file\\(?:watch\\)?\\)")))
	    (let ((pos (marker-position (nth 2 (match-data)))))
	      (pound-scan start (list pos))))))))

(defun pound-indent-level ()
  (save-excursion
    (beginning-of-line)
    (let ((atend (looking-at "^[ \t]*\\(End\\|Else\\)\\>"))
	  (level (length (pound-get-context))))
      (if (and (> level 0) atend) (1- level) level))))

(defun pound-indent-line ()
  "Indent the current line."
  (interactive "*")
  (let ((start-of-line (save-excursion
			 (beginning-of-line)
			 (skip-chars-forward " \t")
			 (point)))
	(shift-amount (* (pound-indent-level) pound-basic-offset)))
    (if (not (= shift-amount (current-indentation)))
	(let ((off (- (point) start-of-line)))
	  (beginning-of-line)
	  (delete-region (point) start-of-line)
	  (indent-to shift-amount)
	  (if (>= off 0)
	      (goto-char (+ (point) off))
	    (beginning-of-line))))))

(defun pound-beginning-of-defun ()
  "Interface to `beginning-of-defun'"
  (let ((ctx (pound-get-context)))
    (if ctx
	(goto-char (car ctx)))))

(defun pound-end-of-defun ()
  "Interface to `end-of-defun'"
  (let ((n 1))
    (while (and (> n 0) (re-search-forward pound-section-delim nil t))
      (cond
       ((pound-streq (match-string 1) "end")
	(setq n (1- n)))
       (t
	(setq n (1+ n)))))
    (beginning-of-line)
    (skip-chars-forward " \t")))

(defun pound-lint ()
  "Check the syntax of the current pound buffer."
  (interactive "*")
  (compile (concat
	    pound-command
	    (cond
	     ((string= pound-include-dir "") "")
	     ((string= pound-include-dir ".") " -Wno-include-dir")
	     (t
	      (concat " -Winclude-dir=" pound-include-dir)))
	    " -c -f "
	    (buffer-file-name))))

(defvar pound-mode-syntax-table
  (let ((syntab (make-syntax-table)))
    (modify-syntax-entry ?_ "\w" syntab)
    (modify-syntax-entry ?: "\w" syntab)
    (modify-syntax-entry ?. "\w" syntab)
    (modify-syntax-entry ?\" "\"" syntab)
    (modify-syntax-entry ?\# "<" syntab)
    (modify-syntax-entry ?\n ">" syntab)
    (modify-syntax-entry ?\r ">" syntab)
    (modify-syntax-entry ?\t "-" syntab)
    syntab))

(defvar pound-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\t" 'pound-indent-line)
    (define-key map "\C-c\C-c" 'pound-lint)
    (define-key map "\e\C-\\" 'indent-region)
    map))

;;;###autoload
(defun pound-mode ()
  "Major mode for editing pound configuration files.

Use \\[beginning-of-defun] to find nearest enclosing
block start and \\[end-of-defun] to find its end.

Other key bindings are:
\\{pound-mode-map}
"
  (interactive)
  (kill-all-local-variables)
  (use-local-map pound-mode-map)
  (set-syntax-table pound-mode-syntax-table)

  (setq major-mode 'pound-mode
	mode-name "Pound")

  (setq-local font-lock-defaults
	'((pound-font-lock-keywords) nil t nil nil))
  (setq-local case-fold-search t)
  (setq-local indent-line-function 'pound-indent-line)
  (setq-local comment-start "# ")
  (setq-local comment-start-skip "#+[\t ]*")
  (setq-local beginning-of-defun-function 'pound-beginning-of-defun
	      end-of-defun-function 'pound-end-of-defun))

(provide 'pound-mode)
;;; pound-mode ends
