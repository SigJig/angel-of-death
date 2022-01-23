
# Overview

The GEDCOM forma consists of multiple lines, matching the production
* ```gedcom_line: number delim [pointer delim] tag [delim line_value] terminator```


Rules
* Lines represent one database field
* Long lines can be split by CONC or CONT
* Levels can not contain a leading 0
* Levels must start at 0 and only increment by one, and be a maximum of 99
* Length constraints are in characters, not bytes
* Pointers must have a maximum length of 22, including leading and trailing @, and its definition must be unique
* GEDCOM Tag have a maximum of 31 characters, 15 being unique (?)
* GEDCOM lines must not exceed 255 characters
* Leading white spaces should be ignored
* Pointers and line_values can not be on the same line
* User defined tags begin with an underscore