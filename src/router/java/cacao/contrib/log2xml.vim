" log2xml.vim - vim 6.1+ settings for browsing ouput from log2xml

" Prerequisites:
"     vim 6.1 or newer
"     matchit.vim (recommended for jumping between <call> and </call> with %)
"     (You can get both at www.vim.org.)
"
" Usage:
"     Before opening the .xml file type
"         :source log2xml.vim
"     You may want to add this to your .vimrc
"
"     You can use F11 to open/close a folded call.

let g:xml_syntax_folding=1

au FileType xml syntax on
au FileType xml set nowrap
au FileType xml let b:match_words="<call:</call"
au FileType xml set foldmethod=syntax
au FileType xml set foldnestmax=200

map <F11> za

hi Folded ctermfg=black ctermbg=lightgray guifg=blue guibg=yellow

