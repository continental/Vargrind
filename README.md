# vargrind

*Variable shadowing tool - Tracks data about variables (frequency of use, etc) during program execution*

## Goals
*bolded = important*

* **Provide shadow states (SS) for all variables in the program** (Shadow state - information describing the various aspect of various variables)
	* **Information Proper**
		* **Name**
		* Type
		* Parent scope (the name of the function that defined it)
		* Special descriptors (const, volatile, etc)
		* References in code?
		* Lifetime (from declaration to deletion/program exit)
		* Location (In memory)

	* **Number of uses**
		* **Total (Inside of the whole program)**
		* **Scope (Per scope)**

	* **Users (functions/operations which used the variable)**
		* **All (Logged all uses of the variable, program wide or scope specific)**
		* Last 
		* Most frequent
	* Changes (Variable's value change)
		* History
		* Current value
		* Previous value
		* Instigator (which function/operation changed it)
* **Output SS info**
	* **To terminal**
	* To external logs/files (export)
* Support complex data types (struct, class)
	* Shallow analysis mode (Treat complex data types as closed packages)
	* Deep analysis mode (Break complex types down into simple types and analyze each one as a separate variable)
		* Shadow attribute's parent class/struct
* Post-completion optimizations

## Prerequisites

In order to use Vargrind, one must have Valgrind source code cloned. Info about cloing Valgrind code can be found here: https://valgrind.org/downloads/repository.html

## Installation

1. Place the repository within your Valgrind installation directory
2. Navigate to the repository folder
3. Run the following command:
	`./SetupVargrind.sh`

### Troubles while installing

If the user encounters some compilation issues, it is recommended to recursively delete whole valgrind directory and start the installation again.

As a prerequisite, user shall have the autotools and automake installed in order to build the project.
	
## Running the tool

Usage: `valgrind --tool=vargrind --read-var-info=yes ./program_name`

Copyright
-------

Copyright (c) 2023 Continental AG and subsidiaries. All rights reserved.

This project is licensed under the GNU GENERAL PUBLIC LICENSE Version 2, see the LICENSE.txt file for details.

