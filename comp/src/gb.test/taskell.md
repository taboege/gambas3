## To Do

- TestAssertion: Skip and Todo
    > TestAssertion must contain variables to store Skip and Todo for summary. See TestSummary
- Summary as a TAP comment
    > Description in .hidden/summary-example-txt
- Skip all
- Free Assert from non-assertions
    > There are things in Assert which are no assertions, like Print, Finish, Note and so on. These should be methods of Test. For example: Test.Plan, Test.Note, Test.Finish ...
- Ensure TAP created according to  spec
- Parser parse subtests
- Plan
    > Plan does not work. It should define the  count of assertions in a subtest and if  defined ensure that all are executed
    * [ ] Plan for subtests
    * [ ] Plan for testmodules (as a constant?)
- Plan selftests
    > gb.test: Every subtest in all tests must plan the own assertion count
- Assert and Test documentation
    > Every public method in Test and Assert has to have a description
- Install gbt3 at Gambas installation
- gbt3 has to return exit status at the end

## Work

- Todo Directive
    > Todo directive does not exist. See TestSummary

## Done

- Test.Main has to complain if testmodule doesn't exist
- Human readability of TAP output
- Subtests don't count the right way
    > If there is only one testmodule called, subtest counts a lot of subtests instead just one
- gb.test doesn't print to console
