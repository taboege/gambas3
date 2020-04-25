## To Do

- Test.Main has to complain if testmodule doesn't exist
- Test.Main options
    > Test.Main needs a couple of options
    * [ ] Sparse to switch off TAP output of subtests
    * [ ] NoSummary to switch off summary
    * [ ] Silent to switch off TAP output to console
- Ensure TAP created according to  spec
- Install gbt3 at Gambas installation
- gbt3 has to return exit status at the end
- Assert documentation

## Work

- Summary as a TAP comment
    > Description in .hidden/summary-example-txt

## Done

- Subtests don't count the right way
    > If there is only one testmodule called, subtest counts a lot of subtests instead just one
- gb.test doesn't print to console
