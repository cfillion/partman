#(define __setup #{#})
#(define __parts (make-hash-table))

#(define (part-file name)
  (string-concatenate (list "parts/" (symbol->string name) ".ily"))
)

setup = #(define-void-function
  (parser location music)
  (ly:music?)

  (set! __setup music)
)


definePart = #(define-music-function
  (parser location name long_name short_name staff_type music)
  (symbol? markup? markup? symbol? ly:music?)

  (let ((this_part (make-hash-table)))
    (hashq-set! this_part 'long_name long_name)
    (hashq-set! this_part 'short_name short_name)
    (hashq-set! this_part 'staff_type staff_type)
    (hashq-set! this_part 'music music)

    (hashq-set! __parts name this_part)
  )

  #{#}
)

defineMusicPart = #(define-music-function
  (parser location name long_name short_name pitch)
  (symbol? markup? markup? ly:pitch?)

  #{
    \definePart #name #long_name #short_name #'Staff {
      \relative #pitch {
        \include #(part-file name)
      }
    }
  #}
)

definePianoPart = #(define-music-function
  (parser location name long_name short_name pitch_up pitch_down)
  (symbol? markup? markup? ly:pitch? ly:pitch?)

  #{
    \definePart #name #long_name #short_name #'PianoStaff <<
      \new Staff \with { \remove Staff_performer } \relative #pitch_up  {
        #__setup
        \include #(part-file (string->symbol (string-concatenate (list (symbol->string name) "_up"))))
      }
      \new Staff \with { \remove Staff_performer } \relative #pitch_down {
        #__setup
        \include #(part-file (string->symbol (string-concatenate (list (symbol->string name) "_down"))))
      }
    >>
  #}
)

defineDrumPart = #(define-music-function
  (parser location name long_name short_name)
  (symbol? markup? markup?)

  #{
    \definePart #name #long_name #short_name #'DrumStaff {
      \drummode {
        \include #(part-file name)
      }
    }
  #}
)

part = #(define-music-function
  (parser location name)
  (symbol?)

  (let ((this_part (hashq-ref __parts name)))
    #{
      \new #(hashq-ref this_part 'staff_type) {
        \set Staff.instrumentName = #(hashq-ref this_part 'long_name)
        \set Staff.shortInstrumentName = #(hashq-ref this_part 'short_name)

        #__setup
        #(hashq-ref this_part 'music)
      }
    #}
  )
)

pianoPart = #(define-music-function
  (parser location name)
  (symbol?)

  (let ((this_part (hashq-ref __parts name)))
    #{
      \new #(hashq-ref this_part 'staff_type) \with { \consists Staff_performer } {
        \set PianoStaff.instrumentName = #(hashq-ref this_part 'long_name)
        \set PianoStaff.shortInstrumentName = #(hashq-ref this_part 'short_name)

        #(hashq-ref this_part 'music)
      }
    #}
  )
)
