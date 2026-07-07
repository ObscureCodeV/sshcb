#EXAMPLE

"SSHCB intergration example
let g:clipboard = '/root/sshcb/build/main/app'

function! SendText(reg, channel) range
  let l:text = getreg(a:reg)
  if l:text == ""
    let l:text = ExtractText()
  endif

  let l:text = substitute(l:text, '\\r', '', 'g')
  let l:text = substitute(l:text, '\\n\\+$', '', '')

  let l:escaped_text = shellescape(l:text, 1)
  let l:cmd = g:clipboard . ' --send ' . l:escaped_text . ' --channel ' . a:channel

  call ExecCmd(l:cmd)

endfunction

function! ExtractText() range
  let l:vmode = visualmode()
  if l:vmode != ""
    let l:lines = getregion("'<", "'>", {'type': l:vmode})
    let l:text = join(l:lines, "\n")
    normal! gv
  else
    "for command mode (:10,20ClipSend)
    let l:text = join(getline(a:firstline, a:lastline), "\n")
  endif
  return l:text
endfunction

function! ReadText(reg, channel)
  let l:cmd = g:clipboard . ' --read --channel ' . a:channel
  let l:text = ExecCmd(l:cmd)

  let l:text = substitute(l:text, '\\r', '', 'g')
  let l:text = substitute(l:text, '\\n\\+$', '', '')

  if l:text != ""
    call setreg(a:reg, l:text)
  endif
endfunction

function! ExecCmd(cmd)
  let l:full_cmd = a:cmd . ' 2>&1'
  let l:output = trim(system(l:full_cmd))

  if v:shell_error != 0
    echohl ErrorMsg | echo l:output | echohl None
    return ""
  endif

  return l:output
endfunction

let mapleader = " "

nnoremap <silent> <Leader>sq :call SendText('q', 0)<CR>
nnoremap <silent> <Leader>sw :call SendText('w', 1)<CR>
nnoremap <silent> <Leader>se :call SendText('e', 2)<CR>
nnoremap <silent> <Leader>sr :call SendText('r', 3)<CR>
nnoremap <silent> <Leader>st :call SendText('t', 4)<CR>

xnoremap <silent> <Leader>sq :call SendText('q', 0)<CR>
xnoremap <silent> <Leader>sw :call SendText('w', 1)<CR>
xnoremap <silent> <Leader>se :call SendText('e', 2)<CR>
xnoremap <silent> <Leader>sr :call SendText('r', 3)<CR>
xnoremap <silent> <Leader>st :call SendText('t', 4)<CR>

nnoremap <silent> <Leader>dq :call ReadText('q', 0)<CR>
nnoremap <silent> <Leader>dw :call ReadText('w', 1)<CR>
nnoremap <silent> <Leader>de :call ReadText('e', 2)<CR>
nnoremap <silent> <Leader>dr :call ReadText('r', 3)<CR>
nnoremap <silent> <Leader>dt :call ReadText('t', 4)<CR>

xnoremap <silent> <Leader>dq :call ReadText('q', 0)<CR>
xnoremap <silent> <Leader>dw :call ReadText('w', 1)<CR>
xnoremap <silent> <Leader>de :call ReadText('e', 2)<CR>
xnoremap <silent> <Leader>dr :call ReadText('r', 3)<CR>
xnoremap <silent> <Leader>dt :call ReadText('t', 4)<CR>
