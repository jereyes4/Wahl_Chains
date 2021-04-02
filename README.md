# Wahl_Chains

This is a program that given a configuration of rational curves on an elliptic surface, does successions of blow ups in hopes to obtain configurations of resolutions of Wahl singularities (Wahl chains).

Specifically, it blows up exhaustively to obtain all possible* configurations of:
- one Wahl chain,
- two Wahl chains,
- one <img src="https://render.githubusercontent.com/render/math?math=\mathbb Q \mathcal H \mathcal D^3"> plumbing graph,
- one <img src="https://render.githubusercontent.com/render/math?math=\mathbb Q \mathcal H \mathcal D^3"> plumbing graph and one Wahl chain.

An example is just a configuration of curves on a surface <img src="https://render.githubusercontent.com/render/math?math=X"> obtained after successive blowups and blowdowns starting from an elliptic surface <img src="https://render.githubusercontent.com/render/math?math=Y">. After contracting the curves in that configuration, a singular surface <img src="https://render.githubusercontent.com/render/math?math=S"> is obtained. Several invariants are obtained from <img src="https://render.githubusercontent.com/render/math?math=S">, such as <img src="https://render.githubusercontent.com/render/math?math=K_S^2">, global obstruction for smoothing the singularities, anr effectiveness and nef-ness of <img src="https://render.githubusercontent.com/render/math?math=K_S">.

The assumption of working on an elliptic surface is only used to calculate obstructions and nef-ness of the canonical class. As such, this program works in any abstract setting for the configuration of curves, with the disclaimer that both nef-ness and the obstruction calculation may not give expected or correct results.

# Usage

