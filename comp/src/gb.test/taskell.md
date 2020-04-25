## To Do

- Test.Main has to complain if testmodule doesn't exist
- Free Assert from non-asserts
    > There are things in Assert which are no assertions, like Print, Finish, Note and so on. These should be methods of Test. For example: Test.Plan, Test.Note, Test.Finish ...
- Assert documentation
- Ensure TAP created according to  spec
- Parser parse subtests
- Skip all
- Install gbt3 at Gambas installation
- gbt3 has to return exit status at the end

## Work

- Summary as a TAP comment
    > Description in .hidden/summary-example-txt

## Done

- Subtests don't count the right way
    > If there is only one testmodule called, subtest counts a lot of subtests instead just one
- gb.test doesn't print to console
