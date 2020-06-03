## To Do

- Write Documentation
    * [ ] Test.Main overall doc what is does and how tests are run.

## Work

- Plan
    > If a plan failes it has to be reported in the TAP output as well as in the summary which is not the case at the moment. There is no plan for testmodules which counts the testmethods.
    * [ ] Plan for testmodules (as a constant?)
    * [ ] Plan failure must be reported in TAP output and in summary (there has to be reported where it fails).
- Plan selftests
    > gb.test: Every subtest in all tests must plan the own assertion count

## Done

- Event handlers are not test methods
    > Some test modules test object which emit events, but the event handlers are Public Subs which get picked up as test methods by gb.test. What can we do about it?
- TestAllAssertion must test all assertions
- Decide where Skip, SkipAll, Todo reside
    > SkipAll is in Test, Skip in Assert, Todo in Assert. This is inconsistent, We have to decide where they should be. Chris: I am for Test.
- Install gbt3 at Gambas installation
    > gbt3 is compiled and installed by make but there is no symlink gbt3 > gbt3.gambas
- Ensure TAP created according to  spec
    > Tested TAP via gbt3 and python tappy
- gbt3 has to return exit status at the end
- Assert and Test documentation
    > Every public method in Test and Assert has to have a description
- Hide symbols in Test
    > There are symbols in Test which should not be shown to the enduser. These shoult be hidden by a leading underscore:
    * [ ] Subtest
    * [ ] Finish
    * [ ] Reset
- No Bug: Directives always 0
    > Was no bug, was lack of understanding
- TestAssertion: Skip and Todo
    > TestAssertion must contain variables to store Skip and Todo for summary. See TestSummary.
- Todo Directive
    > Todo directive does not exist. See TestSummary. Was wrong. Todo exists.
- Free Assert from non-assertions
    > There are things in Assert which are no assertions, like Print, Finish, Note and so on. These should be methods of Test. For example: Test.Plan, Test.Note, Test.Finish ...
- Test.Main has to complain if testmodule doesn't exist
- Human readability of TAP output
- Subtests don't count the right way
    > If there is only one testmodule called, subtest counts a lot of subtests instead just one
- gb.test doesn't print to console
- Summary as a TAP comment
    > Description in .hidden/summary-example-txt
    * [x] New method Test.ShowTodos
    * [x] New method Test.ShowSkips
- Parser parse subtests
- Skip all
    > Parse `1..0 # SKIP` style TAPs indicating that a whole test was skipped.
- Document Approximate and RelativeApproximate