The program is written in standard `C++17`.  
The first step is to compile it. For example, using `GNU C++` with `-Ofast` and `-flto` optimizations, run from the terminal the line:

    g++ -std=c++17 -Ofast -flto src/*.cpp -o Search.exe

The program has support for multi-threading. To use it with `GNU C++`, run

    g++ -std=c++17 -Ofast -flto -pthread -D WAHL_MULTITHREAD src/*.cpp -o Search.exe

Alternatively, in the file `config.hpp` uncomment the line

    #define WAHL_MULTITHREAD

and run

    g++ -std=c++17 -Ofast -flto -pthread src/*.cpp -o Search.exe

The result will be an executable file called `Search.exe`.

To test a configuration, a test file must be provided. For example, if the test file is called, `test.txt`, one must run from the terminal the line

    Search.exe test.txt

or if using Unix-like

    ./Search.exe test.txt

If `test.txt` is a valid file, the testing will begin immediately. Afterwards, an example database file and a summary file will be created, depending on the settings provided in the test file.

The database file has extension `.jsonl` and is meant to have all the necessary information of the examples to reconstruct them. It is not meant to be human readable. The summary file has extension `.txt` or `.tex` and is meant to be readable. It lists some numerical invariants and general information of the examples, each with a unique ID.

To view the complete information of an example and how to recover it from the original configuration, the `Display.py` script is used. Suppose that the database is called `database.jsonl` and the ID of the example you want to examine is `6`. Then from the terminal run

    Display.py database.jsonl 6

or on Unix-like,

    ./Display.py database.jsonl 6

or alternatively

    python3 Display.py database.jsonl 6

A window should pop up containing the requested information.  
Of course, `python3` is required.

# Testing structure

A test file gives the program a list of rational curves on the surface and the intersections between them. To search for chains, the first step is to choose a subset of those curves, and consider only the sub-graph generated by this subset. This is called a `sub-test`.  
To efficiently describe thousands of `sub-tests`, the test file describes one or more `tests`: Within a `test`, each curve is either **fixed**, **ignored** or **tried**. **Fixed** curves shall be contained in every sub-test of a the given test. **Ignored** shall not be included in any sub-test. **Tried** curves are both included and ignored in different sub-tests, such that every possibility of including **try** curves is probed. Thus the amount of sub-tests inside a test is exponential in the amount of **try** curves.  
One more option for Fibers is provided. A **discard** fiber treats all its irreducible members as **try** curves, but excludes the case when all of them are included.

# Test file syntax

A test file consists of settings and curve definitions. All settings must be declared before starting the definitions of curves.

Single and multi-line comments are available loosely following `Python` and `C++` comment syntax.
- Single comments are done by **starting** the line with `#` or `//`
- Multiline comments begin with `/*` and end with `*/`. **Both symbols shall be at the start of the line**.

<b></b>

    # This is a valid comment
    // This is also a valid comment
    
    Something something // This is not a valid comment

    /* This is 
    a valid
    multiline comment
    */

    /*
    This one 
    is not */


## Settings
There are several settings that affect the search in different ways. All of them are of the form

    Setting: Option/value

Both settings and options are case-sensitive.  
Following is the full list of settings and their possible options:

- `Tests`: Takes a range of positive numbers, for example "`2 - 5`". It determines a range of tests to be performed on the configuration. The difference between these tests is determined by the options given later for each curve.  
  Can also take a single number `n`, which is effectively equivalent to the range "`1 - n`".  
  Values are clamped below the hard limit of `100`, given by the macro `MAX_TESTS` in `config.hpp`.
  Defaults to `1`.
- `Output`: Takes the name of the file where a database of examples will be exported. It will be of extension `.jsonl`. For example if one writes

      Output: filename

  A file called `filename.jsonl` will be created. Unfortunately, the filename shall not contain white spaces.  
  Defaults to `OUT`.
- `Summary_Style`: Determines the style of the summary file. Takes two possible options
  - `LaTeX_Table`: Summary is exported as an easy to include table in Latex format.
  - `Plain_Text`: Summary is exported as a plain text file.

  Defaults to `Plain_Text`.
- `Summary_Include_GCD`: For examples with two singularities, include in the summary the gcd of the common denominators for discrepancies. Useful when calculating fundamental groups. Takes `Y` as yes and `N` as no.  
  Defaults to `N`.
- `LaTeX_Include_Subsection`: When the summary is styled as latex tables, includes a `\subsection` before each table with some invariants. Takes `Y` as yes and `N` as no.  
  Defaults to `N`.
- `Summary`: Takes the name of the file where a database of examples will be exported. It will be of extension `.txt` or `.tex` depending on `Summary_Style`. For example if one writes

      Summary: filename

  A file called `filename.txt` will be created. Unfortunately, the filename shall not contain white spaces.  
  Defaults to `SUMMARY`.
- `Summary_Sort`: Determines how to sort the examples to export them to the summary file. Takes two possible options:
  - `By_N`: Sorts relative to the invariants of the chains in increasing order.
  - `By_Length`: Sorts by size of the chains in increasing order.

  Defaults to `By_N`.
- `Nef_Check`: Determines wether to check for nef-ness of the canonical class <img src="https://render.githubusercontent.com/render/math?math=K_S">. Takes four possible options:
  - `Y`: Tests for nef-ness. It the test fails, discard the example.
  - `skip`: Same as `Y`.
  - `N`: Does not test for nef-ness.
  - `print`: Tests for nef-ness, but does not discard examples. Instead adds the result of this test to the summary file.

  Defaults to `Y`.
- `Effective_Check`: Determines wether to check for <img src="https://render.githubusercontent.com/render/math?math=\mathbb Q">-effectiveness of the canonical class <img src="https://render.githubusercontent.com/render/math?math=K_S">. Takes the same options as `Nef_Check` that act analogously.  
  Defaults to `Y`.
- `Obstruction_Check`: Determines wether to check if the examples have no global obstruction for smoothing. Takes the same options as `Nef_Check` that act analogously.  
  Defaults to `Y`.
- `Single_Chain`: Determines wether to search for one Wahl chain. Takes `Y` as yes and `N` as no.  
  Defaults to `Y`.
- `Double_Chain`: Determines wether to search for two Wahl chain. Takes `Y` as yes and `N` as no.  
  Defaults to `N`.
- `Single_QHD`: Determines wether to search for one <img src="https://render.githubusercontent.com/render/math?math=\mathbb Q \mathcal H \mathcal D^3"> plumbing graph. Takes `Y` as yes and `N` as no.  
  Defaults to `N`.
- `Double_QHD`: Determines wether to search for one <img src="https://render.githubusercontent.com/render/math?math=\mathbb Q \mathcal H \mathcal D^3"> plumbing graph and one Wahl chain. Takes `Y` as yes and `N` as no.  
  Defaults to `N`.
- `Search_For`: Takes a space-separated list of numbers. Those are the <img src="https://render.githubusercontent.com/render/math?math=K_X^2"> that we are aiming for. For example,

      Search_For: 3 4

  Defaults to `1 2 3 4`
- `K2`: Sets the self intersection of the canonical class in the original surface.  
  Defaults to `0`.
- `Keep_First`: Determines how to deal with multiple examples with the same invariants. Takes three options:
  - `local`: Within each sub-test, keep the first example with the given invariants.  
  Since configurations are usually very symmetric, one can expect a huge number of repetitions, since equivalent although different sub-configurations will give equivalent results.
  - `global`: Keep the first example with the given invariants along all tests.
  - `N`: Keep all examples. Not only multiple examples may appear thanks to symmetries of the configuration, but within the same sub-test it is possible to obtain the exact same example more than once, due to fundamental limitations of the program, so this is highly unrecommended.
  
  When an example is obtained such that its invariants were already found, it gets immediately discarded. This means that if any check is set to `print` and the first example found did not pass the test, but the second example did, then only the less interesting example is included in the results. Thus an option other than `global` still has some merit.  
  Defaults to `global`.
- `Threads`: The amount of threads to spawn with multi thread support. Has a hard limit of `16`, given by the macro `MAX_THREADS` in `config.hpp`.  
  Defaults to `1`.
- `Sections_Input`: Determines the meaning of the parameter when adding curves. Takes two options:
  - `By_Self_Intersection`: The parameter for sections correspond to their self intersection in the original surface <img src="https://render.githubusercontent.com/render/math?math=Y">.
  - `By_Canonical_Intersection`: The parameter corresponds to the intersection of the section with the canonical class in the original surface <img src="https://render.githubusercontent.com/render/math?math=Y">.
  
  This option is useful when dealing with highly singular multiple sections. For example if a curve <img src="https://render.githubusercontent.com/render/math?math=C"> is sextuple section, then <img src="https://render.githubusercontent.com/render/math?math=C.K_Y = -6">, but if it is too singular, perhaps it is not immediate knowing <img src="https://render.githubusercontent.com/render/math?math=C^2">.

## Adding Curves

There are three ways of adding curves. Either as complete fibers, as sections (which in practice can work with any kind of curves, not only sections) and as exceptional curves (or named points). A block of fibers is declared with `Fibers:`. A block of sections is declared with `Sections(n):`, where `n` is a parameter whose meaning is given by the option `Sections_Input`. A block of exceptional curves is declared with `Name:` or `Merge:`. A block ends when another block is declared, or when reaching the end of file. After declaring the first block, it will be impossible to declare further options.  
Indentation makes no difference, it is just useful to make it more readable.
Each curve must be declared with a name, and each name must be unique.

- `Fiber` block: Within a `Fiber` block, fibers are declared. Each fiber declaration consists of two lines. Empty lines or comments are ignored, but the two lines of a fiber declaration **must** be contiguous, that is, no empty lines or comments may appear in between.

  The first line consists of the fiber type followed by a possibly empty list of space-separated test options. Fiber types follow Kodaira's notation of normal crossing singular elliptic fibers, namely,
  - `I1` for a singular node of self intersection <img src="https://render.githubusercontent.com/render/math?math=0">. Note that this fiber is special in that the curve has a double point.
  - `In` for a cycle of <img src="https://render.githubusercontent.com/render/math?math=n"> curves, each with self intersection <img src="https://render.githubusercontent.com/render/math?math=-2">, for <img src="https://render.githubusercontent.com/render/math?math=n = 2,\ldots,9">.
  - `In*` for X-shaped trees of <img src="https://render.githubusercontent.com/render/math?math=n %2B 5"> curves, each with self intersection <img src="https://render.githubusercontent.com/render/math?math=-2">, for <img src="https://render.githubusercontent.com/render/math?math=n = 0,1,2,3,4">.
  - `IV*`, `III*` and `II*` for the rest of the tree-shaped fibers, where each curve has also self intersection <img src="https://render.githubusercontent.com/render/math?math=-2">.

  (Types `II`, `III` and `IV` may still be declared, but require special treatment discussed later)

  The next line contains a space-separated list of names, used to identify the curves in the fiber. For non cyclical fibers, the order is as follows:
    - `In*`: The first <img src="https://render.githubusercontent.com/render/math?math=n%2B1"> curves correspond to the chain in the interior of the X-shape in order. The next two curves are the ones intersecting the first curve of the chain. The last two curves are the ones intersecting the last curve of the chain.
    - `IV*`, `III*` and `II*`: The first curve is the one with order 3 at the middle. Then come the branches in order from smallest branch to largest branch. Within each branch, the curves are ordered starting from the one intersecting the order 3 curve in the middle.

  The options for the fiber in the first line are as follows:
  - `Fix` or `F`: Within the corresponding test, include every curve in every pre-test.
  - `Ign` or `I`: Within the corresponding test, ignore every curve in every pre-test.
  - `Try` or `T`: Within the corresponding test, try every possibility of including or ignoring each curve.
  - `Dis` or `D`: Within the corresponding test, try every possibility of including or ignoring each curve, but ignore the case when including all of them.

  If no options are included, it defaults to `Try`. If a test number is greater than the amount of options given, it defaults to the last option in the list.
  
  For example,

      Fibers:
          III* Try Fix Dis
              A B C D E F G H
          I2   Dis Fix
              X Y
          I1
              Z
  
  declares three fibers, and the graph of the `III*` is given by

                  B
                  |
      E - D - C - A - F - G - H

  In the first test, all possibilities for the curves in `III*` and `I1`, and only those possibilities including <img src="https://render.githubusercontent.com/render/math?math=0"> or <img src="https://render.githubusercontent.com/render/math?math=1"> curve from the `I2` are tested.  
  Assuming there are no more curves defined, the second test does two sub-tests: One including all curves, and other including all curves except for `Z`.  
  The third and further tests always include `X` and `Y`, tries both possibilities for `Z`, and all possibilities for the `III*`, except the case including all of its curves.

  Curves declared through fiber cannot be declared to intersect other curves (including already defined sections), so fibers should usually be declared at the beginning.

- `Sections(n)` block: Within a `Sections` block, sections are declared. Each section declaration consists of two lines. Empty lines or comments are ignored, but the two lines of a section declaration **must** be contiguous, that is, no empty lines or comments may appear in between.

  The first line consists of the name of the new section followed by a possibly empty list of space-separated options, which can be:
  - `Fix` or `F`: to include the curve throughout the test.
  - `Ign` or `I`: to ignore the curve throughout the test.
  - `Try` or `T`: to test both possibilities.
  
  If no options are included, it defaults to `Try`. If a test number is greater than the amount of options given, it defaults to the last option in the list.
  
  The second line contains a possibly empty list of space-separated names, all of which must correspond to curves already declared, be them part of fibers, other sections or exceptionals, all of which can be repeated. The section itself can be included in the list. Each time it appears, it represents a general double point in the curve, and increase its arithmetic genus by one.

  The parameter `n` represents either the self intersection of all sections in the block, or their intersection with the canonical class, depending on `Sections_Input`. **This value is calculated in the original surface** <img src="https://render.githubusercontent.com/render/math?math=Y">.

  For historical and readability reasons, `DoubleSections` is a synonym of `Sections(0)` (resp. `Sections(-2)`) when `Sections_Input` is `By_Self_Intersection` (resp. `By_Canonical_Intersection`).

  Also, `Sections` is a synonym of `Sections(-1)`.

- `Name` or `Merge` block: Within a `Name` block, named points are declared (and more generally, a merge operation is performed). The way in which points are named is by blowing up a point (usually an intersection) and naming the exceptional divisor instead. Each name declaration consists of two lines. Empty lines or comments are ignored, but the two lines of a name declaration **must** be contiguous, that is, no empty lines or comments may appear in between.

  The first line consists of the name of the new exceptional followed by a possibly empty list of space-separated options, which can be:
  - `Fix` or `F`: to include the curve throughout the test.
  - `Ign` or `I`: to ignore the curve throughout the test.
  - `Try` or `T`: to test both possibilities.
  
  If no options are included, it defaults to `Try`. If a test number is greater than the amount of options given, it defaults to the last option in the list.

  The second line contains a possibly empty list of space-separated names, all of which must correspond to curves already declared, be them part of fibers, sections or other exceptionals, all of which can be repeated. An exceptional cannot have double points, thus **it is illegal to include the exceptional itself in the list**. The curves in the list must intersect each other in a way made precise later.

  When the list has at most two curves (possibly both the same one), a normal blow up is done:
  - If the list is empty, the blowing up is done at a general point outside all other curves.
  - If the list contains only one curve, the blowup is done at a general point inside the curve, decreasing its self intersection by one.
  - If the list contains two different curves, these curves are expected to have been intersecting at least one point. The blowup then separates these curves at some intersection and decreases their self intersections.
  - If the list only contains a curve twice, the curve is expected to contain at least a double point (for example be a `I1`). The blowup is done at one such double point, solving the singularity and decreasing its self intersection accordingly.

  This process is generalized to include more curves by means of a *merge* operation. Essentially declares that through a point there pass precisely the curves in the list with multiplicities given by the amount of times they appear in the list, where before the operation, those curves were assumed to be in general position. This declaration makes it so that after blowing up the point and naming the exceptional, all curves in the list intersect the exceptional in general points instead of intersecting each other.

  This operation specializes a configuration that was in a more general position, and thus we ask that the intersection between the total transforms of pairs of curves remain the same. Thus,
  - For every curve <img src="https://render.githubusercontent.com/render/math?math=C"> that appears <img src="https://render.githubusercontent.com/render/math?math=a"> times in the list, <img src="https://render.githubusercontent.com/render/math?math=C"> must have at least <img src="https://render.githubusercontent.com/render/math?math=\frac{a(a-1)}{2}"> double points, which will be solved after the operation. It will end up intersecting the exceptional <img src="https://render.githubusercontent.com/render/math?math=a"> times in general points.
  - For every curve <img src="https://render.githubusercontent.com/render/math?math=C"> that appears <img src="https://render.githubusercontent.com/render/math?math=a"> times in the list and every curve <img src="https://render.githubusercontent.com/render/math?math=D"> that appears <img src="https://render.githubusercontent.com/render/math?math=b"> times, they must intersect at least <img src="https://render.githubusercontent.com/render/math?math=a \cdot b"> times. After the operation, they will intersect each other <img src="https://render.githubusercontent.com/render/math?math=a \cdot b"> less times.
  
  By realizing the merge operation multiple times, any resolution of plane curve singularity can be achieved. As an example, we can simulate `II`, `III` and `IV` fibers in the following way:

      // Fiber of type IV
      Fibers:
          I3
          A B C
      Merge:
          E
          A B C
    
  The result is given by

      A(-3)  B(-3)
         \    /
          E(-1)
            |
          C(-3)
    
  Which correctly represents the log resolution of a `IV` fiber.

      // Fiber of type III
      Fibers:
          I2
          A B
      Merge:
          E
          A B
          F
          A B E
    
  The result is given by

      A(-4)  B(-4)
         \    /
          F(-1)
            |
          E(-2)
    
  Which correctly represents the log resolution of a `III` fiber.

      // Fiber of type II
      Fibers:
          I1
          A
      Merge:
          E
          A A
          F
          A E
          G
          A E F
    
  The result is given by

      A(-6)  E(-3)
         \    /
          G(-1)
            |
          F(-2)
    
  Which correctly represents the log resolution of a `II` fiber.

  Disclaimer: This simulation is still just a trick that fools the computer into working with these fibers. In reality it keeps believing the fibers are of types `I3`, `I2` or `I1`. A consequence of this is that it will probably give false positives for the effective check in the case of `IV`.

  **Even after blowing up, the parameter** `n` **of** `Sections(n)` **is a value calculated in the original surface** <img src="https://render.githubusercontent.com/render/math?math=Y">, **not in the surface after blowups done in** `Name`.


**Testing cannot start if there are curves with double points. Every singularity must be solved.**

Several test files are provided in this repository as examples of test files.

# Effective check

This checks for <img src="https://render.githubusercontent.com/render/math?math=\mathbb Q"> effectiveness of <img src="https://render.githubusercontent.com/render/math?math=K_S">. Assuming there are no fibers with triple points, this does the following: Consider only those fibers whose every curve is either included on the configuration or contracted. For each fiber, calculate the minimum discrepancy of all curves in the fiber, and take minimum again so this value is at most <img src="https://render.githubusercontent.com/render/math?math=-1/2">. Sum over all these values. Then <img src="https://render.githubusercontent.com/render/math?math=K_S"> is considered effective if this sum is at most <img src="https://render.githubusercontent.com/render/math?math=-1">.

As long as every complete fiber is of type `In`, this should not give false positives.

# Nef check

This checks for nef-ness of <img src="https://render.githubusercontent.com/render/math?math=K_S">. It assumes that <img src="https://render.githubusercontent.com/render/math?math=K_S"> is effective, so to verify that for each exceptional <img src="https://render.githubusercontent.com/render/math?math=(-1)">-curve, the sum of discrepancies over all curves it intersects is at most <img src="https://render.githubusercontent.com/render/math?math=-1">.  
Due to some limitations of the program, this check may fail if the original configuration had infinitely near blowups. If this happens, a warning is added to the problematic example.

# Obstruction check

This checks if the global obstruction for smoothing the singularities in <img src="https://render.githubusercontent.com/render/math?math=S"> is zero. This checks for the following:
- At most two complete fibers are included in the example.
- after all blowups as declared by `Merge` operations, every (smooth) curve that is not exceptional or fiber has self intersection at least <img src="https://render.githubusercontent.com/render/math?math=-1">.

Sometimes it is useful to know why the discrepancy is not zero. Therefore if `Obstruction_Check` is set to `print`, whenever an example fails this check, the number of complete fibers in the configuration is also included in the summary.

As long as every complete fiber is of type `In`, this should not give false positives. It could however give many false negatives, specially when dealing with infinitely near blowups.

---

Note that all three checks really only make sense when the original surface is an elliptic surface with sections, under the assumption that every fiber component is declared in the a `Fiber` block. This program could be used in a more abstract setting, but the three checks would become invalid.

About the exhaustiveness of the search, some restrictions are taken: The original aim is to find Wahl singularities with effective / ample canonical class, then blowing up non singular points in the configuration would immediately invalidate that example. Also, no examples will appear that have a (-1)-curve intersecting both ends of a Wahl chain. These conditions are used extensively during the search, so possible examples that require wilder blowing up would not be found.
