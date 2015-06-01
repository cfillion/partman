# PartMAN Lilypond Generator

This is a work in progress.

I've written this tool to help me write big orchestral scores using lilypond by
generating the repetitive code I use to define and add parts. It is still
imperfect, but it does its job.

Example input file (YAML for the moment):

```yaml
header:
  # tagline is set to false by default
  title: "Lorem Ipsum"
  composer: "John Doe"
paper:
  # default paper-size is "letter"
  top-margin: 0.7\cm
  ragged-last-bottom: false
setup:
  # this is prepended to all parts
  key: c \major
  time: 4/4
  tempo: 4 = 120
parts:
  violin:
    name: ["Violin", "Vln."]
    instrument: "violin" # midi instrument
    relative: c''
  piano:
    name: ["Piano", "Pno."]
    type: PianoStaff
    instrument: "acoustic grand"
    parts:
      upper:
        relative: c''
      lower:
        relative: c,
score:
  parts: [violin, piano]
```

PartMAN will generate the lilypond code required to build this score with
`\include` statements pointing to these files:

- parts/violin.ily
- parts/piano_upper.ily
- parts/piano_lower.ily

TODO:

- [X] Organize parts (order/`StaffGroup`) differently in each score blocks
- [X] `\book` support
- [X] Prevent lilypond from wasting midi channels
- [X] Find a good unit test framework and use it
    ([Catch](https://github.com/philsquared/Catch)?
    [Boost.Test](http://www.boost.org/libs/test)?)
- [ ] Drums support
- [ ] Dynamics support
- [ ] Use an input langage with a syntax closer to lilypond's
- [ ] Better error handling and reporting
- [ ] Call lilypond automatically
