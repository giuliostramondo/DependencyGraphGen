(TeX-add-style-hook
 "dsd20_main"
 (lambda ()
   (TeX-add-to-alist 'LaTeX-provided-class-options
                     '(("IEEEtran" "conference")))
   (TeX-add-to-alist 'LaTeX-provided-package-options
                     '(("adjustbox" "export")))
   (add-to-list 'LaTeX-verbatim-environments-local "lstlisting")
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "lstinline")
   (add-to-list 'LaTeX-verbatim-macros-with-delims-local "lstinline")
   (TeX-run-style-hooks
    "latex2e"
    "./sections/dsd20_abstract"
    "./sections/dsd20_introduction"
    "./sections/dsd20_background"
    "./sections/dsd20_framework"
    "./sections/dsd20_architecture_template"
    "./sections/dsd20_experiments"
    "./sections/dsd20_related_work"
    "./sections/dsd20_conclusion"
    "IEEEtran"
    "IEEEtran10"
    "cite"
    "amsmath"
    "amssymb"
    "amsfonts"
    "algorithmic"
    "graphicx"
    "textcomp"
    "xcolor"
    "listings"
    "tikz"
    "tikzscale"
    "placeins"
    "adjustbox"
    "subcaption"
    "setspace"
    "pdfsync")
   (TeX-add-symbols
    "squeezeup"
    "BibTeX")
   (LaTeX-add-bibliographies
    "dsd20_biblio")
   (LaTeX-add-xcolor-definecolors
    "codegreen"
    "codegray"
    "codepurple"
    "backcolour"
    "punct"
    "background"
    "delim"
    "numb")
   (LaTeX-add-listings-lstdefinestyles
    "mystyle"))
 :latex)

