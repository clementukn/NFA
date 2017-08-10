Non-Deterministic Finite State Automata in C
Uses non-conventional approach to build the automata's links.

**Usage** : auto <word>|<RE> <text_file_path> [<option1><option2>...<optionN>]


| Option   | Description 	   			  		|
| -------- | ----------------------------------	|
|    t     | Display created transitions  		|
|    p     | display path in the automata 	  	|
|    i     | display parameters of the automata |


| Operators   | Description 	|
| ----------- | --------------- |
|    +    	  | one or many		|
|    ,     	  | OR		  		|
|    .        | AND 			|


word.c
======
contains the algorithm (not implemented anymore) of a powerful approach to create automata for words only.


