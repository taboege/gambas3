# gb.test – A Gambas Unittest

A Gambas component for unittesting and test-driven programming. Inspired from quite an old program: [COMUnit](http://comunit.sourceforge.net) and other test frameworks. With this component you can develop software in a test-driven matter (write test first, program functionality afterwards) and you are able to ensure that on refactoring the desired results of your code stays the same.

Tests are output as [Tap](https://testanything.org/ testanything.org) so that they can be displayed, analyzed or viewed with any [Tap consumer](https://testanything.org/consumers.html).

## How it works

There is an example in [this simple Gambas project](unittesthelloworld-0.0.8.tar.gz).

### Example TestContainer

Start by creating a TestContainer, this is a module with any name, but the ending ".test", for example "TestHelloWorld.test". This class contains one or more public testmethod(s).

----
    ' Gambas test module file
    ''' TestContainer TestHelloWorld

    Public Sub TestHelloWorld()

        Assert.EqualsString("Hello World", Hello.World(), "Strings should be equal")

    End
----

### Module(Function) to test:

To make it work, we need a funktion that will be tested. So we create a function "World" in a module "Hello" in our project:

----

    ' Gambas module file

    ''' Module is named "Hello"

    Public Function World() As String

      Return "Hello World"

    End

----

###  Bringing Test into play

A simple way to execute the Unittest is to create another module, name it "TestMe" or something more interesting and make it a Gambas startclass:

----

    'Module TestMe

    'Is a startclass
    'Starts the Unittest, when F5 was hit

    Public Sub Main()

        Test.Main()

    End

----

If you did all this correctly and now hit &lt;F5&gt;, Gambas will execute the startfunction in module TestMe, which works through the method(s) of our TestContainer and presents the test result in the console, for example (this is from unittesthelloworld):

    1..3
    
    TestCase TestHello:TestFortyTwo
      ok 1 - Test Hello.FortyTwo
      1..1
    ok 1 - TestHello:TestFortyTwo
    
    TestCase TestHello:TestHelloWorld
      ok 1 - Test equal strings just for fun
      ok 2 - HW strings should be equal
      1..2
    ok 2 - TestHello:TestHelloWorld
    
    TestCase TestHello:TestRight
      ok 1 - Test Hello.Right says Right
      1..1
    ok 3 - TestHello:TestRight
    
    # Ran: '' 
    #
    # PASSED
    
If a failure occurs it will report FAILED instead of PASSED and will show you the place of the failure. I you want to debug the code you can set a breakpoint inside TestHello.TestRight, hit &lt;F5&gt; again and start debugging.

If you have a lot of tests, and want to let run just one, you can do that like so:

----

    'Module TestMe

    'Is a startclass
    'Starts the Unittest, when F5 was hit

    Public Sub Main()

        Test.Main("TestHello.TestRight")

    End

----

## Test your project on the console

If you made an executable of your project, you can even test it on the console. The command **/usr/bin/gbt3 /path/to/my/project** executes the unittests and prints the result to standard output:

christof@chrisvirt ~/temp/unittesthelloworld » /usr/bin/gbt3.gambas ~/temp/unittesthelloworld 

    1..3
    
    
    TestCase TestHello:TestFortyTwo
      ok 1 - Test Hello.FortyTwo
      1..1
    ok 1 - TestHello:TestFortyTwo
    
    
    TestCase TestHello:TestHelloWorld
      ok 1 - Test equal strings just for fun
      ok 2 - HW strings should be equal
      1..2
    ok 2 - TestHello:TestHelloWorld
    
    
    TestCase TestHello:TestRight
      ok 1 - Test Hello.Right says Right
      1..1
    ok 3 - TestHello:TestRight
    
    
    # Ran: '' 
    #
    # PASSED
    
    
## Test fixture

Sometimes it is neccessary to create a "fixture", a special environment for a test or a couple of tests, and to destroy this environment after the test is done. For example a database connection should be established, some tables for testing should be created and this has to be reverted afterwards. This can be done with Setup... and Teardown... functions inside the TestContainer.

### Sub SetupEach() and Sub TeardownEach()

You can create methods with these names to create an environment for each testmethod before it is invoked and to destroy it afterwards. If you have five testmethods inside your TestContainer these functions will be invoked five times, SetupEach() before each testmethod, TeardownEach() after each testmethod. Got it?

### Sub Setup() and Sub Teardown()

You can create methods with these names to create an environment for all testmethods inside a TestContainer, in the beginning Setup() is invoked and after all testmethods inside the testclass are done you can destroy the environment with Teardown().