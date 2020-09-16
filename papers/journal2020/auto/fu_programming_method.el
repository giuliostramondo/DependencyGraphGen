(TeX-add-style-hook
 "fu_programming_method"
 (lambda ()
   (add-to-list 'LaTeX-verbatim-environments-local "lstlisting")
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "lstinline")
   (add-to-list 'LaTeX-verbatim-macros-with-delims-local "lstinline")
   (TeX-add-symbols
    '("rowgroup" 1))
   (LaTeX-add-labels
    "chap:design"
    "sec:requirements"
    "sec:assembly"
    "tab:inputs"
    "tab:insts"
    "tab:ATA-MC"
    "tab:mux"
    "tab:reg"
    "sec:overview"
    "fig:framework"))
 :latex)

