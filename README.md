# OSET: Organic Synthesis Exploration Tool

This is a mirror of abandoned project [OSET](https://web.archive.org/web/20130622005623/http://ivan.tubert.org:80/caos/index.html), distributed under [GNU General Public License](http://www.gnu.org/copyleft/gpl.html)

## *Repository structure*
|Index| Content |
|--|--|
| sources | OSET source code  |
| compiled_win32 | Last compiled binaries for Windows |
| moldb | The compounds database |
| thesis.pdf | Iván Tubert's undergraduate thesis |

To download the classes for the structure editor applet, go to  [ChemAxon](https://web.archive.org/web/20130701134137/http://www.chemaxon.com/marvin/) (current website: http://www.chemaxon.com/marvin/)

Below is reported the original documentation, taken from the [original website](https://web.archive.org/web/20130622005623/http://ivan.tubert.org:80/caos/index.html) (visitable only through web.archive.org)

## *About the project*

The OSET project is currently under development at the  [School of Chemistry](https://web.archive.org/web/20130622005623/http://www.fquim.unam.mx/)  of the  [National Autonomous University of Mexico](http://www.unam.mx/)  by  [Iván Tubert](mailto:tubert@eros.pquim.unam.mx)  and  [Eduardo Tubert](mailto:eatubert@hotmail.com).The aim of the project is to develop a Computer-Assisted Organic Synthesis (CAOS) program for use in the teaching of Organic Chemistry, and retrosynthetic analysis in particular.

-   The program will be  [open-source](http://www.opensource.org/)  and freely available under the  [GNU General Public License](http://www.gnu.org/copyleft/gpl.html). This is in support of the  [Open Science Project](http://www.openscience.org/). As far as we know, there is no other open-source project in Computer Aided Organic Synthesis. There are a couple of commercial CAOS programs costing around $200 USD. But the main advantage of the open-source paradigm is not the price but the possibility to adapt the program to your needs and the possibility of distributed development (and debugging!).
-   It should run on the most popular computer types and operating systems (Windows, Mac, Unix, Linux, etc.). To achieve this, rather than writing a different version for each operating system, we decided to take advantage of the portability of Java.

At this time we have a working prototype for Windows 95-98 (development of the windows version has been discontinued), and we are currently developing a client-server system with the client written in Java and the server written in C. We used the freely distributed stucture editor applet from  [ChemAxon](http://www.chemaxon.com/)  for the structure input part.

## *Computer-Assisted Organic Synthesis (CAOS)*

What is Computer-Assisted Organic Synthesis?

CAOS was born in the late 1960s, when E. J. Corey and T. Wipke created the first CAOS program, OCSS (Organic Chemistry Synthesis Simulator). Since then, a few dozen CAOS programs have been developed. Most of them are academic projects and run in expensive computers such as the VAX or the PD-10. On the other hand, a few commercial programs for the PC have appeared recently, such as  [Syntree](https://web.archive.org/web/20130826174534/http://www.trinitysoftware.com/trinity2/orgchem/syntree.htm)  or  [HOLOWin](https://web.archive.org/web/20130826174534/http://www.chemcad.fr/produits/abinitio/holowin.html). As far as we know, OSET is the first open source project in this field.

The aim of an CAOS program is to assist the chemist in the design of the synthetic pathway for a complex target molecule. Most programs work in the  [retrosynthetic direction](https://web.archive.org/web/20130826174534/http://www.cmbi.kun.nl/cheminf/ira), that is, from target molecule to simple precursor, in a process opposite to the one actually done in the laboratory. Some examples of these programs are  [LHASA](https://web.archive.org/web/20130826174534/http://www.cmbi.kun.nl/cheminf/lhasa/),  [SYNGEN](https://web.archive.org/web/20130826174534/http://syngen2.chem.brandeis.edu/syngen.html),  [SYNCHEM](https://web.archive.org/web/20130826174534/http://www.cs.sunysb.edu/~synchem/), and OSET. A few programs work in the synthetic direction: some attempt to predict reactions given a set of reactants and conditions (like  [CAMEO](https://web.archive.org/web/20130826174534/http://zarbi.chem.yale.edu/programs/cameo.html)); others try to find a synthetic pathway from designated starting materials searching in the synthetic direction. Some programs try to combine both approaches (e.g.,  [WODCA](https://web.archive.org/web/20130826174534/http://www2.ccc.uni-erlangen.de/software/wodca/index.html)).

In order to do its job, a CAOS program must solve most of the following problems:

-   Structure input and output (usually via a graphic user interface)
-   Internal representation of molecules and transforms
-   Canonicalization
-   Perception of atom types, functional groups, rings, strategic bonds, etc.
-   Transform evaluation
-   Synthetic tree management
-   Strategies

## *Transform description language*

Each transform consists of two parts: a header and a body. The language is case insensitive. Optional fields are enclosed by brackets [].

## Header

The header has the following items:

>.rxn

Marks the beginning of a new transform. Notice the dot before  rxn.

>name  "_transform name"_

_transform name_  must be unique and begin with a letter. The name must be enclosed by double quotes.

>type  _transform type_

At the present time, type may be GP1 or GP2. To add new types it is necessary to modify the source code.

>G1  _functional group list_

Specifies the functional group that keys a GP1 transform or one of the two groups for a GP2 transform. It is a comma-separated list of functional group names. The names must match those in the file  funcgrp.chm.

>[G2  _functional group list_]

Used only in GP2 type reactions; specifies the second group that keys a GP2 transform. Note that if GP1 = GP2, the transform is called twice (usually not a bad idea).

>[path  _pathlength_]

Used only in GP2, but it is mandatory for all GP2 transforms.  _pathlength_  is an integer. The length is the number of atoms between the root atoms of the two groups,  _including_  both root atoms. Thus, the minimum length is usually two.

>[rating=_rating_]

_A priori_  rating. Default=50. The rating is a predefined numerical variable and may be modified anywhere in the transform body.

>[.comments]

Comments block. The comments are a list of double quote-enclosed strings. The strings are automatically concatenated. Newline characters may be specified with the \n (backslash-en) sequence within the string. The comments block ends at the next header field.

>.start

Marks the beginning of the body

## Body

The body of the transform is a statement list and goes after  .start. The following statements are available:

### Assignment

>_variable_  =  _expression_

For example,  rating = rating + 10  is a valid assignment.

### If ... then ... else

>if  (_expresion_)  then

_statement list_

>[else

_statement list_]

>endif

The classical conditional execution statement. The  endif  is mandatory, even if there the statement list is null or just one line.

### Foratom

>foratom (_atom1_  from  _atom0_)

_statement list_

>next

This is used to visit all of the neighbors of the atom specified by the atom  _atom0_. The current neighbor is stored in the variable  _atom1_.

### Breakbond

>breakbond(_atom1_,  _atom2_)

Breaks the bond between the specified atoms. If the bond is double or triple, it breaks it completely.

### Makebond

>makebond(_atom1_,  _atom2_)

Creates a single bond between  _atom1_  and  _atom2_.

### Add

>add(_atom1_,  _element_)

Creates a new atom of the specified element and joins it to  _atom1_  with a single bond.

### Done

>done

This is an extremely important statement and can be easily overlooked. Repeat: do not forget to put at least a  done  statement in each transform. After all operations required by the transform are applied to the molecule, the  done  statement is used to indicate this. What actually happens is that a copy of the working molecule is saved and a new working molecule is created. The new molecule is exactly as it was at the beginning of the execution of the transform. The execution continues at the next statement after  done, so it is  _not_  like an "end" statement, it is more like a "save". For example, if  done  is within a loop, several different molecules may be generated by a single execution of a transform. There may be more than one  done  statement in a transform. If there is no  done  statement, the transform is executed but it returns no results!

## Operators

Expressions are very similar to those in other languages such as Pascal or BASIC, so they may contain nested parentheses. The available operators are the following.

### Is / isnot

_atom_  is  _atom_type_

These are used in expressions to check whether a given atom is of a given type. The types are reserved words, and may be any of the following:
- methyl
- primary
- secondary
- tertiary
- quaternary
- vinyl
- carbonyl
- alkynyl
- nitrile
- allene
- alkyl
- sp3
- sp2
- sp
- noncarbon
- nitrile
- hydroxyl
- ether
- peroxide
- carbonyl
- nitro
- unidentified
- allyl
- benzyl
- alpha_carbonyl
- alpha_alkynyl
- alpha_nitrile

The  *is  /  isnot*  operator has the highest priority. The operator type is boolean.

### Logical operators

>and, or, not

The name says it all. The priority follows the typical order:  not  >  and  >  or.

### Comparison operators

>, >=, <, <=, <>, =

Notice that the equality operator is the same as the assignment operator. Surely this will be disliked by mathematicians and philosophers. The equality and not-equals operator may be used with variables of any type; the other comparison operators are only valid for numeric types.

### Math operators

>+, –, *, /

As usual, multiplication and division have a higher priority than addition and subtraction. Associativity is from left to right.

## Types and variables

The variable types commonly used are numeric (integer) and atom. Variables do not need to be declared; their type is automatically assigned by the program. But the typing is global. That means that once a variable type is assigned the same variable cannot be used with other type, even in another transform. Other variable types include string and functional groups, but currently they cannot be manipulated, so they are more like constants.

There are several pre-defined variables.

A1 ... A9, B1 ... B9, P1 ... P9

These are all atom-type variables. They are assigned automatically at the beginning of each transform and are used as the basis for all atom-based operations. AN  are the atoms of the first functional group, and are assigned according to the definition of the functional group (see the functional group description language guide). BN  are likewise defined for the second functional group; obviously they are only useful in GP2-type transforms. PN  are defined for the path between the two groups: P1 is the root of the first group, and the numbering goes on until the root of the second group. Notice that, for a 2-group transform, several atoms are assigned more than one variable (for example, A1 might be the same as P1).

### Rating

rating  is a numerical variable used to evaluate how chemically useful a transform is. It is a semi-quantitative measure of the yield, generality, cost, etc. of the reaction. It should be modified as necessary by the transform body if structural conditions warrant it.

## Functional group description language

Functional groups are described in the file  funcgrp.chm. Each entry consists of the following:

1. Group name. This identifier must be unique and cannot be a reserved word. This means that it is not possible to use an atom type as a name. The name must begin with a letter or underscore, followed by any length of letters, numbers or underscores.

2. Pattern string. This is described below.

3. Complexity.

For example, the line describing carboxylic acids might look like this:

>ACID = "C(=O O([H]) [CH])", 10

The pattern string consists of several "hard" and "soft" atoms. The hard atoms  _must_  be present in the structure and are automatically numbered from left to right and assigned the default variables A1 ... An. Soft atoms may be several atoms that must match a list of elements.  **A**  is a hard atom and  **O**  is a soft atom.

    description =  **"A"**
    
    A =  **S**_(A... O)_ 
    
    O =  **[S...]**_^{n-m}_  or  **[S...]**_^n_
    
    S = chemical element symbol (case sensitive).

Fields in italics are optional. Notice that the above definition is recursive.

The pattern must begin with a hard atom, which is called the  _root atom_, optionally followed by a neighbor list in parentheses. The importance of the root atom is that it is used to calculate distances between pairs of functional groups.

The neighbor list is a list of zero or more hard atoms, optionally followed by a soft atom. The soft atom is enclosed by brackets and optionally followed by a number or number range. Double and triple bonds are represented by = or %, respectively.

![](https://web.archive.org/web/20130622005734im_/http://ivan.tubert.org:80/caos/doc/Image63.gif)

In the carboxylic acid example we have as the root atom the carboxyl carbon (numbered A1), followed by: an oxygen bonded to the carbon with a double bond (A2), an oxygen with a single bond (A3), which in turn is followed by a soft hydrogen, and finally a carbon or hydrogen joined to the root atom. Why did we use a soft hydrogen instead of a hard hydrogen? Actually, the only difference is that as a soft hydrogen that atom will not be numbered. Since all hydrogen atoms in our program are implicit, it does not make sense to number them.

Let us look at another example: the alcohol.

> ALCOHOL = "C(O([H]) [CH]^3)", 10

Here the carbon is the root atom, and its neighbors are the oxygen (which in turn is bonded to hydrogen) atom and three soft atoms, each of which may be either carbon or hydrogen. Notice that this definition rules out enols, gem-diols, a-halohydrins, phenols, among others. We decided that this restrictive definition was better from a synthetic point of view. For example, even if an enol may be regarded as a kind of alcohol, its reactions and synthesis are different enough to regard it as an independent functional group.

Here is the complete list of special characters used in functional group descriptions:
|Chars| Function |
|--|--|
| ***[ ]*** | Encloses soft atoms. Between the brackets there must be a list of one or more chemical elements. The number of soft atoms may be optionally specified using ^n after the closing bracket. |
| ***^*** | Must be followed by a number or a range of numbers. Ranges consist of two numbers separated by a hyphen and enclosed in curly brackets. Numbers may only be used for soft atoms. |
| ***{ }*** | Encloses a range. |
| ***( )***  | Encloses the neighbor list of an atom. Only hard atoms may have neighbor lists. There may be parentheses inside parentheses, generating a tree-like structure. The neighbor list can have zero or more hard atoms, optionally followed by one soft atom.|
| ***-*** | Separates the lower limit from the upper limit of a range. |
| ***=*** | Double bond. Must be immediately before a hard atom. |
| ***%*** | Triple bond. Must be immediately before a hard atom. |
| ***" "*** | Encloses the pattern string. |


## *OSET Applet Help*

Here is a screen shot of the OSET applet, followed by a description of its components. The target molecule in this example is LSD.

![](https://web.archive.org/web/20130622005618im_/http://ivan.tubert.org:80/caos/applet.gif)

### Upper panel (tree panel)

In this panel you can visualize the entire synthetic tree for the synthesis that is currently under consideration.When the tree is large the molecules may become illegible (especially if the screen resolution is small; we recommend 1024 x 768 or better), but the idea is to get an overview of the structure of the tree, not of each molecule. When possible, the heteroatoms are indicated by their chemical symbols, else they are shown as color dots according to the following code:

O: red ·

N: blue

Br: orange ·

Cl: green ·

### Lower panel (precursors panel)

In this panel you can visualize the precursors from the last retrosynthetic step, as well as unanalyzed precursors from previous steps. On the left side of this panel you can see the precursors from the last step. They are relatively big and with black lines (if a precursor appears with green lines it's because it has been analyzed previously). On the right side of the panel are the  _orphans_, precursors found on previous steps that were left unanalyzed. Thay appear here to remind the user that they must be analyzed later unless they are considered as available starting materials. The orphans appear smaller than the other precursors and with gray lines. If an orphan has been analyzed previously, it appears red.

When a compound is found in our "starting materials catalog", the name appears under its structure, like diethyl amine in the example. The catalog contains currently 70 compounds; not much, but they are the most common acceptable starting materials in textbook problems and exercises (mostly four carbon atoms or less).

The  **scrollbar**  under this panel is used to view alternative precursors for the current target. On the upper left corner you can see how many results were found for the current target. In the example, we are viewing the first result out of eleven. The results are sorted in decreasing order of preference according to our evaluation function, but the user does the best evaluation, so please check all the results. The number on the upper right indicates the depth of the analysis, that is, the number of analysis steps.

**To analyze a molecule, just click over the structure.**  The mouse cursor should change to a hand when it is over a clickable structure, and during the analysis it should change to a hourglass or equivalent. The analysis usually takes a few seconds, but the delay depends more on the speed of the network connection than anything else. When analyzing a structure, remember to select the desired analysis mode (see below).

### Right panel (information panel)

In this panel you can read the information regarding the last analysis step. The following information is available:

-   **Transform name:**  In this case the transform corresponds with the Wittig reaction.
-   **Rating:**  An evaluation of the transform on chemical grounds. The rating should consider factors such as yield, cost, selectivity, reliablitiy, etc., but currently the rating function is very crude.
-   **Simplification:**  A measure of the simplification (decrease in molecular complexity) achieved by the use of this transform. Notice that the results are sorted by the product Rating · Simplification.
-   **Comments:**  This includes more information about the reaction, including reaction conditions and other comments, and the reference. In this example the only information provided is a reference.
-   **Fitness:**  This function is under development and it is not very useful at this time, but it should be a global evaluation function which takes into account not only the rating and simplification, but also the structure of the tree.

### _Up_  button

Click on this button to go up one level in the retrosynthetic tree. When you are on the first level (i.e., the main target molecule) this button is grayed.

### _Find CAS_  button

Use this button to get the names, SMILES, and CAS registry numbers of all the compounds in the precursors panel. We currently use a catalog with more than 40,000 compounds. If a molecule is not found, it returns "unknown".

### Mode selector

This radio button is used to select between three modes of analysis: disconnective, auxiliary, and FG0.

-   **Disconnective:**  This is the default mode, which uses only transforms which break skeletical bonds (mainly C-C bonds). This mode applies to the entire structure of a molecule. This class of transforms include GP1 and GP2 transforms, which are keyed by one or two functional groups in the molecule.
-   **Auxiliary:**  This mode uses transforms which do not simplify the target structure, such as  _functional group interchanges_  (FGI) and  _functional group additions_  (FGA). These transforms are used to obtain the full retron for a simplifying (i.e., disconnective) transform.  **To use an auxiliary transform, you must click on an atom**, because otherwise the number of results would be unwieldy. Only transforms which affect the functional group which contains the selected atom are used.
-   **FG0 (that's a zero!):**  These are disconnective transforms that are not keyed by functional groups. This includes some coupling reactions, such as the Wurtz coupling.  **To use an FG0 you must click on a bond.**

**Important note:**  If you analyze a molecule, then go up, and then analyze it again using a different mode (or a different atom or bond), the new results are appended after the previous results. So if it seems like the results aren't new, try moving to the end of the list.

## *OSET Reaction Index*
The following sections describe briefly some of the most important synthetic reactions, and have links to some other sites with information about the same reactions. If you would like to suggest more links, don't hesitate to write to  [caos@litio.pquim.unam.mx](mailto:caos@litio.pquim.unam.mx). You are also invited to collaborate with summaries for other reactions. You will receive due credit, of course.

## Aldol Reaction

![](https://web.archive.org/web/20130325080338im_/http://ivan.tubert.org:80/caos/reactions/aldol.gif)

The addition of an enolate to an aldehyde or ketone is called the  _aldol reaction_. In the classical aldol reaction, compounds  **1**  and  **2**  are aldehydes or ketones. However, there are closely related reactions where compound  **1**  is an aldehyde, ketone, ester, anhydride, or nitrile. The aldol and related reactions are base-catalyzed, but in some cases it is possible to use acid catalysis.  b-hydroxyaldehyde  **3**, which is called an aldol, may be dehydrated under the reaction conditions to give  a,b-unsaturated carbonyl compound  **4**  if R2 or R3 is an hydrogen atom. The aldol reaction is an equilibrium, which may lie to the right or to the left depending on circumstances (see below).

The aldol reaction may occur between two molecules of the same aldehyde or the same ketone, or between different molecules (crossed aldol reaction). There are five possibilities:

1.  **Two molecules of the same aldehyde.**  This is the classical case: there is only one product (or two, considering the dehydrated product) and the equilibrium lies far to the right.
    
2.  **Two molecules of the same ketone.**  In this case the equilibrium lies to the left, but it can be shifted by using appropiate reaction conditions, such as continuous extraction. If the ketone is unsymmetrical the condensation might occurr on either side, giving a mixture of products. In practice, the condensation occurs usually on the side that has more hydrogens (that is, the kinetic enolate is formed).
    
3.  **Two different aldehydes.**  In general a mixture of four products is obtained (eight counting the dehydration products). However, if one of the aldehydes does not have  a  hydrogens (e.g., benzaldehyde), only two products are obtained, and in many cases the crossed product is the main one. The crossed aldol reaction is often called the Claisen-Schmidt reaction.
    
4.  **Two different ketones.**  This is the most problematic case, but it can be done if one of the ketones is transformed first into a preformed enolate.
    
5.  **An aldehyde and a ketone.**  This is usually feasible because aldehydes are stronger enolates than ketones, so usually it is the a carbon of the ketone the one which adds to the aldehyde. This reaction is also called the Claisen-Schmidt reaction. To insure regioselectivity when using unsymmetrical ketones it is necessary to use preformed enolates.
    

Other problems that may occur with the aldol reaction include polycondensation and undesirable Michael Additions if the product is dehydrated. The aldol reaction may be used intramolecularly to close five- and six-membered rings, like in the Robinson annulation reaction.

**Stereoselectivity:**  this reaction may be carried out stereoselectively by using a chiral auxiliary group such as  [oxazolidinone](https://web.archive.org/web/20130325080338/http://yakka.pharm.kumamoto-u.ac.jp/yakkagaku/yakka-2-e.html)  derivatives and/or asymmetric aldehydes or ketones. This requires the use of preformed enolates with a certain geometry (E/Z) which can be obtained by varying the reaction conditions (termodynamic vs kinetic).

### References
March, J.  _Advanced Organic Chemistry_, 4th ed., reactions 6-39, 6-40, 6-41

### More Links
[More information about the aldol condensation](https://web.archive.org/web/20130325080338/http://macweb.acs.usm.maine.edu/chemistry/newton/Chy251_253/Lectures/Aldol%20Condensation/AldolFS.html)
[Some general references](https://web.archive.org/web/20130325080338/http://home.ici.net/~hfevans/reactions/RXN009.htm)
[An introduction to the aldol reaction](https://web.archive.org/web/20130325080338/http://www.chem.orst.edu/ch336/Chapter18/Aldol.htm)
[A quick quiz about aldol condensations](https://web.archive.org/web/20130325080338/http://www.cem.msu.edu/~reusch/OrgPage/Questions/Match/aldol.htm)
[Organic Reactions at Penn State](https://web.archive.org/web/20130325080338/http://courses.chem.psu.edu/chem39/reactions/reactions.html)  - Includes 49 reactions, including aldol condensation (reaction number 29).
[Asymmetric Aldol reactions](https://web.archive.org/web/20130325080338/http://www.carreira.chem.ethz.ch/Carreira-Website/OCVI-99/section1pg16.html)

## Enolate alkylation

![](https://web.archive.org/web/20110819052027im_/http://ivan.tubert.org:80/caos/reactions/enolate.gif)

Ketones, esters, nitriles, and carboxylic acids may be alkylated at the alpha position in strongly basic conditions. Typical bases for this reaction include Et2NLi, LDA (lithium diisopropyl amide), t-BuOK, NaNH2, and KH. This reaction can very useful for creating carbon-carbon bonds. However, it is usually not as general as the scheme above might suggest. The main limitations to the scope of this reaction are as follows:

-   **The nature of the R group:**  From the standpoint of the alkyl halide, the reaction follows an  [SN2](https://web.archive.org/web/20110819052027/http://www.chem.arizona.edu/courses/chem242/sn2.html)  mechanism. Therefore, the alkyl halide should preferably be primary, allylic, or benzylic. Secondary halides sometimes work, but may give predominant elimination ([E2](https://web.archive.org/web/20110819052027/http://www2.trincoll.edu/~sstickle/handouts/spring/snhandout.html)). Tertiary halides do not work at all. However, there is an alternative method which permits the use of tertiary halides: the silyl enol ether derived from an aldehyde, ketone or ester may be alkylated using a  [Lewis acid](https://web.archive.org/web/20110819052027/http://kauai.cudenver.edu:3001/0/acbspg1.dir/acidpg4.html)  as catalyst.
-   **Overalkylation:**  Another problem is the fact that the alkylated ketone is just as acidic (or more) than the starting material. Therefore it can be alkylated also, so di- and trialkylation may be common. An alternative method to avoid this problem is to use the  [Stork Enamine Reaction](https://web.archive.org/web/20110819052027/http://home.ici.net/~hfevans/reactions/RXN373.htm), in which th ketone is first transformed into an enamine and then alkylated. This method works only with active halides such as allylic, benzylic, and propargillic, but primary and secondaty halides may be alkylated if the enamine is first transformed into a salt.
-   **Regioselectivity:**  The last problem is whether the desired side of an unsymmetric ketone may be alkylated selectively. Sometimes this can be solved by using  [kinetic versus thermodynamic control](https://web.archive.org/web/20110819052027/http://www2.trincoll.edu/~sstickle/handouts/fall/KvsT.html)  when forming the enolate: for example, under kinetic control the least substituted enolate is obtaines; while the thermodinamic enolate is the more substituded one. This requires the use of  _preformed enolates_. Another way is to use a precursor with two electron-withdrawing groups such as acetoacetic acid instead of a ketone, and then decarboxylate. Also, it is possible to introduce a blocking group in one side of the ketone and then remove it after the alkylation.

**Stereoselectivity:**  this reaction may be carried out stereoselectively by using a chiral auxiliary group such as  [oxazolidinone](https://web.archive.org/web/20110819052027/http://sgich1.unifr.ch/oc/renaud/research/la.html)  derivatives.

### References

March, J.  _Advanced Organic Chemistry_, 4th ed., reactions 0-95, 0-94, 0-96, 2-19

### More Links
[Alkylation of Enolate Ions](https://web.archive.org/web/20110819052027/http://macweb.acs.usm.maine.edu/chemistry/newton/Chy251_253/Lectures/EsterSyntheses/AlkylationFS.html)
[Kinetic versus Thermodynamic Control Tutorial](https://web.archive.org/web/20110819052027/http://www.cem.msu.edu/~parrill/thermo/)
[Examples of asymmetric alkylations](https://web.archive.org/web/20110819052027/http://www.carreira.chem.ethz.ch/Carreira-Website/OC-VI/alkylationpg1.html)
[Organic Reactions at Penn State](https://web.archive.org/web/20110819052027/http://courses.chem.psu.edu/chem39/reactions/reactions.html)  - Includes 49 reactions, the enolate alkylation and Stork reaction among them (reactions 25 and 32).
[More alkylations, including the preparation of LDA](https://web.archive.org/web/20110819052027/http://macweb.acs.usm.maine.edu/chemistry/newton/Chy251_253/Lectures/Alkylations/Alkylations.html)

## Grignard addition to carbonyl

![](https://web.archive.org/web/20121215053220im_/http://ivan.tubert.org:80/caos/reactions/grignard.gif)

The addition of  [Grignard reagents](https://web.archive.org/web/20121215053220/http://www.britannica.com/bcom/eb/article/2/0,5716,38892+1+38117,00.html)  to aldehydes and ketones is known as the Grignard reaction. Formaldehyde gives primary alcohols; other aldehydes give secondary alcohols; and ketones give tertiary alcohols. The reaction is of very broad scope, and hundreds of alcohols have been prepared in this manner. R may be alkyl or aryl. In many cases the hydrolysis step is carried out with dilute HCl or H2SO4, but this cannot be done for tertiary alcohols in which at least one alkyl group is alkyl because such alcohols are easily dehydrated under acidic conditios. In such cases (and often for other alcohols as well) an aqueous solution of ammonium chloride is used instead of a strong acid. Other organometallic compounds can also be used, but in general only of active metals; e.g., alkylmercurys do not react. In practice, the only organometallica compounds used to any extent, besides Grignard reagents, are alkyl- and aryllithiums, and alkylzink reagents where enantioselective addition is desired. For the addition of aceylenic groups, sodium may be the metal used: RC%CNa, while vinylic alanes are the reagents of choice for the addition of vinylic groups. Many methods have been reported for the addition of allylic groups. Among these are the use of allyltrialkyltin compounds (in the presence of BF3-etherate), allyltrialkylsilanes (in the presence of a Lewis acid), as well as other allylic metal compounds. (March, 1992)

### References

March, J.  _Advanced Organic Chemistry_, 4th ed., reaction 6-29

### More Links

[Introduction to the Grignard reaction](https://web.archive.org/web/20121215053220/http://www.sunderland.ac.uk/~hs0bcl/org3.htm)
[A more detailed introduction to the Grignard reaction](https://web.archive.org/web/20121215053220/http://www.chem.wsu.edu/chem240/grignard.html)
[Some references](https://web.archive.org/web/20121215053220/http://home.ici.net/~hfevans/reactions/rxn455.htm)
[The Nobel Prize in Chemistry 1912](https://web.archive.org/web/20121215053220/http://www.nobel.se/laureates/chemistry-1912.html)

## Wittig Reaction

![](https://web.archive.org/web/20081006063020im_/http://ivan.tubert.org/caos/reactions/wittig.gif)


### References

March, J.  _Advanced Organic Chemistry_, 4th ed., reaction 6-47

### More Links
[Introduction to the Wittig reaction](https://web.archive.org/web/20081006063020/http://www.sunderland.ac.uk/~hs0bcl/org8.htm)
[Some references](https://web.archive.org/web/20081006063020/http://home.ici.net/~hfevans/reactions/RXN409.htm)
[Organic Reactions at Penn State](https://web.archive.org/web/20081006063020/http://courses.chem.psu.edu/chem39/reactions/reactions.html)  - Includes 49 reactions, including aldol condensation (reaction number 29).
[The Nobel Prize in Chemistry 1979](https://web.archive.org/web/20081006063020/http://www.nobel.se/laureates/chemistry-1979.html)