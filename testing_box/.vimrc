"return" 2>&- || "exit"

" Disable compatibility with vi which can cause unexpected issues.
set nocompatible
set nofixendofline

set bg=dark

" Automatically wrap text that extends beyond the screen length
set wrap

" Set syntax highlighting
syntax on

" Encoding
set encoding=utf-8

" Highlight cursor line underneath the cursor horizontally.
" set cursorline

" Disable swap file
set noswapfile

" While searching though a file incrementally highlight matching characters as you type.
set incsearch

" Enable auto completion menu after pressing TAB.
set wildmenu

filetype on
filetype plugin indent on

set autoindent
set autoread
set backspace=2
set encoding=utf-8
set expandtab
set ruler
set number
set list
set list listchars=tab:▸\ ,trail:·
set mouse=a
set tabstop=4
autocmd FileType javascript setlocal expandtab tabstop=2 shiftwidth=2
set autoindent

autocmd vimenter * ++nested colorscheme gruvbox

" PLUGINS ---------------------------------------------------------------- {{{

call plug#begin('~/.vim/plugged')

  Plug 'dense-analysis/ale'
  Plug 'preservim/nerdtree'
  Plug 'junegunn/fzf.vim'
  Plug 'vim-ruby/vim-ruby'
  Plug 'morhetz/gruvbox'
  Plug 'pangloss/vim-javascript'

call plug#end()

" }}}

" keyboard shortcuts
nnoremap <leader>n :NERDTreeFocus<CR>
nnoremap <C-n> :NERDTree<CR>
nnoremap <C-t> :NERDTreeToggle<CR>
nnoremap <C-f> :NERDTreeFind<CR>

nnoremap <C-q> :q<CR>
