;; see http://www.emacswiki.org/cgi-bin/wiki/GenericMode

;; ;IFM mode
;; (autoload 'ifm-generic-mode "ifm-generic-mode")
;; (setq auto-mode-alist (cons '("\\.ifm$" . ifm-generic-mode) auto-mode-alist))



(define-generic-mode
  ;; NAME
  'ifm-generic-mode
  ;; COMMENT-LIST
  '("#")
  ;; KEYWORD-LIST
  '("%include" "%define" "%defeval" "%if" "%ifdef" "%undef"
    "%ifndef" "%ifeq" "%ifneq" "%else" "%endif" "%eval"
    "%1" "%2" "%3" "%4" "%5" "%6" "%7" "%8" "%9" "%exec" "defined"
    "room" "link"
    "n" "north" "ne" "northeast" "e" "east" "se" "southeast" "s" "south" "sw"
    "southwest" "w" "west" "nw" "northwest"
    "endstyle" "style" "title" "map" "start" "finish" "safe" "require"
    "all" "any" "it" "last" "none" "undef"
    "after" "before" "cmd" "d" "do" "down" "dir" "drop" "except" "exit" "follow"
    "from" "get" "give" "go" "goto" "hidden" "ignore" "in" "item" "join" "keep"
    "leave" "length" "lose" "lost" "need" "nodrop" "nolink" "nopath" "note"
    "oneway" "out" "score" "tag" "task" "to" "u" "up" "until" "with")
  ;; FONT-LOCK-LIST
  nil
  ;; AUTO-MODE-LIST
  '("\\.ifm\\'")
  ;; FUNCTION-LIST
  (list (lambda ()
          (defun ifm-insert-room ()
            "Defines a room and gives it a tag"
            (interactive)
            (setq ROOM (read-string "ROOM: "))
            (insert
             "room \"" ROOM "\" tag r"
             (replace-regexp-in-string
              " " "" (upcase-initials ROOM)) " "))
          (defun ifm-insert-item ()
            "Defines an item and gives it a tag"
            (interactive)
            (setq ITEM (read-string "ITEM: "))
            (insert
             "item \"" ITEM "\" tag i"
             (replace-regexp-in-string
              " " "" (upcase-initials ITEM)) " "))
          (defun ifm-insert-task ()
            "Defines a task and gives it a tag"
            (interactive)
            (setq TASK (read-string "TASK: "))
            (insert
             "task \"" TASK "\" tag t"
             (replace-regexp-in-string
              " " "" (upcase-initials TASK)) " "))
          (local-set-key "\C-cr" 'ifm-insert-room)
          (local-set-key "\C-ci" 'ifm-insert-item)
          (local-set-key "\C-ct" 'ifm-insert-task)
          ))

  ;; DESCRIPTION
  "Major mode for editing IFM files.
Useful keys:
   C-c r : `ifm-insert-room'
   C-c i : `ifm-insert-item'
   C-c t : `ifm-insert-task'")


